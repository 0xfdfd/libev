#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#if !defined(_WIN32)
#   include <net/if.h>
#   include <sys/un.h>
#endif

typedef struct ev_strerror_pair
{
    int             errn;           /**< Error number */
    const char*     info;           /**< Error string */
}ev_strerror_pair_t;

static int _ev_loop_init(ev_loop_t* loop)
{
    memset(loop, 0, sizeof(*loop));

    loop->hwtime = 0;
    ev_list_init(&loop->handles.idle_list);
    ev_list_init(&loop->handles.active_list);

    ev_list_init(&loop->backlog_queue);
    ev_list_init(&loop->endgame_queue);

    ev__init_timer(loop);

    loop->threadpool.pool = NULL;
    loop->threadpool.node = (ev_list_node_t)EV_LIST_NODE_INIT;
    ev_mutex_init(&loop->threadpool.mutex, 0);
    ev_list_init(&loop->threadpool.work_queue);

    ev__loop_link_to_default_threadpool(loop);

    return 0;
}

static void _ev_loop_exit(ev_loop_t* loop)
{
    ev_mutex_exit(&loop->threadpool.mutex);
    ev_loop_unlink_threadpool(loop);
}

/**
 * @return bool. non-zero for have active events.
 */
static int _ev_loop_alive(ev_loop_t* loop)
{
    return ev_list_size(&loop->handles.active_list)
        || ev_list_size(&loop->backlog_queue)
        || ev_list_size(&loop->endgame_queue);
}

static uint32_t _ev_backend_timeout_timer(ev_loop_t* loop)
{
    ev_map_node_t* it = ev_map_begin(&loop->timer.heap);
    if (it == NULL)
    {
        return (uint32_t)-1;
    }

    ev_timer_t* timer = EV_CONTAINER_OF(it, ev_timer_t, node);
    if (timer->data.active <= loop->hwtime)
    {
        return 0;
    }
    uint64_t dif = timer->data.active - loop->hwtime;
    return dif > UINT32_MAX ? UINT32_MAX : (uint32_t)dif;
}

/**
 * @brief Calculate wait timeout
 * 
 * Calculate timeout as small as posibile.
 * 
 * @param[in] loop  Event loop
 * @return          Timeout in milliseconds
 */
static uint32_t _ev_backend_timeout(ev_loop_t* loop)
{
    if (loop->mask.b_stop)
    {
        return 0;
    }

    if (ev_list_size(&loop->backlog_queue) != 0 || ev_list_size(&loop->endgame_queue) != 0)
    {
        return 0;
    }

    return _ev_backend_timeout_timer(loop);
}

static size_t _ev_calculate_read_capacity(const ev_read_t* req)
{
    size_t total = 0;

    size_t i;
    for (i = 0; i < req->data.nbuf; i++)
    {
        total += req->data.bufs[i].size;
    }

    return total;
}

static size_t _ev_calculate_write_size(const ev_write_t* req)
{
    size_t total = 0;

    size_t i;
    for (i = 0; i < req->nbuf; i++)
    {
        total += req->bufs[i].size;
    }
    return total;
}

EV_LOCAL void ev__loop_update_time(ev_loop_t* loop)
{
    loop->hwtime = ev_hrtime() / 1000000;
}

EV_LOCAL int ev__ipc_check_frame_hdr(const void* buffer, size_t size)
{
    const ev_ipc_frame_hdr_t* hdr = buffer;
    if (size < sizeof(ev_ipc_frame_hdr_t))
    {
        return 0;
    }

    if (hdr->hdr_magic != EV_IPC_FRAME_HDR_MAGIC)
    {
        return 0;
    }

    return 1;
}

EV_LOCAL void ev__ipc_init_frame_hdr(ev_ipc_frame_hdr_t* hdr, uint8_t flags, uint16_t exsz, uint32_t dtsz)
{
    hdr->hdr_magic = EV_IPC_FRAME_HDR_MAGIC;
    hdr->hdr_flags = flags;
    hdr->hdr_version = 0;
    hdr->hdr_exsz = exsz;

    hdr->hdr_dtsz = dtsz;
    hdr->reserved = 0;
}

EV_LOCAL ev_loop_t* ev__handle_loop(ev_handle_t* handle)
{
    return handle->loop;
}

int ev_loop_init(ev_loop_t* loop)
{
    int ret;
    if ((ret = _ev_loop_init(loop)) != 0)
    {
        return ret;
    }

    if ((ret = ev__loop_init_backend(loop)) != 0)
    {
        _ev_loop_exit(loop);
        return ret;
    }

    ev__loop_update_time(loop);
    return 0;
}

int ev_loop_exit(ev_loop_t* loop)
{
    if (ev_list_size(&loop->handles.active_list)
        || ev_list_size(&loop->handles.idle_list))
    {
        return EV_EBUSY;
    }

    ev__loop_exit_backend(loop);
    _ev_loop_exit(loop);

    return 0;
}

void ev_loop_stop(ev_loop_t* loop)
{
    loop->mask.b_stop = 1;
}

static uint32_t _ev_loop_calculate_timeout(ev_loop_t* loop, ev_loop_mode_t mode, size_t active_count)
{
    if (mode == EV_LOOP_MODE_ONCE && active_count != 0)
    {
        return 0;
    }

    return _ev_backend_timeout(loop);
}

int ev_loop_run(ev_loop_t* loop, ev_loop_mode_t mode, uint32_t timeout)
{
    int ret;
    size_t active_count = 0;

    if (mode == EV_LOOP_MODE_NOWAIT)
    {
        timeout = 0;
    }

    ev__loop_update_time(loop);
    const uint64_t start_time_ms = loop->hwtime;

    while ((ret = _ev_loop_alive(loop)) != 0 && !loop->mask.b_stop)
    {
        ev__loop_update_time(loop);

        active_count += ev__process_timer(loop);
        active_count += ev__process_backlog(loop);
        active_count += ev__process_endgame(loop);

        if ((ret = _ev_loop_alive(loop)) == 0)
        {
            break;
        }

        /* IO multiplexing */
        uint32_t loop_timeout = _ev_loop_calculate_timeout(loop, mode, active_count);
        loop_timeout = EV_MIN(loop_timeout, timeout);
        ev__poll(loop, loop_timeout);

        /**
         * #EV_LOOP_MODE_ONCE implies forward progress: at least one callback must have
         * been invoked when it returns. #_ev_poll_win() can return without doing
         * I/O (meaning: no callbacks) when its timeout expires - which means we
         * have pending timers that satisfy the forward progress constraint.
         *
         * #EV_LOOP_MODE_NOWAIT makes no guarantees about progress so it's omitted from
         * the check.
         */
        ev__loop_update_time(loop);
        const uint32_t diff_time_ms = (uint32_t)(loop->hwtime - start_time_ms);

        active_count += ev__process_timer(loop);
        active_count += ev__process_backlog(loop);
        active_count += ev__process_endgame(loop);

        if (timeout != EV_INFINITE_TIMEOUT)
        {
            timeout = timeout > diff_time_ms ? timeout - diff_time_ms : 0;
        }
        if (timeout == 0 || (mode == EV_LOOP_MODE_ONCE && active_count != 0))
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

EV_LOCAL int ev__write_init(ev_write_t* req, ev_buf_t* bufs, size_t nbuf)
{
    req->nbuf = nbuf;

    if (nbuf <= ARRAY_SIZE(req->bufsml))
    {
        req->bufs = req->bufsml;
    }
    else
    {
        req->bufs = ev_malloc(sizeof(ev_buf_t) * nbuf);
        if (req->bufs == NULL)
        {
            return EV_ENOMEM;
        }
    }

    memcpy(req->bufs, bufs, sizeof(ev_buf_t) * nbuf);
    req->size = 0;
    req->capacity = _ev_calculate_write_size(req);
    return 0;
}

EV_LOCAL void ev__write_exit(ev_write_t* req)
{
    if (req->bufs != req->bufsml)
    {
        ev_free(req->bufs);
    }

    req->bufs = NULL;
    req->nbuf = 0;
}

EV_LOCAL int ev__read_init(ev_read_t* req, ev_buf_t* bufs, size_t nbuf)
{
    req->data.nbuf = nbuf;

    if (nbuf <= ARRAY_SIZE(req->data.bufsml))
    {
        req->data.bufs = req->data.bufsml;
    }
    else
    {
        req->data.bufs = ev_malloc(sizeof(ev_buf_t) * nbuf);
        if (req->data.bufs == NULL)
        {
            return EV_ENOMEM;
        }
    }

    memcpy(req->data.bufs, bufs, sizeof(ev_buf_t) * nbuf);
    req->data.capacity = _ev_calculate_read_capacity(req);
    req->data.size = 0;
    return 0;
}

EV_LOCAL void ev__read_exit(ev_read_t* req)
{
    if (req->data.bufs != req->data.bufsml)
    {
        ev_free(req->data.bufs);
    }
    req->data.bufs = NULL;
    req->data.nbuf = 0;
}

EV_LOCAL socklen_t ev__get_addr_len(const struct sockaddr* addr)
{
    if (addr->sa_family == AF_INET)
    {
        return sizeof(struct sockaddr_in);
    }
    if (addr->sa_family == AF_INET6)
    {
        return sizeof(struct sockaddr_in6);
    }
#if defined(AF_UNIX) && !defined(_WIN32)
    if (addr->sa_family == AF_UNIX)
    {
        return sizeof(struct sockaddr_un);
    }
#endif

    return (socklen_t)-1;
}

void ev_loop_walk(ev_loop_t* loop, ev_walk_cb cb, void* arg)
{
    ev_list_t* walk_lists[] = {
        &loop->handles.active_list,
        &loop->handles.idle_list,
    };

    size_t i;
    for (i = 0; i < ARRAY_SIZE(walk_lists); i++)
    {
        ev_list_node_t* it = ev_list_begin(walk_lists[i]);
        for (; it != NULL; it = ev_list_next(it))
        {
            ev_handle_t* handle = EV_CONTAINER_OF(it, ev_handle_t, handle_queue);
            if (cb(handle, arg) != 0)
            {
                return;
            }
        }
    }
}
