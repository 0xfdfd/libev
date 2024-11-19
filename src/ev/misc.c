#include <assert.h>
#include <string.h>

#if !defined(_WIN32)
#   include <net/if.h>
#   include <sys/un.h>
#endif

static void _ev_set_scope_id(struct sockaddr_in6* addr, const char* ip)
{
    const char* zone_index = strchr(ip, '%');
    if (zone_index == NULL)
    {
        return;
    }

    char address_part[40];
    size_t address_part_size = zone_index - ip;
    if (address_part_size >= sizeof(address_part))
    {
        address_part_size = sizeof(address_part) - 1;
    }

    memcpy(address_part, ip, address_part_size);
    address_part[address_part_size] = '\0';
    ip = address_part;

    zone_index++; /* skip '%' */
    /* NOTE: unknown interface (id=0) is silently ignored */
#ifdef _WIN32
    addr->sin6_scope_id = atoi(zone_index);
#else
    addr->sin6_scope_id = if_nametoindex(zone_index);
#endif
}

static void _ev_random_on_work(ev_work_t* work)
{
    ev_random_req_t* req = EV_CONTAINER_OF(work, ev_random_req_t, work);
    req->ret = ev__random(req->buf, req->len);
}

static void _ev_random_on_done(ev_work_t* work, int status)
{
    ev_random_req_t* req = EV_CONTAINER_OF(work, ev_random_req_t, work);
    status = status != 0 ? status : req->ret;
    req->cb(req, status, req->buf, req->len);
}

int ev_ipv4_addr(const char* ip, int port, struct sockaddr_in* addr)
{
    memset(addr, 0, sizeof(*addr));

    addr->sin_family = AF_INET;
    addr->sin_port = htons((uint16_t)port);

    return inet_pton(AF_INET, ip, &addr->sin_addr) ? 0 : EV_EINVAL;
}

int ev_ipv6_addr(const char* ip, int port, struct sockaddr_in6* addr)
{
    memset(addr, 0, sizeof(*addr));

    addr->sin6_family = AF_INET6;
    addr->sin6_port = htons((uint16_t)port);
    _ev_set_scope_id(addr, ip);

    return inet_pton(AF_INET6, ip, &addr->sin6_addr) ? 0 : EV_EINVAL;
}

int ev_ipv4_name(const struct sockaddr_in* addr, int* port, char* ip, size_t len)
{
    if (port != NULL)
    {
        *port = ntohs(addr->sin_port);
    }

    if (ip != NULL)
    {
        return inet_ntop(AF_INET, &addr->sin_addr, ip, len) != NULL ?
            0 : EV_ENOSPC;
    }

    return 0;
}

int ev_ipv6_name(const struct sockaddr_in6* addr, int* port, char* ip, size_t len)
{
    if (port != NULL)
    {
        *port = ntohs(addr->sin6_port);
    }

    if (ip != NULL)
    {
        return inet_ntop(AF_INET6, &addr->sin6_addr, ip, len) != NULL ?
            0 : EV_ENOSPC;
    }

    return 0;
}

int ev_ip_addr(const char* ip, int port, struct sockaddr* addr, size_t size)
{
    if (strstr(ip, ":") != NULL)
    {
        if (size < sizeof(struct sockaddr_in6))
        {
            return EV_EINVAL;
        }
        return ev_ipv6_addr(ip, port, (struct sockaddr_in6*)addr);
    }

    if (size < sizeof(struct sockaddr_in))
    {
        return EV_EINVAL;
    }
    return ev_ipv4_addr(ip, port, (struct sockaddr_in*)addr);
}

int ev_ip_name(const struct sockaddr* addr, int* port, char* ip, size_t len)
{
    if (addr->sa_family == AF_INET)
    {
        return ev_ipv4_name((struct sockaddr_in*)addr, port, ip, len);
    }
    return ev_ipv6_name((struct sockaddr_in6*)addr, port, ip, len);
}

ev_buf_t ev_buf_make(void* buf, size_t len)
{
    ev_buf_t tmp;

#if defined(_WIN32)
    tmp.data = (CHAR*)buf;
    tmp.size = (ULONG)len;
#else
    tmp.data = buf;
    tmp.size = len;
#endif

    return tmp;
}

void ev_buf_make_n(ev_buf_t bufs[], size_t nbuf, ...)
{
    va_list ap;
    va_start(ap, nbuf);
    ev_buf_make_v(bufs, nbuf, ap);
    va_end(ap);
}

void ev_buf_make_v(ev_buf_t bufs[], size_t nbuf, va_list ap)
{
    size_t i;
    for (i = 0; i < nbuf; i++)
    {
        void* v_b = va_arg(ap, void*);
        size_t v_l = va_arg(ap, size_t);
        bufs[i] = ev_buf_make(v_b, v_l);
    }
}

EV_LOCAL int ev__translate_posix_sys_error(int syserr)
{
#define EV_EXPAND_ERRMAP(err, syserr, str) case syserr: return err;

    switch (syserr)
    {
    /* Success */
    case 0:                 return 0;
#if EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:       return EV_EAGAIN;
#endif
    EV_ERRNO_POSIX_MAP(EV_EXPAND_ERRMAP);

    /* Unknown */
    default:                break;
    }

    EV_ABORT("unknown system errno %d.", syserr);

#undef EV_EXPAND_ERRMAP
}

void ev_library_shutdown(void)
{
    ev_threadpool_default_cleanup();
    ev__atomic_exit();
}

EV_API int ev_random(ev_loop_t* loop, ev_random_req_t* req, void* buf,
    size_t len, int flags, ev_random_cb cb)
{
    if (loop == NULL)
    {
        return ev__random(buf, len);
    }

    req->buf = buf;
    req->len = len;
    req->flags = flags;
    req->cb = cb;
    req->ret = 0;
    return ev_loop_queue_work(loop, &req->work, _ev_random_on_work, _ev_random_on_done);
}
