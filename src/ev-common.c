#include <assert.h>
#include <string.h>
#include "ev-common.h"
#include "ev.h"

typedef struct ev_strerror_pair
{
    int             errn;   /**< Error number */
    const char*     info;   /**< Error string */
}ev_strerror_pair_t;

static int _ev_cmp_timer(const ev_map_node_t* key1, const ev_map_node_t* key2, void* arg)
{
    (void)arg;
    ev_timer_t* t1 = container_of(key1, ev_timer_t, node);
    ev_timer_t* t2 = container_of(key2, ev_timer_t, node);

    if (t1->data.active == t2->data.active)
    {
        if (t1 == t2)
        {
            return 0;
        }
        return t1 < t2 ? -1 : 1;
    }

    return t1->data.active < t2->data.active ? -1 : 1;
}

static void _ev_loop_init(ev_loop_t* loop)
{
    loop->hwtime = 0;
    ev_list_init(&loop->handles.idle_list);
    ev_list_init(&loop->handles.active_list);

    ev_map_init(&loop->timer.heap, _ev_cmp_timer, NULL);
    ev_list_init(&loop->todo.pending);

    memset(&loop->mask, 0, sizeof(loop->mask));
}

/**
 * @return bool. non-zero for have active events.
 */
static int _ev_loop_alive(ev_loop_t* loop)
{
    return ev_list_size(&loop->handles.active_list)
        || ev_list_size(&loop->todo.pending);
}

static int _ev_loop_active_timer(ev_loop_t* loop)
{
    int ret = 0;

    ev_map_node_t* it;
    while ((it = ev_map_begin(&loop->timer.heap)) != NULL)
    {
        ev_timer_t* timer = container_of(it, ev_timer_t, node);
        if (timer->data.active > loop->hwtime)
        {
            break;
        }

        ev_timer_stop(timer);
        if (timer->attr.repeat != 0)
        {
            ev_timer_start(timer, timer->attr.cb, timer->attr.repeat, timer->attr.repeat);
        }
        timer->attr.cb(timer);
        ret++;
    }

    return ret;
}

static void _ev_loop_active_todo(ev_loop_t* loop)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&loop->todo.pending)) != NULL)
    {
        ev_todo_t* todo = container_of(it, ev_todo_t, node);
        todo->mask.queued = 0;
        todo->cb(todo);
    }
}

static uint32_t _ev_backend_timeout_timer(ev_loop_t* loop)
{
    ev_map_node_t* it = ev_map_begin(&loop->timer.heap);
    if (it == NULL)
    {
        return 0;
    }

    ev_timer_t* timer = container_of(it, ev_timer_t, node);
    if (timer->data.active <= loop->hwtime)
    {
        return 0;
    }
    uint64_t dif = timer->data.active - loop->hwtime;
    return dif > UINT32_MAX ? UINT32_MAX : (uint32_t)dif;
}

static uint32_t _ev_backend_timeout(ev_loop_t* loop)
{
    uint32_t ret;
    if (loop->mask.b_stop)
    {
        return 0;
    }

    if (ev_list_size(&loop->todo.pending) != 0)
    {
        return 0;
    }

    if ((ret = _ev_backend_timeout_timer(loop)) != 0)
    {
        return ret;
    }

    /* If no active handle, set timeout to max value */
    return ev_list_size(&loop->handles.active_list) ? 0 : (uint32_t)-1;
}

static void _ev_to_close(ev_todo_t* todo)
{
    ev_handle_t* handle = container_of(todo, ev_handle_t, data.close_queue);

    handle->data.flags &= ~EV_HANDLE_CLOSING;
    handle->data.flags |= EV_HANDLE_CLOSED;

    ev_list_erase(&handle->data.loop->handles.idle_list, &handle->node);
    handle->data.close_cb(handle);
}

int ev_loop_init(ev_loop_t* loop)
{
    _ev_loop_init(loop);

    int ret = ev__loop_init_backend(loop);
    if (ret < 0)
    {
        return ret;
    }

    ev__loop_update_time(loop);
    return EV_SUCCESS;
}

int ev_loop_exit(ev_loop_t* loop)
{
    if (ev_list_size(&loop->handles.active_list)
        || ev_list_size(&loop->todo.pending)
        || ev_map_size(&loop->timer.heap))
    {
        return EV_EBUSY;
    }

    ev__loop_exit_backend(loop);
    return EV_SUCCESS;
}

void ev_loop_stop(ev_loop_t* loop)
{
    loop->mask.b_stop = 1;
}

void ev__handle_init(ev_loop_t* loop, ev_handle_t* handle, ev_role_t role, ev_close_cb close_cb)
{
    assert(close_cb != NULL);
    ev_list_push_back(&loop->handles.idle_list, &handle->node);

    handle->data.loop = loop;
    handle->data.role = role;
    handle->data.flags = 0;

    handle->data.close_cb = close_cb;
    ev__todo_init(&handle->data.close_queue);
}

void ev__handle_exit(ev_handle_t* handle)
{
    if (ev__handle_is_closing(handle))
    {
        BREAK_ABORT();
        return;
    }

    /* Stop if necessary */
    ev__handle_deactive(handle);

    handle->data.flags |= EV_HANDLE_CLOSING;
    ev__todo_queue(handle->data.loop, &handle->data.close_queue, _ev_to_close);
}

void ev__handle_active(ev_handle_t* handle)
{
    if (handle->data.flags & EV_HANDLE_ACTIVE)
    {
        return;
    }
    handle->data.flags |= EV_HANDLE_ACTIVE;

    ev_loop_t* loop = handle->data.loop;
    ev_list_erase(&loop->handles.idle_list, &handle->node);
    ev_list_push_back(&loop->handles.active_list, &handle->node);
}

void ev__handle_deactive(ev_handle_t* handle)
{
    if (!(handle->data.flags & EV_HANDLE_ACTIVE))
    {
        return;
    }
    handle->data.flags &= ~EV_HANDLE_ACTIVE;

    ev_loop_t* loop = handle->data.loop;
    ev_list_erase(&loop->handles.active_list, &handle->node);
    ev_list_push_back(&loop->handles.idle_list, &handle->node);
}

int ev__handle_is_active(ev_handle_t* handle)
{
    return handle->data.flags & EV_HANDLE_ACTIVE;
}

int ev__handle_is_closing(ev_handle_t* handle)
{
    return handle->data.flags & (EV_HANDLE_CLOSING | EV_HANDLE_CLOSED);
}

void ev__todo_init(ev_todo_t* token)
{
    token->mask.queued = 0;
    token->cb = NULL;
}

void ev__todo_queue(ev_loop_t* loop, ev_todo_t* token, ev_todo_cb cb)
{
    assert(!token->mask.queued);

    ev_list_push_back(&loop->todo.pending, &token->node);
    token->mask.queued = 1;
    token->cb = cb;
}

void ev__todo_cancel(ev_loop_t* loop, ev_todo_t* token)
{
    assert(!token->mask.queued);

    ev_list_erase(&loop->todo.pending, &token->node);
    token->mask.queued = 0;
    token->cb = NULL;
}

int ev_loop_run(ev_loop_t* loop, ev_loop_mode_t mode)
{
    uint32_t timeout;

    int ret;
    while ((ret = _ev_loop_alive(loop)) != 0 && !loop->mask.b_stop)
    {
        ev__loop_update_time(loop);

        _ev_loop_active_timer(loop);
        _ev_loop_active_todo(loop);

        if ((ret = _ev_loop_alive(loop)) == 0)
        {
            break;
        }

        /* Calculate timeout */
        timeout = mode != EV_LOOP_MODE_NOWAIT ?
            _ev_backend_timeout(loop) : 0;

        ev__poll(loop, timeout);

        /**
         * #EV_LOOP_MODE_ONCE implies forward progress: at least one callback must have
         * been invoked when it returns. #_ev_poll_win() can return without doing
         * I/O (meaning: no callbacks) when its timeout expires - which means we
         * have pending timers that satisfy the forward progress constraint.
         *
         * #EV_LOOP_MODE_NOWAIT makes no guarantees about progress so it's omitted from
         * the check.
         */
        if (mode == EV_LOOP_MODE_ONCE)
        {
            ev__loop_update_time(loop);
            _ev_loop_active_timer(loop);
        }

        /* Callback maybe added */
        _ev_loop_active_todo(loop);

        if (mode != EV_LOOP_MODE_DEFAULT)
        {
            break;
        }
    }

    /* Prevent #ev_loop_t::mask::b_stop from compile optimization */
    if (loop->mask.b_stop)
    {
        loop->mask.b_stop = 0;
    }

    return ret;
}

int ev_ipv4_addr(const char* ip, int port, struct sockaddr_in* addr)
{
    memset(addr, 0, sizeof(*addr));

    addr->sin_family = AF_INET;
    addr->sin_port = htons((uint16_t)port);

    return inet_pton(AF_INET, ip, &addr->sin_addr) ? EV_SUCCESS : EV_EINVAL;
}

int ev_ipv6_addr(const char* ip, int port, struct sockaddr_in6* addr)
{
    memset(addr, 0, sizeof(*addr));

    addr->sin6_family = AF_INET6;
    addr->sin6_port = htons((uint16_t)port);

    return inet_pton(AF_INET6, ip, &addr->sin6_addr) ? EV_SUCCESS : EV_EINVAL;
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
            EV_SUCCESS : EV_ENOSPC;
    }

    return EV_SUCCESS;
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
            EV_SUCCESS : EV_ENOSPC;
    }

    return EV_SUCCESS;
}

int ev_write_init(ev_write_t* req, ev_buf_t* bufs, size_t nbuf, ev_write_cb cb)
{
    if (nbuf > ARRAY_SIZE(req->data.bufs))
    {
        return EV_E2BIG;
    }

    req->data.cb = cb;
    req->data.nbuf = nbuf;
    memcpy(req->data.bufs, bufs, sizeof(ev_buf_t) * nbuf);

    return EV_SUCCESS;
}

int ev_read_init(ev_read_t* req, ev_buf_t* bufs, size_t nbuf, ev_read_cb cb)
{
    if (nbuf > ARRAY_SIZE(req->data.bufs))
    {
        return EV_E2BIG;
    }

    req->data.cb = cb;
    req->data.nbuf = nbuf;
    memcpy(req->data.bufs, bufs, sizeof(ev_buf_t) * nbuf);

    return  EV_SUCCESS;
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

const char* ev_strerror(int err)
{
    switch (err)
    {
        /* Success */
    case EV_SUCCESS:            return "Operation success";

        /* Posix compatible error code */
    case EV_EPERM:              return "Operation not permitted";
    case EV_ENOENT:             return "No such file or directory";
    case EV_EIO:                return "Host is unreachable";
    case EV_E2BIG:              return "Argument list too long";
    case EV_EBADF:              return "Bad file descriptor";
    case EV_EAGAIN:             return "Resource temporarily unavailable";
    case EV_ENOMEM:             return "EV_ENOMEM";
    case EV_EACCES:             return "Permission denied";
    case EV_EFAULT:             return "Bad address";
    case EV_EBUSY:              return "Device or resource busy";
    case EV_EEXIST:             return "File exists";
    case EV_EXDEV:              return "Improper link";
    case EV_EISDIR:             return "Is a directory";
    case EV_EINVAL:             return "Invalid argument";
    case EV_EMFILE:             return "Too many open files";
    case EV_ENOSPC:             return "No space left on device";
    case EV_EROFS:              return "Read-only filesystem";
    case EV_EPIPE:              return "Broken pipe";
    case EV_ENAMETOOLONG:       return "Filename too long";
    case EV_ENOTEMPTY:          return "Directory not empty";
    case EV_EADDRINUSE:         return "Address already in use";
    case EV_EADDRNOTAVAIL:      return "Address not available";
    case EV_EAFNOSUPPORT:       return "Address family not supported";
    case EV_EALREADY:           return "Connection already in progress";
    case EV_ECANCELED:          return "Operation canceled";
    case EV_ECONNABORTED:       return "Connection aborted";
    case EV_ECONNREFUSED:       return "Connection refused";
    case EV_ECONNRESET:         return "Connection reset";
    case EV_EHOSTUNREACH:       return "Host is unreachable";
    case EV_EINPROGRESS:        return "Operation in progress";
    case EV_EISCONN:            return "Socket is connected";
    case EV_ELOOP:              return "Too many levels of symbolic links";
    case EV_EMSGSIZE:           return "Message too long";
    case EV_ENETUNREACH:        return "Network unreachable";
    case EV_ENOBUFS:            return "No buffer space available";
    case EV_ENOTCONN:           return "The socket is not connected";
    case EV_ENOTSOCK:           return "Not a socket";
    case EV_ENOTSUP:            return "Operation not supported";
    case EV_EPROTONOSUPPORT:    return "Protocol not supported";
    case EV_ETIMEDOUT:          return "Operation timed out";

        /* Extend error code */
    case EV_EOF:                return "End of file";
    case EV_UNKNOWN:            return "Unknown error";

        /* Unknown error */
    default:                    break;
    }
    return "Unknown error";
}
