#include "unix/loop.h"
#include "udp-common.h"
#include <unistd.h>
#include <string.h>

static void _ev_udp_close_unix(ev_udp_t* udp)
{
    if (udp->sock != EV_OS_SOCKET_INVALID)
    {
        ev__nonblock_io_del(udp->base.data.loop, &udp->backend.io, EPOLLIN | EPOLLOUT);
        close(udp->sock);
        udp->sock = EV_OS_SOCKET_INVALID;
    }
}

static void _ev_udp_w_user_callback_unix(ev_udp_write_t* req, size_t size, int stat)
{
    ev__write_exit(&req->base);
    req->usr_cb(req, size, stat);
}

static void _ev_udp_r_user_callback_unix(ev_udp_read_t* req, size_t size, int stat)
{
    ev__read_exit(&req->base);
    req->usr_cb(req, size, stat);
}

static void _ev_udp_cancel_all_w_unix(ev_udp_t* udp, int err)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&udp->send_list)) != NULL)
    {
        ev_udp_write_t* req = container_of(it, ev_udp_write_t, base.node);
        _ev_udp_w_user_callback_unix(req, req->base.data.size, err);
    }
}

static void _ev_udp_cancel_all_r_unix(ev_udp_t* udp, int err)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&udp->recv_list)) != NULL)
    {
        ev_udp_read_t* req = container_of(it, ev_udp_read_t, base.node);
        _ev_udp_r_user_callback_unix(req, req->base.data.size, err);
    }
}

static void _ev_udp_abort_unix(ev_udp_t* udp, int err)
{
    _ev_udp_close_unix(udp);
    _ev_udp_cancel_all_w_unix(udp, err);
    _ev_udp_cancel_all_r_unix(udp, err);
}

static void _ev_udp_on_close_unix(ev_handle_t* handle)
{
    ev_udp_t* udp = container_of(handle, ev_udp_t, base);

    _ev_udp_abort_unix(udp, EV_ECANCELED);

    if (udp->close_cb != NULL)
    {
        udp->close_cb(udp);
    }
}

/**
 * @return bool
 */
static int _ev_udp_is_connected_unix(ev_os_socket_t sock)
{
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);

    if (getpeername(sock, (struct sockaddr*)&addr, &addrlen) != 0)
    {
        return 0;
    }
    return addrlen > 0;
}

static int _ev_udp_set_flags_unix(ev_udp_t* udp, unsigned flags)
{
    int err;
    if (flags & EV_UDP_IPV6_ONLY)
    {
        int yes = 1;
        if (setsockopt(udp->sock, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes)) == -1)
        {
            err = errno;
            return ev__translate_sys_error(err);
        }
    }

    if (flags & EV_UDP_REUSEADDR)
    {
        if ((err = ev__reuse_unix(udp->sock)) != EV_SUCCESS)
        {
            return err;
        }
    }

    return EV_SUCCESS;
}

static int _ev_udp_disconnect_unix(ev_udp_t* udp)
{
    struct sockaddr addr;
    memset(&addr, 0, sizeof(addr));
    addr.sa_family = AF_UNSPEC;

    int r;
    do
    {
        r = connect(udp->sock, &addr, sizeof(addr));
    } while (r == -1 && errno == EINTR);

    if (r == -1 && errno != EAFNOSUPPORT)
    {
        r = errno;
        return ev__translate_sys_error(r);
    }

    udp->base.data.flags &= ~EV_UDP_CONNECTED;
    return EV_SUCCESS;
}

static ssize_t _ev_udp_sendmsg_unix(int fd, struct iovec* iov, int iovcnt, void* arg)
{
    struct msghdr* p_hdr = arg;
    p_hdr->msg_iov = iov;
    p_hdr->msg_iovlen = iovcnt;

    ssize_t write_size;
    do 
    {
        write_size = sendmsg(fd, p_hdr, 0);
    } while (write_size < 0 && errno == EINTR);

    if (write_size < 0)
    {
        int err = errno;
        return ev__translate_sys_error(err);
    }

    return write_size;
}

static int _ev_udp_do_sendmsg_unix(ev_udp_t* udp, ev_udp_write_t* req)
{
    struct msghdr hdr;
    memset(&hdr, 0, sizeof(hdr));

    if (req->backend.peer_addr.ss_family == AF_UNSPEC)
    {
        hdr.msg_name = NULL;
        hdr.msg_namelen = 0;
    }
    else
    {
        hdr.msg_name = &req->backend.peer_addr;
        hdr.msg_namelen = ev__get_addr_len((struct sockaddr*)&req->backend.peer_addr);
    }

    return ev__send_unix(udp->sock, &req->base, _ev_udp_sendmsg_unix, &hdr);
}

static int _ev_udp_on_io_write_unix(ev_udp_t* udp)
{
    int ret = EV_SUCCESS;
    ev_list_node_t* it;
    while ((it = ev_list_begin(&udp->send_list)) != NULL)
    {
        ev_udp_write_t* req = container_of(it, ev_udp_write_t, base.node);

        if ((ret = _ev_udp_do_sendmsg_unix(udp, req)) != EV_SUCCESS)
        {
            break;
        }

        ev_list_erase(&udp->send_list, it);
        _ev_udp_w_user_callback_unix(req, req->base.data.size, EV_SUCCESS);
    }

    if (ret == EV_EAGAIN)
    {
        ret = EV_SUCCESS;
    }
    return ret;
}

static int _ev_udp_do_recvmsg_unix(ev_udp_t* udp, ev_udp_read_t* req)
{
    struct msghdr hdr;
    memset(&hdr, 0, sizeof(hdr));
    memset(&req->addr, 0, sizeof(req->addr));

    hdr.msg_name = &req->addr;
    hdr.msg_namelen = sizeof(req->addr);
    hdr.msg_iov = (struct iovec*)req->base.data.bufs;
    hdr.msg_iovlen = req->base.data.nbuf;

    ssize_t read_size;
    do
    {
        read_size = recvmsg(udp->sock, &hdr, 0);
    } while (read_size < 0 && errno == EINTR);

    if (read_size < 0)
    {
        int err = errno;
        if (err == EAGAIN || err == EWOULDBLOCK)
        {
            return EV_EAGAIN;
        }
        return ev__translate_sys_error(err);
    }

    req->base.data.size += read_size;
    return EV_SUCCESS;
}

static int _ev_udp_on_io_read_unix(ev_udp_t* udp)
{
    int ret = EV_SUCCESS;
    ev_list_node_t* it;
    while ((it = ev_list_begin(&udp->recv_list)) != NULL)
    {
        ev_udp_read_t* req = container_of(it, ev_udp_read_t, base.node);

        if ((ret = _ev_udp_do_recvmsg_unix(udp, req)) != EV_SUCCESS)
        {
            break;
        }

        ev_list_erase(&udp->recv_list, it);
        _ev_udp_r_user_callback_unix(req, req->base.data.size, EV_SUCCESS);
    }

    if (ret == EV_EAGAIN)
    {
        ret = EV_SUCCESS;
    }
    return ret;
}

static void _ev_udp_smart_deactive_unix(ev_udp_t* udp)
{
    int flag_send_idle = 0;
    int flag_recv_idle = 0;
    if (ev_list_size(&udp->send_list) == 0)
    {
        flag_send_idle = 1;
        ev__nonblock_io_del(udp->base.data.loop, &udp->backend.io, EPOLLOUT);
    }
    if (ev_list_size(&udp->recv_list) == 0)
    {
        flag_recv_idle = 1;
        ev__nonblock_io_del(udp->base.data.loop, &udp->backend.io, EPOLLIN);
    }

    if (flag_recv_idle && flag_send_idle)
    {
        ev__handle_deactive(&udp->base);
    }
}

static void _ev_udp_on_io_unix(ev_nonblock_io_t* io, unsigned evts, void* arg)
{
    (void)arg;
    int ret;
    ev_udp_t* udp = container_of(io, ev_udp_t, backend.io);

    if (evts & EPOLLOUT)
    {
        if ((ret = _ev_udp_on_io_write_unix(udp)) != EV_SUCCESS)
        {
            goto err;
        }
    }

    if (evts & EPOLLIN)
    {
        if ((ret = _ev_udp_on_io_read_unix(udp)) != EV_SUCCESS)
        {
            goto err;
        }
    }

    _ev_udp_smart_deactive_unix(udp);

    return;

err:
    _ev_udp_abort_unix(udp, ret);
}

static int _ev_udp_maybe_deferred_socket_unix(ev_udp_t* udp, int domain)
{
    if (udp->sock != EV_OS_SOCKET_INVALID)
    {
        return EV_SUCCESS;
    }

    if ((udp->sock = socket(domain, SOCK_DGRAM, 0)) < 0)
    {
        int ret = errno;
        return ev__translate_sys_error(ret);
    }

    ev__nonblock_io_init(&udp->backend.io, udp->sock, _ev_udp_on_io_unix, NULL);

    return EV_SUCCESS;
}

static int _ev_udp_do_connect_unix(ev_udp_t* udp, const struct sockaddr* addr)
{
    int ret;
    socklen_t addrlen = ev__get_addr_len(addr);
    if (addrlen == (socklen_t)-1)
    {
        return EV_EINVAL;
    }

    if ((ret = _ev_udp_maybe_deferred_socket_unix(udp, addr->sa_family)) != EV_SUCCESS)
    {
        return ret;
    }

    do 
    {
        ret = connect(udp->sock, addr, addrlen);
    } while (ret == -1 && errno == EINTR);

    if (ret != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    udp->base.data.flags |= EV_UDP_CONNECTED;
    return EV_SUCCESS;
}

static int _ev_udp_maybe_deferred_bind_unix(ev_udp_t* udp, int domain, int flags)
{
    int ret = _ev_udp_maybe_deferred_socket_unix(udp, domain);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    if (udp->base.data.flags & EV_UDP_BOUND)
    {
        return EV_SUCCESS;
    }

    struct sockaddr_storage addr;
    struct sockaddr_in* addr4 = (struct sockaddr_in*)&addr;
    struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&addr;

    switch (domain)
    {
    case AF_INET:
    {
        memset(addr4, 0, sizeof(*addr4));
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY;
        return ev_udp_bind(udp, (struct sockaddr*)addr4, flags);
    }

    case AF_INET6:
    {
        memset(addr6, 0, sizeof(*addr6));
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any;
        return ev_udp_bind(udp, (struct sockaddr*)addr6, flags);
    }

    default:
        break;
    }

    abort();
}

static int _ev_udp_convert_interface_addr4_unix(struct ip_mreq* dst,
    const struct sockaddr_in* multicast_addr, const char* interface_addr)
{
    int ret;
    memset(dst, 0, sizeof(*dst));

    if (interface_addr != NULL)
    {
        if ((ret = inet_pton(AF_INET, interface_addr, &dst->imr_interface.s_addr)) != 1)
        {
            int ret = errno;
            return ev__translate_sys_error(ret);
        }
    }
    else
    {
        dst->imr_interface.s_addr = htonl(INADDR_ANY);
    }

    dst->imr_multiaddr.s_addr = multicast_addr->sin_addr.s_addr;

    return EV_SUCCESS;
}

static int _ev_udp_setmembership4_unix(ev_udp_t* udp, struct sockaddr_in* multicast_addr,
    const char* interface_addr, ev_udp_membership_t membership)
{
    int ret;

    struct ip_mreq mreq;
    if ((ret = _ev_udp_convert_interface_addr4_unix(&mreq, multicast_addr, interface_addr)) != EV_SUCCESS)
    {
        return ret;
    }

    int optname = membership == EV_UDP_ENTER_GROUP ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP;
    if (setsockopt(udp->sock, IPPROTO_IP, optname, &mreq, sizeof(mreq)) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

static void _ev_udp_convert_interface_addr6_unix(struct ipv6_mreq* dst,
    struct sockaddr_in6* multicast_addr, const char* interface_addr)
{
    memset(dst, 0, sizeof(*dst));

    if (interface_addr != 0)
    {
        struct sockaddr_in6 addr6;
        ev_ipv6_addr(interface_addr, 0, &addr6);

        dst->ipv6mr_interface = addr6.sin6_scope_id;
    }
    else
    {
        dst->ipv6mr_interface = 0;
    }
    dst->ipv6mr_multiaddr = multicast_addr->sin6_addr;
}

static int _ev_udp_setmembership6_unix(ev_udp_t* udp, struct sockaddr_in6* multicast_addr,
    const char* interface_addr, ev_udp_membership_t membership)
{
    int ret;
    struct ipv6_mreq mreq;
    _ev_udp_convert_interface_addr6_unix(&mreq, multicast_addr, interface_addr);

    int optname = membership == EV_UDP_ENTER_GROUP ? IPV6_ADD_MEMBERSHIP : IPV6_DROP_MEMBERSHIP;
    if (setsockopt(udp->sock, IPPROTO_IPV6, optname, &mreq, sizeof(mreq)) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

static int _ev_udp_convert_source_interface_addr4_unix(struct ip_mreq_source* dst,
    const char* interface_addr, const struct sockaddr_in* multicast_addr, const struct sockaddr_in* source_addr)
{
    int ret;
    memset(dst, 0, sizeof(*dst));

    if (interface_addr != NULL)
    {
        if ((ret = inet_pton(AF_INET, interface_addr, &dst->imr_interface.s_addr)) != 1)
        {
            ret = errno;
            return ev__translate_sys_error(ret);
        }
    }
    else
    {
        dst->imr_interface.s_addr = htonl(INADDR_ANY);
    }

    dst->imr_multiaddr.s_addr = multicast_addr->sin_addr.s_addr;
    dst->imr_sourceaddr.s_addr = source_addr->sin_addr.s_addr;

    return EV_SUCCESS;
}

static int _ev_udp_set_source_membership4_unix(ev_udp_t* udp, const struct sockaddr_in* multicast_addr,
    const char* interface_addr, const struct sockaddr_in* source_addr, ev_udp_membership_t membership)
{
    int ret = _ev_udp_maybe_deferred_bind_unix(udp, AF_INET, EV_UDP_REUSEADDR);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    struct ip_mreq_source mreq;
    if ((ret = _ev_udp_convert_source_interface_addr4_unix(&mreq, interface_addr,
        multicast_addr, source_addr)) != EV_SUCCESS)
    {
        return ret;
    }

    int optname = membership == EV_UDP_ENTER_GROUP ? IP_ADD_SOURCE_MEMBERSHIP : IP_DROP_SOURCE_MEMBERSHIP;
    if (setsockopt(udp->sock, IPPROTO_IP, optname, &mreq, sizeof(mreq)) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

static int _ev_udp_convert_source_interface_addr6_unix(struct group_source_req* dst,
    const char* interface_addr, const struct sockaddr_in6* multicast_addr,
    const struct sockaddr_in6* source_addr)
{
    int ret;
    struct sockaddr_in6 addr_6;

    memset(dst, 0, sizeof(*dst));

    if (interface_addr != NULL)
    {
        if ((ret = ev_ipv6_addr(interface_addr, 0, &addr_6)) != EV_SUCCESS)
        {
            return ret;
        }
        dst->gsr_interface = addr_6.sin6_scope_id;
    }
    else
    {
        dst->gsr_interface = 0;
    }

    memcpy(&dst->gsr_group, multicast_addr, sizeof(*multicast_addr));
    memcpy(&dst->gsr_source, source_addr, sizeof(*source_addr));

    return EV_SUCCESS;
}

static int _ev_udp_set_source_membership6_unix(ev_udp_t* udp, const struct sockaddr_in6* multicast_addr,
    const char* interface_addr, const struct sockaddr_in6* source_addr, ev_udp_membership_t membership)
{
    int ret = _ev_udp_maybe_deferred_bind_unix(udp, AF_INET6, EV_UDP_REUSEADDR);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    struct group_source_req mreq;
    if ((ret = _ev_udp_convert_source_interface_addr6_unix(&mreq, interface_addr,
        multicast_addr, source_addr)) != EV_SUCCESS)
    {
        return ret;
    }

    int optname = membership == EV_UDP_ENTER_GROUP ? MCAST_JOIN_SOURCE_GROUP : MCAST_LEAVE_SOURCE_GROUP;
    if (setsockopt(udp->sock, IPPROTO_IPV6, optname, &mreq, sizeof(mreq)) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

static int _ev_udp_set_ttl_unix(ev_udp_t* udp, int ttl, int option4, int option6)
{
    int level;
    int option;
    if (udp->base.data.flags & EV_UDP_IPV6)
    {
        level = IPPROTO_IPV6;
        option = option6;
    }
    else
    {
        level = IPPROTO_IP;
        option = option4;
    }

    void* optval = &ttl;
    socklen_t optlen = sizeof(ttl);

    /**
     * On Solaris and derivatives such as SmartOS, the length of socket options
     * is sizeof(int) for IPV6_MULTICAST_LOOP and sizeof(char) for
     * IP_MULTICAST_LOOP, so hardcode the size of the option in the IPv6 case,
     * and use the general uv__setsockopt_maybe_char call otherwise.
     */
#if defined(__sun) || defined(_AIX) || defined(__OpenBSD__) || defined(__MVS__) || defined(__QNX__)
    char char_val = ttl;
    if (!(udp->base.data.flags & EV_UDP_IPV6))
    {
        optval = &char_val;
        optlen = sizeof(char_val);
    }
#endif

    if (setsockopt(udp->sock, level, option, optval, optlen) != 0)
    {
        int ret = errno;
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

int ev__udp_recv(ev_udp_t* udp, ev_udp_read_t* req)
{
    (void)req;
    if (ev_list_size(&udp->recv_list) == 1)
    {
        ev__nonblock_io_add(udp->base.data.loop, &udp->backend.io, EPOLLIN);
    }

    ev__handle_active(&udp->base);

    return EV_SUCCESS;
}

int ev__udp_send(ev_udp_t* udp, ev_udp_write_t* req, const struct sockaddr* addr, socklen_t addrlen)
{
    int ret;

    if (addr == NULL)
    {
        req->backend.peer_addr.ss_family = AF_UNSPEC;
    }
    else
    {
        if ((ret = _ev_udp_maybe_deferred_bind_unix(udp, addr->sa_family, 0)) != EV_SUCCESS)
        {
            return ret;
        }

        if (addrlen == (socklen_t)-1)
        {
            return EV_EINVAL;
        }

        memcpy(&req->backend.peer_addr, addr, addrlen);
    }

    if (ev_list_size(&udp->send_list) == 1)
    {
        ev__nonblock_io_add(udp->base.data.loop, &udp->backend.io, EPOLLOUT);
    }

    ev__handle_active(&udp->base);

    return EV_SUCCESS;
}

int ev_udp_init(ev_loop_t* loop, ev_udp_t* udp, int domain)
{
    int err;
    if (domain != AF_INET && domain != AF_INET6 && domain != AF_UNSPEC)
    {
        return EV_EINVAL;
    }

    udp->sock = EV_OS_SOCKET_INVALID;
    if (domain != AF_UNSPEC)
    {
        if ((err = _ev_udp_maybe_deferred_socket_unix(udp, domain)) != EV_SUCCESS)
        {
            return err;
        }
    }

    ev__handle_init(loop, &udp->base, EV_ROLE_EV_UDP, _ev_udp_on_close_unix);
    ev_list_init(&udp->send_list);
    ev_list_init(&udp->recv_list);

    return EV_SUCCESS;
}

int ev_udp_open(ev_udp_t* udp, ev_os_socket_t sock)
{
    int err;
    if (udp->sock != EV_OS_SOCKET_INVALID)
    {
        return EV_EBUSY;
    }

    if ((err = ev__nonblock(sock, 1)) != EV_SUCCESS)
    {
        return err;
    }

    if (_ev_udp_is_connected_unix(sock))
    {
        udp->base.data.flags |= EV_UDP_CONNECTED;
    }

    return EV_SUCCESS;
}

void ev_udp_exit(ev_udp_t* udp, ev_udp_cb close_cb)
{
    _ev_udp_close_unix(udp);
    udp->close_cb = close_cb;
    ev__handle_exit(&udp->base, 0);
}

int ev_udp_bind(ev_udp_t* udp, const struct sockaddr* addr, unsigned flags)
{
    int ret;
    socklen_t addrlen = ev__get_addr_len(addr);
    if (addrlen == (socklen_t)-1)
    {
        return EV_EINVAL;
    }

    if ((ret = _ev_udp_maybe_deferred_socket_unix(udp, addr->sa_family)) != EV_SUCCESS)
    {
        return ret;
    }

    if ((ret = _ev_udp_set_flags_unix(udp, flags)) != EV_SUCCESS)
    {
        return ret;
    }

    if (bind(udp->sock, addr, addrlen) < 0)
    {
        ret = errno;
        if (ret == EAFNOSUPPORT)
        {
            return EV_EINVAL;
        }
        return ev__translate_sys_error(ret);
    }

    if (addr->sa_family == AF_INET6)
    {
        udp->base.data.flags |= EV_UDP_IPV6;
    }

    udp->base.data.flags |= EV_UDP_BOUND;
    return EV_SUCCESS;
}

int ev_udp_connect(ev_udp_t* udp, const struct sockaddr* addr)
{
    if (addr == NULL)
    {
        if (!(udp->base.data.flags & EV_UDP_CONNECTED))
        {
            return EV_ENOTCONN;
        }
        return _ev_udp_disconnect_unix(udp);
    }

    return _ev_udp_do_connect_unix(udp, addr);
}

int ev_udp_getsockname(ev_udp_t* udp, struct sockaddr* name, size_t* len)
{
    socklen_t wrap_len = *len;
    if (getsockname(udp->sock, name, &wrap_len) != 0)
    {
        int err = errno;
        return ev__translate_sys_error(err);
    }

    *len = wrap_len;
    return EV_SUCCESS;
}

int ev_udp_getpeername(ev_udp_t* udp, struct sockaddr* name, size_t* len)
{
    socklen_t wrap_len = *len;
    if (getpeername(udp->sock, name, &wrap_len) != 0)
    {
        int err = errno;
        return ev__translate_sys_error(err);
    }

    *len = wrap_len;
    return EV_SUCCESS;
}

int ev_udp_set_membership(ev_udp_t* udp, const char* multicast_addr,
    const char* interface_addr, ev_udp_membership_t membership)
{
    int ret;
    struct sockaddr_storage addr;

    if (membership != EV_UDP_LEAVE_GROUP && membership != EV_UDP_ENTER_GROUP)
    {
        return EV_EINVAL;
    }

    struct sockaddr_in* addr_4 = (struct sockaddr_in*)&addr;
    memset(addr_4, 0, sizeof(*addr_4));

    if (ev_ipv4_addr(multicast_addr, 0, addr_4) == EV_SUCCESS)
    {
        if ((ret = _ev_udp_maybe_deferred_bind_unix(udp, AF_INET, EV_UDP_REUSEADDR)) != EV_SUCCESS)
        {
            return ret;
        }

        return _ev_udp_setmembership4_unix(udp, addr_4, interface_addr, membership);
    }

    struct sockaddr_in6* addr_6 = (struct sockaddr_in6*)&addr;
    memset(addr_6, 0, sizeof(*addr_6));

    if (ev_ipv6_addr(multicast_addr, 0, addr_6) == EV_SUCCESS)
    {
        if ((ret = _ev_udp_maybe_deferred_bind_unix(udp, AF_INET6, EV_UDP_REUSEADDR)) != EV_SUCCESS)
        {
            return ret;
        }

        return _ev_udp_setmembership6_unix(udp, addr_6, interface_addr, membership);
    }

    return EV_EINVAL;
}

int ev_udp_set_source_membership(ev_udp_t* udp, const char* multicast_addr,
    const char* interface_addr, const char* source_addr, ev_udp_membership_t membership)
{
    int ret;
    struct sockaddr_storage mcast_addr;
    struct sockaddr_storage src_addr;

    if (membership != EV_UDP_LEAVE_GROUP && membership != EV_UDP_ENTER_GROUP)
    {
        return EV_EINVAL;
    }

    struct sockaddr_in* mcast_addr_4 = (struct sockaddr_in*)&mcast_addr;
    if ((ret = ev_ipv4_addr(multicast_addr, 0, mcast_addr_4)) == EV_SUCCESS)
    {
        struct sockaddr_in* src_addr_4 = (struct sockaddr_in*)&src_addr;
        if ((ret = ev_ipv4_addr(source_addr, 0, src_addr_4)) != EV_SUCCESS)
        {
            return ret;
        }

        return _ev_udp_set_source_membership4_unix(udp, mcast_addr_4, interface_addr, src_addr_4, membership);
    }

    struct sockaddr_in6* mcast_addr_6 = (struct sockaddr_in6*)&mcast_addr;
    if ((ret = ev_ipv6_addr(multicast_addr, 0, mcast_addr_6)) == EV_SUCCESS)
    {
        struct sockaddr_in6* src_addr_6 = (struct sockaddr_in6*)&src_addr;
        if ((ret = ev_ipv6_addr(source_addr, 0, src_addr_6)) != EV_SUCCESS)
        {
            return ret;
        }

        return _ev_udp_set_source_membership6_unix(udp, mcast_addr_6, interface_addr, src_addr_6, membership);
    }

    return ret;
}

int ev_udp_set_multicast_loop(ev_udp_t* udp, int on)
{
    return _ev_udp_set_ttl_unix(udp, on, IP_MULTICAST_LOOP, IPV6_MULTICAST_LOOP);
}

int ev_udp_set_multicast_ttl(ev_udp_t* udp, int ttl)
{
    return _ev_udp_set_ttl_unix(udp, ttl, IP_MULTICAST_TTL, IPV6_MULTICAST_HOPS);
}

int ev_udp_set_multicast_interface(ev_udp_t* udp, const char* interface_addr)
{
    int ret;
    struct sockaddr_storage addr_st;
    struct sockaddr_in* addr_4 = (struct sockaddr_in*)&addr_st;
    struct sockaddr_in6* addr_6 = (struct sockaddr_in6*)&addr_st;

    int is_ipv6 = udp->base.data.flags & EV_UDP_IPV6;
    if ((ret = ev__udp_interface_addr_to_sockaddr(&addr_st, interface_addr, is_ipv6)) != EV_SUCCESS)
    {
        return ret;
    }

    int level;
    int optname;
    void* optval;
    socklen_t optlen;

    if (addr_st.ss_family == AF_INET)
    {
        level = IPPROTO_IP;
        optname = IP_MULTICAST_IF;
        optval = &addr_4->sin_addr;
        optlen = sizeof(addr_4->sin_addr);
    }
    else if(addr_st.ss_family == AF_INET6)
    {
        level = IPPROTO_IPV6;
        optname = IPV6_MULTICAST_IF;
        optval = &addr_6->sin6_scope_id;
        optlen = sizeof(addr_6->sin6_scope_id);
    }
    else
    {
        abort();
    }

    if (setsockopt(udp->sock, level, optname, optval, optlen) != 0)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

int ev_udp_set_broadcast(ev_udp_t* udp, int on)
{
    int err;
    if (setsockopt(udp->sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) != 0)
    {
        err = errno;
        return ev__translate_sys_error(err);
    }

    return EV_SUCCESS;
}

int ev_udp_set_ttl(ev_udp_t* udp, int ttl)
{
    if (ttl < 1 || ttl > 255)
    {
        return EV_EINVAL;
    }

#if defined(__MVS__)
    /* zOS does not support setting ttl for IPv4 */
    if (!(udp->base.data.flags & EV_UDP_IPV6))
    {
        return EV_ENOTSUP;
    }
#endif

    return _ev_udp_set_ttl_unix(udp, ttl, IP_TTL, IPV6_UNICAST_HOPS);
}
