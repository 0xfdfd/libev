#include "win/loop.h"
#include "win/winapi.h"
#include "udp-common.h"

static int _ev_udp_setup_socket_attribute_win(ev_loop_t* loop, ev_udp_t* udp, int family)
{
    DWORD yes = 1;
    int ret;

    assert(udp->sock != EV_OS_SOCKET_INVALID);

    /* Set the socket to nonblocking mode */
    if (ioctlsocket(udp->sock, FIONBIO, &yes) == SOCKET_ERROR)
    {
        ret = WSAGetLastError();
        goto err;
    }

    /* Make the socket non-inheritable */
    if (!SetHandleInformation((HANDLE)udp->sock, HANDLE_FLAG_INHERIT, 0))
    {
        ret = GetLastError();
        goto err;
    }

    /**
     * Associate it with the I/O completion port. Use uv_handle_t pointer as
     * completion key.
     */
    if (CreateIoCompletionPort((HANDLE)udp->sock, loop->backend.iocp, (ULONG_PTR)udp->sock, 0) == NULL)
    {
        ret = GetLastError();
        goto err;
    }

    if (family == AF_INET6)
    {
        udp->base.data.flags |= EV_HANDLE_UDP_IPV6;
    }

    return EV_SUCCESS;

err:
    return ev__translate_sys_error(ret);
}

static void _ev_udp_on_close_win(ev_handle_t* handle)
{
    ev_udp_t* udp = EV_CONTAINER_OF(handle, ev_udp_t, base);
    if (udp->close_cb != NULL)
    {
        udp->close_cb(udp);
    }
}

static int _ev_udp_is_bound_win(ev_udp_t* udp)
{
    struct sockaddr_storage addr;
    size_t addrlen = sizeof(addr);

    int ret = ev_udp_getsockname(udp, (struct sockaddr*)&addr, &addrlen);
    return ret == EV_SUCCESS && addrlen > 0;
}

static int _ev_udp_is_connected_win(ev_udp_t* udp)
{
    struct sockaddr_storage addr;
    size_t addrlen = sizeof(addr);

    int ret = ev_udp_getpeername(udp, (struct sockaddr*)&addr, &addrlen);
    return ret == EV_SUCCESS && addrlen > 0;
}

static int _ev_udp_maybe_deferred_socket_win(ev_udp_t* udp, int domain)
{
    int ret;
    if (udp->sock != EV_OS_SOCKET_INVALID)
    {
        return EV_SUCCESS;
    }

    if ((udp->sock = socket(domain, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    if ((ret = _ev_udp_setup_socket_attribute_win(udp->base.data.loop, udp, domain)) != EV_SUCCESS)
    {
        closesocket(udp->sock);
        udp->sock = EV_OS_SOCKET_INVALID;
        return ret;
    }

    return EV_SUCCESS;
}

static void _ev_udp_close_win(ev_udp_t* udp)
{
    if (udp->sock != EV_OS_SOCKET_INVALID)
    {
        closesocket(udp->sock);
        udp->sock = EV_OS_SOCKET_INVALID;
    }
}

static int _ev_udp_disconnect_win(ev_udp_t* udp)
{
    struct sockaddr addr;
    memset(&addr, 0, sizeof(addr));

    int ret = connect(udp->sock, (struct sockaddr*)&addr, sizeof(addr));
    if (ret != 0)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    udp->base.data.flags &= ~EV_HANDLE_UDP_CONNECTED;
    return EV_SUCCESS;
}

static int _ev_udp_maybe_deferred_bind_win(ev_udp_t* udp, int domain)
{
    if (udp->base.data.flags & EV_HANDLE_UDP_BOUND)
    {
        return EV_SUCCESS;
    }

    struct sockaddr* bind_addr;

    if (domain == AF_INET)
    {
        bind_addr = (struct sockaddr*)&g_ev_loop_win_ctx.net.addr_any_ip4;
    }
    else if (domain == AF_INET6)
    {
        bind_addr = (struct sockaddr*)&g_ev_loop_win_ctx.net.addr_any_ip6;
    }
    else
    {
        return EV_EINVAL;
    }

    return ev_udp_bind(udp, bind_addr, 0);
}

static int _ev_udp_do_connect_win(ev_udp_t* udp, const struct sockaddr* addr, socklen_t addrlen)
{
    int ret;

    if ((ret = _ev_udp_maybe_deferred_bind_win(udp, addr->sa_family)) != EV_SUCCESS)
    {
        return ret;
    }

    if ((ret = connect(udp->sock, addr, addrlen)) != 0)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

static int _ev_udp_set_membership_ipv4_win(ev_udp_t* udp,
    const struct sockaddr_in* multicast_addr, const char* interface_addr,
    ev_udp_membership_t membership)
{
    int ret;
    struct ip_mreq mreq;
    memset(&mreq, 0, sizeof(mreq));

    if (interface_addr)
    {
        if (inet_pton(AF_INET, interface_addr, &mreq.imr_interface.s_addr) != 1)
        {
            ret = WSAGetLastError();
            return ev__translate_sys_error(ret);
        }
    }
    else
    {
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    }

    mreq.imr_multiaddr.s_addr = multicast_addr->sin_addr.s_addr;

    int optname = membership == EV_UDP_ENTER_GROUP ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP;
    if (setsockopt(udp->sock, IPPROTO_IP, optname, (char*)&mreq, sizeof(mreq)) != 0)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

static int _ev_udp_set_membership_ipv6_win(ev_udp_t* udp,
    const struct sockaddr_in6* multicast_addr, const char* interface_addr,
    ev_udp_membership_t membership)
{
    int ret;
    struct ipv6_mreq mreq;
    struct sockaddr_in6 addr6;

    memset(&mreq, 0, sizeof(mreq));

    if (interface_addr)
    {
        if (ev_ipv6_addr(interface_addr, 0, &addr6))
        {
            return EV_EINVAL;
        }
        mreq.ipv6mr_interface = addr6.sin6_scope_id;
    }
    else
    {
        mreq.ipv6mr_interface = 0;
    }
    mreq.ipv6mr_multiaddr = multicast_addr->sin6_addr;

    int optname = membership == EV_UDP_ENTER_GROUP ? IPV6_ADD_MEMBERSHIP : IPV6_DROP_MEMBERSHIP;
    if (setsockopt(udp->sock, IPPROTO_IPV6, optname, (char*)&mreq, sizeof(mreq)) != 0)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

static int _ev_udp_set_source_membership_ipv4(ev_udp_t* udp,
    const struct sockaddr_in* multicast_addr, const char* interface_addr,
    const struct sockaddr_in* source_addr, ev_udp_membership_t membership)
{
    int err;
    struct ip_mreq_source mreq;
    memset(&mreq, 0, sizeof(mreq));

    if (interface_addr != NULL)
    {
        if (inet_pton(AF_INET, interface_addr, &mreq.imr_interface.s_addr) != 1)
        {
            err = WSAGetLastError();
            return ev__translate_sys_error(err);
        }
    }
    else
    {
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    }

    mreq.imr_multiaddr.s_addr = multicast_addr->sin_addr.s_addr;
    mreq.imr_sourceaddr.s_addr = source_addr->sin_addr.s_addr;

    int optname = membership == EV_UDP_ENTER_GROUP ? IP_ADD_SOURCE_MEMBERSHIP : IP_DROP_SOURCE_MEMBERSHIP;

    if (setsockopt(udp->sock, IPPROTO_IP, optname, (char*)&mreq, sizeof(mreq)) != 0)
    {
        err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    return EV_SUCCESS;
}

static int _ev_udp_set_source_membership_ipv6(ev_udp_t* udp,
    const struct sockaddr_in6* multicast_addr, const char* interface_addr,
    const struct sockaddr_in6* source_addr, ev_udp_membership_t membership)
{
    int ret;
    struct group_source_req mreq;
    struct sockaddr_in6 addr6;

    memset(&mreq, 0, sizeof(mreq));

    if (interface_addr != NULL)
    {
        if ((ret = ev_ipv6_addr(interface_addr, 0, &addr6)) != EV_SUCCESS)
        {
            return ret;
        }
        mreq.gsr_interface = addr6.sin6_scope_id;
    }
    else
    {
        mreq.gsr_interface = 0;
    }

    memcpy(&mreq.gsr_group, multicast_addr, sizeof(*multicast_addr));
    memcpy(&mreq.gsr_source, source_addr, sizeof(*source_addr));

    int optname = membership == EV_UDP_ENTER_GROUP ? MCAST_JOIN_SOURCE_GROUP : MCAST_LEAVE_SOURCE_GROUP;
    if (setsockopt(udp->sock, IPPROTO_IPV6, optname, (char*)&mreq, sizeof(mreq)) != 0)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

static void _ev_udp_smart_deactive_win(ev_udp_t* udp)
{
    int flag_send_idle = 0;
    int flag_recv_idle = 0;
    if (ev_list_size(&udp->send_list) == 0)
    {
        flag_send_idle = 1;
    }
    if (ev_list_size(&udp->recv_list) == 0)
    {
        flag_recv_idle = 1;
    }

    if (flag_recv_idle && flag_send_idle)
    {
        ev__handle_deactive(&udp->base);
    }
}

static void _ev_udp_w_user_callback_win(ev_udp_write_t* req, size_t size, int stat)
{
    ev__write_exit(&req->base);
    req->usr_cb(req, size, stat);
}

static void _ev_udp_r_user_callback_win(ev_udp_read_t* req, size_t size, int stat)
{
    ev__read_exit(&req->base);
    req->usr_cb(req, size, stat);
}

static void _ev_udp_on_send_complete_win(ev_udp_t* udp, ev_udp_write_t* req)
{
    ev_list_erase(&udp->send_list, &req->base.node);
    _ev_udp_smart_deactive_win(udp);

    _ev_udp_w_user_callback_win(req, req->base.size, req->backend.stat);
}

static void _ev_udp_on_send_bypass_iocp(ev_todo_token_t* todo)
{
    ev_udp_write_t* req = EV_CONTAINER_OF(todo, ev_udp_write_t, backend.token);
    ev_udp_t* udp = req->backend.owner;

    _ev_udp_on_send_complete_win(udp, req);
}

static void _ev_udp_on_send_iocp_win(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    ev_udp_t* udp = arg;
    ev_udp_write_t* req = EV_CONTAINER_OF(iocp, ev_udp_write_t, backend.io);

    req->base.size = transferred;
    req->backend.stat = NT_SUCCESS(iocp->overlapped.Internal) ?
        EV_SUCCESS : ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal));

    _ev_udp_on_send_complete_win(udp, req);
}

static void _ev_udp_do_recv_win(ev_udp_t* udp, ev_udp_read_t* req)
{
    DWORD recv_bytes;
    DWORD flags = 0;
    socklen_t peer_addr_len = sizeof(req->addr);

    int ret = WSARecvFrom(udp->sock, (WSABUF*)req->base.data.bufs, (DWORD)req->base.data.nbuf,
        &recv_bytes, &flags, (struct sockaddr*)&req->addr, &peer_addr_len, NULL, NULL);
    if (ret != SOCKET_ERROR)
    {
        req->base.data.size = recv_bytes;
        req->backend.stat = EV_SUCCESS;
    }
    else
    {
        ret = WSAGetLastError();
        req->backend.stat = ev__translate_sys_error(ret);
    }

    ev_list_erase(&udp->recv_list, &req->base.node);
    _ev_udp_smart_deactive_win(udp);
    _ev_udp_r_user_callback_win(req, req->base.data.size, req->backend.stat);
}

static void _ev_udp_on_recv_iocp_win(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)transferred;
    ev_udp_t* udp = arg;
    ev_udp_read_t* req = EV_CONTAINER_OF(iocp, ev_udp_read_t, backend.io);

    _ev_udp_do_recv_win(udp, req);
}

static void _ev_udp_on_recv_bypass_iocp_win(ev_todo_token_t* todo)
{
    ev_udp_read_t* req = EV_CONTAINER_OF(todo, ev_udp_read_t, backend.token);
    ev_udp_t* udp = req->backend.owner;

    _ev_udp_do_recv_win(udp, req);
}

int ev__udp_recv(ev_udp_t* udp, ev_udp_read_t* req)
{
    WSABUF buf;
    buf.buf = g_ev_loop_win_ctx.net.zero_;
    buf.len = 0;

    DWORD bytes;
    DWORD flags = MSG_PEEK;

    req->backend.owner = udp;
    req->backend.stat = EV_EINPROGRESS;
    ev__iocp_init(&req->backend.io, _ev_udp_on_recv_iocp_win, udp);

    int ret = WSARecv(udp->sock, &buf, 1, &bytes, &flags, &req->backend.io.overlapped, NULL);
    if (ret == 0 && (udp->base.data.flags & EV_HANDLE_UDP_BYPASS_IOCP))
    {
        ev_todo_submit(udp->base.data.loop, &req->backend.token, _ev_udp_on_recv_bypass_iocp_win);
        return EV_SUCCESS;
    }

    int err;
    if (ret == 0 || (err = WSAGetLastError()) == ERROR_IO_PENDING)
    {
        return EV_SUCCESS;
    }

    _ev_udp_smart_deactive_win(udp);
    return ev__translate_sys_error(err);
}

int ev__udp_send(ev_udp_t* udp, ev_udp_write_t* req, const struct sockaddr* addr, socklen_t addrlen)
{
    int ret, err;

    if (!(udp->base.data.flags & EV_HANDLE_UDP_BOUND))
    {
        if (addr == NULL)
        {
            return EV_EINVAL;
        }

        if ((ret = _ev_udp_maybe_deferred_bind_win(udp, addr->sa_family)) != EV_SUCCESS)
        {
            return ret;
        }
    }

    req->backend.owner = udp;
    req->backend.stat = EV_EINPROGRESS;
    ev__iocp_init(&req->backend.io, _ev_udp_on_send_iocp_win, udp);

    DWORD send_bytes;

    ev__handle_active(&udp->base);
    ret = WSASendTo(udp->sock, (WSABUF*)req->base.bufs, (DWORD)req->base.nbuf,
        &send_bytes, 0, addr, addrlen, &req->backend.io.overlapped, NULL);

    if (ret == 0 && (udp->base.data.flags & EV_HANDLE_UDP_BYPASS_IOCP))
    {
        req->base.size += req->base.capacity;
        req->backend.stat = EV_SUCCESS;
        ev_todo_submit(udp->base.data.loop, &req->backend.token, _ev_udp_on_send_bypass_iocp);
        return EV_SUCCESS;
    }

    if (ret == 0 || (err = GetLastError()) == ERROR_IO_PENDING)
    {
        req->backend.stat = EV_EINPROGRESS;
        return EV_SUCCESS;
    }

    _ev_udp_smart_deactive_win(udp);
    return ev__translate_sys_error(err);
}

int ev_udp_init(ev_loop_t* loop, ev_udp_t* udp, int domain)
{
    int err;

    udp->sock = EV_OS_SOCKET_INVALID;
    udp->close_cb = NULL;
    ev_list_init(&udp->send_list);
    ev_list_init(&udp->recv_list);
    ev__handle_init(loop, &udp->base, EV_ROLE_EV_UDP, _ev_udp_on_close_win);

    if (domain != AF_UNSPEC)
    {
        if ((err = _ev_udp_maybe_deferred_socket_win(udp, domain)) != EV_SUCCESS)
        {
            ev__handle_exit(&udp->base, 1);
            return err;
        }
    }

    udp->backend.fn_wsarecvfrom = WSARecvFrom;

    return EV_SUCCESS;
}

void ev_udp_exit(ev_udp_t* udp, ev_udp_cb close_cb)
{
    _ev_udp_close_win(udp);

    udp->close_cb = close_cb;
    ev__handle_exit(&udp->base, 0);
}

int ev_udp_open(ev_udp_t* udp, ev_os_socket_t sock)
{
    int ret;
    if (udp->sock != EV_OS_SOCKET_INVALID)
    {
        return EV_EBUSY;
    }

    WSAPROTOCOL_INFOW protocol_info;
    int opt_len = sizeof(protocol_info);
    if (getsockopt(sock, SOL_SOCKET, SO_PROTOCOL_INFOW, (char*)&protocol_info, &opt_len) != 0)
    {
        int err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    udp->sock = sock;
    if ((ret = _ev_udp_setup_socket_attribute_win(udp->base.data.loop, udp, protocol_info.iAddressFamily)) != EV_SUCCESS)
    {
        udp->sock = EV_OS_SOCKET_INVALID;
        return ret;
    }

    if (_ev_udp_is_bound_win(udp))
    {
        udp->base.data.flags |= EV_HANDLE_UDP_BOUND;
    }

    if (_ev_udp_is_connected_win(udp))
    {
        udp->base.data.flags |= EV_HANDLE_UDP_CONNECTED;
    }

    return EV_SUCCESS;
}

int ev_udp_bind(ev_udp_t* udp, const struct sockaddr* addr, unsigned flags)
{
    int ret;
    if (udp->base.data.flags & EV_HANDLE_UDP_BOUND)
    {
        return EV_EALREADY;
    }

    if ((flags & EV_UDP_IPV6_ONLY) && addr->sa_family != AF_INET6)
    {
        return EV_EINVAL;
    }

    socklen_t addrlen = ev__get_addr_len(addr);
    int flag_have_orig_sock = udp->sock != EV_OS_SOCKET_INVALID;

    if ((ret = _ev_udp_maybe_deferred_socket_win(udp, addr->sa_family)) != EV_SUCCESS)
    {
        return ret;
    }

    if (flags & EV_UDP_REUSEADDR)
    {
        if ((ret = ev__reuse_win(udp->sock, 1)) != EV_SUCCESS)
        {
            return ret;
        }
    }

    if (addr->sa_family == AF_INET6)
    {
        udp->base.data.flags |= EV_HANDLE_UDP_IPV6;
        int is_ipv6_only = flags & EV_UDP_IPV6_ONLY;

        if ((ret = ev__ipv6only_win(udp->sock, is_ipv6_only)) != EV_SUCCESS)
        {
            goto err;
        }
    }

    if (bind(udp->sock, addr, addrlen) == SOCKET_ERROR)
    {
        ret = WSAGetLastError();
        ret = ev__translate_sys_error(ret);
        goto err;
    }

    udp->base.data.flags |= EV_HANDLE_UDP_BOUND;
    return EV_SUCCESS;

err:
    if (flag_have_orig_sock)
    {
        _ev_udp_close_win(udp);
    }
    return ret;
}

int ev_udp_connect(ev_udp_t* udp, const struct sockaddr* addr)
{
    if (addr == NULL)
    {
        if (!(udp->base.data.flags & EV_HANDLE_UDP_CONNECTED))
        {
            return EV_ENOTCONN;
        }

        return _ev_udp_disconnect_win(udp);
    }

    if (udp->base.data.flags & EV_HANDLE_UDP_CONNECTED)
    {
        return EV_EISCONN;
    }

    socklen_t addrlen = ev__get_addr_len(addr);
    if (addrlen == (socklen_t)-1)
    {
        return EV_EINVAL;
    }

    return _ev_udp_do_connect_win(udp, addr, addrlen);
}

int ev_udp_getsockname(ev_udp_t* udp, struct sockaddr* name, size_t* len)
{
    int wrap_len = (int)*len;
    if (getsockname(udp->sock, name, &wrap_len) != 0)
    {
        int err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    *len = wrap_len;
    return EV_SUCCESS;
}

int ev_udp_getpeername(ev_udp_t* udp, struct sockaddr* name, size_t* len)
{
    int wrap_len = (int)*len;
    if (getpeername(udp->sock, name, &wrap_len) != 0)
    {
        int err = WSAGetLastError();
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

    if ((ret = ev_ipv4_addr(multicast_addr, 0, (struct sockaddr_in*)&addr)) == EV_SUCCESS)
    {
        if (udp->base.data.flags & EV_HANDLE_UDP_IPV6)
        {
            return EV_EINVAL;
        }
        if ((ret = _ev_udp_maybe_deferred_bind_win(udp, AF_INET)) != EV_SUCCESS)
        {
            return ret;
        }
        return _ev_udp_set_membership_ipv4_win(udp, (struct sockaddr_in*)&addr,
            interface_addr, membership);
    }

    if ((ret = ev_ipv6_addr(multicast_addr, 0, (struct sockaddr_in6*)&addr)) == EV_SUCCESS)
    {
        if ((udp->base.data.flags & EV_HANDLE_UDP_BOUND) && !(udp->base.data.flags & EV_HANDLE_UDP_IPV6))
        {
            return EV_EINVAL;
        }
        if ((ret = _ev_udp_maybe_deferred_bind_win(udp, AF_INET6)) != EV_SUCCESS)
        {
            return ret;
        }
        return _ev_udp_set_membership_ipv6_win(udp, (struct sockaddr_in6*)&addr,
            interface_addr, membership);
    }

    return ret;
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

    if ((ret = ev_ipv4_addr(multicast_addr, 0, (struct sockaddr_in*)&mcast_addr)) == EV_SUCCESS)
    {
        if ((ret = ev_ipv4_addr(source_addr, 0, (struct sockaddr_in*)&src_addr)) != EV_SUCCESS)
        {
            return ret;
        }
        if (udp->base.data.flags & EV_HANDLE_UDP_IPV6)
        {
            return EV_EINVAL;
        }
        if ((ret = _ev_udp_maybe_deferred_bind_win(udp, AF_INET)) != EV_SUCCESS)
        {
            return ret;
        }
        return _ev_udp_set_source_membership_ipv4(udp, (struct sockaddr_in*)&mcast_addr,
            interface_addr, (struct sockaddr_in*)&src_addr, membership);
    }

    if ((ret = ev_ipv6_addr(multicast_addr, 0, (struct sockaddr_in6*)&mcast_addr)) == EV_SUCCESS)
    {
        if ((ret = ev_ipv6_addr(source_addr, 0, (struct sockaddr_in6*)&src_addr)) != EV_SUCCESS)
        {
            return ret;
        }
        if ((udp->base.data.flags & EV_HANDLE_UDP_BOUND) && !(udp->base.data.flags & EV_HANDLE_UDP_IPV6))
        {
            return EV_EINVAL;
        }
        if ((ret = _ev_udp_maybe_deferred_bind_win(udp, AF_INET6)) != EV_SUCCESS)
        {
            return ret;
        }
        return _ev_udp_set_source_membership_ipv6(udp, (struct sockaddr_in6*)&mcast_addr,
            interface_addr, (struct sockaddr_in6*)&src_addr, membership);
    }

    return ret;
}

int ev_udp_set_multicast_loop(ev_udp_t* udp, int on)
{
    DWORD optval = on;
    if (udp->sock == EV_OS_SOCKET_INVALID)
    {
        return EV_EBADF;
    }

    int level = IPPROTO_IP;
    int optname = IP_MULTICAST_LOOP;
    if (udp->base.data.flags & EV_HANDLE_UDP_IPV6)
    {
        level = IPPROTO_IPV6;
        optname = IPV6_MULTICAST_LOOP;
    }

    if (setsockopt(udp->sock, level, optname, (char*)&optval, sizeof(optval)) != 0)
    {
        int ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

int ev_udp_set_multicast_ttl(ev_udp_t* udp, int ttl)
{
    DWORD optval = (DWORD)ttl;
    if (ttl < -1 || ttl > 255)
    {
        return EV_EINVAL;
    }
    if (udp->sock == EV_OS_SOCKET_INVALID)
    {
        return EV_EBADF;
    }

    int level = IPPROTO_IP;
    int optname = IP_MULTICAST_TTL;
    if (udp->base.data.flags & EV_HANDLE_UDP_IPV6)
    {
        level = IPPROTO_IPV6;
        optname = IPV6_MULTICAST_HOPS;
    }

    if (setsockopt(udp->sock, level, optname, (char*)&optval, sizeof(optval)) != 0)
    {
        int ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

int ev_udp_set_multicast_interface(ev_udp_t* udp, const char* interface_addr)
{
    int ret;
    struct sockaddr_storage addr_st;
    struct sockaddr_in* addr_4 = (struct sockaddr_in*)&addr_st;
    struct sockaddr_in6* addr_6 = (struct sockaddr_in6*)&addr_st;

    if (udp->sock == EV_OS_SOCKET_INVALID)
    {
        return EV_EBADF;
    }

    int is_ipv6 = udp->base.data.flags & EV_HANDLE_UDP_IPV6;
    if ((ret = ev__udp_interface_addr_to_sockaddr(&addr_st, interface_addr, is_ipv6)) != EV_SUCCESS)
    {
        return ret;
    }

    int level = IPPROTO_IP;
    int optname = IP_MULTICAST_IF;
    char* optval = (char*)&addr_4->sin_addr;
    int optlen = sizeof(addr_4->sin_addr);
    if (addr_st.ss_family == AF_INET6)
    {
        optval = (char*)&addr_6->sin6_scope_id;
        optlen = sizeof(addr_6->sin6_scope_id);
    }

    if (setsockopt(udp->sock, level, optname, optval, optlen) != 0)
    {
        ret = WSAGetLastError();
        return ev__translate_sys_error(ret);
    }

    return EV_SUCCESS;
}

int ev_udp_set_broadcast(ev_udp_t* udp, int on)
{
    BOOL optval = !!on;

    if (udp->sock == EV_OS_SOCKET_INVALID)
    {
        return EV_EBADF;
    }

    if (setsockopt(udp->sock, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(optval)) != 0)
    {
        int err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    return EV_SUCCESS;
}

int ev_udp_set_ttl(ev_udp_t* udp, int ttl)
{
    DWORD optval = ttl;
    if (optval < 1 || optval > 255)
    {
        return EV_EINVAL;
    }
    if (udp->sock == EV_OS_SOCKET_INVALID)
    {
        return EV_EBADF;
    }

    int level = IPPROTO_IP;
    int optname = IP_TTL;
    if (udp->base.data.flags & EV_HANDLE_UDP_IPV6)
    {
        level = IPPROTO_IPV6;
        optname = IPV6_HOPLIMIT;
    }

    if (setsockopt(udp->sock, level, optname, (char*)&optval, sizeof(optval)) != 0)
    {
        int err = WSAGetLastError();
        return ev__translate_sys_error(err);
    }

    return EV_SUCCESS;
}
