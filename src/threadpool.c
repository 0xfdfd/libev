#include "ev-common.h"
#include "handle.h"

static void _ev_threadpool_on_loop(ev_todo_token_t* todo)
{
    ev_threadpool_work_t* work = EV_CONTAINER_OF(todo, ev_threadpool_work_t, token);
    ev__handle_exit(&work->base, 0);
    work->data.done_cb(work, work->data.status);
}

static void _ev_threadpool_submit_to_loop(ev_threadpool_work_t* work)
{
    ev__loop_submit_task_mt(work->data.loop, &work->token, _ev_threadpool_on_loop);
}

static ev_threadpool_work_t* _ev_threadpool_get_work_locked(ev_threadpool_t* pool)
{
    ev_queue_node_t* it;
    size_t i;

    ev_mutex_enter(&pool->mutex);
    for (i = 0; i < ARRAY_SIZE(pool->work_queue); i++)
    {
        it = ev_queue_pop_front(&pool->work_queue[i]);
        if (it != NULL)
        {
            ev_queue_init(it);
            break;
        }
    }
    ev_mutex_leave(&pool->mutex);

    if (it == NULL)
    {
        return NULL;
    }

    return EV_CONTAINER_OF(it, ev_threadpool_work_t, node);
}

static void _ev_threadpool_commit(ev_threadpool_work_t* work, int status)
{
    work->data.status = status;
    _ev_threadpool_submit_to_loop(work);
}

static void _ev_threadpool_do_work(ev_threadpool_t* pool)
{
    ev_threadpool_work_t* work = _ev_threadpool_get_work_locked(pool);
    if (work == NULL)
    {
        return;
    }

    work->data.status = EV_EBUSY;
    work->data.work_cb(work);

    _ev_threadpool_commit(work, EV_SUCCESS);
}

static void _ev_threadpool_cleanup(ev_threadpool_t* pool)
{
    ev_threadpool_work_t* work;
    while ((work = _ev_threadpool_get_work_locked(pool)) != NULL)
    {
        _ev_threadpool_commit(work, EV_ECANCELED);
    }
}

static void _ev_threadpool_worker(void* arg)
{
    ev_threadpool_t* pool = arg;

    while (pool->looping)
    {
        ev_sem_wait(&pool->p2w_sem);
        _ev_threadpool_do_work(pool);
    }
    _ev_threadpool_cleanup(pool);
}

static void _cancel_work_queue_for_loop(ev_threadpool_t* pool,
        ev_queue_node_t* wqueue, ev_loop_t* loop)
{
    ev_queue_node_t* it;

    ev_mutex_enter(&pool->mutex);
    {
        it = ev_queue_head(wqueue);
        while (it != NULL)
        {
            ev_threadpool_work_t* work = EV_CONTAINER_OF(it, ev_threadpool_work_t, node);
            it = ev_queue_next(wqueue, it);

            if (work->data.loop != loop)
            {
                continue;
            }

            _ev_threadpool_commit(work, EV_ECANCELED);
        }
    }
    ev_mutex_leave(&pool->mutex);
}

int ev_threadpool_init(ev_threadpool_t* pool, const ev_thread_opt_t* opt,
    ev_os_thread_t* storage, size_t num)
{
    int ret;
    size_t i, idx;

    for (i = 0; i < ARRAY_SIZE(pool->work_queue); i++)
    {
        ev_queue_init(&pool->work_queue[i]);
    }

    pool->threads = storage;
    pool->thrnum = num;
    pool->looping = 1;

    if ((ret = ev_mutex_init(&pool->mutex, 0)) != EV_SUCCESS)
    {
        return ret;
    }
    if ((ret = ev_sem_init(&pool->p2w_sem, 0)) != EV_SUCCESS)
    {
        goto err_release_mutex;
    }

    for (idx = 0; idx < num; idx++)
    {
        ret = ev_thread_init(&storage[idx], opt, _ev_threadpool_worker, pool);
        if (ret < 0)
        {
            goto err_release_thread;
        }
    }

    return EV_SUCCESS;

err_release_thread:
    pool->looping = 0;
    for (i = 0; i < idx; i++)
    {
        ev_thread_exit(&storage[i], EV_INFINITE_TIMEOUT);
    }
    ev_sem_exit(&pool->p2w_sem);
err_release_mutex:
    ev_mutex_exit(&pool->mutex);
    return ret;
}

int ev_threadpool_submit(ev_threadpool_t* pool, ev_loop_t* loop,
    ev_threadpool_work_t* work, ev_threadpool_work_type_t type,
    ev_threadpool_work_cb work_cb, ev_threadpool_work_done_cb done_cb)
{
    if (!pool->looping)
    {
        return EV_EACCES;
    }
    assert(type < ARRAY_SIZE(pool->work_queue));

    ev__handle_init(loop, &work->base, EV_ROLE_EV_WORK, NULL);
    ev__handle_active(&work->base);
    work->data.pool = pool;
    work->data.loop = loop;
    work->data.status = EV_ELOOP;
    work->data.work_cb = work_cb;
    work->data.done_cb = done_cb;

    ev_mutex_enter(&pool->mutex);
    {
        ev_queue_push_back(&pool->work_queue[type], &work->node);
    }
    ev_mutex_leave(&pool->mutex);

    ev_sem_post(&pool->p2w_sem);

    return EV_SUCCESS;
}

int ev_threadpool_cancel(ev_threadpool_work_t* work)
{
    ev_threadpool_t* pool = work->data.pool;

    int cancelled;
    ev_mutex_enter(&pool->mutex);
    {
        cancelled = !ev_queue_empty(&work->node);
        if (cancelled)
        {
            ev_queue_erase(&work->node);
        }
    }
    ev_mutex_leave(&pool->mutex);

    if (!cancelled)
    {
        return EV_EBUSY;
    }

    _ev_threadpool_commit(work, EV_ECANCELED);

    return EV_SUCCESS;
}

static void _threadpool_cancel_work_queue(ev_threadpool_t* pool, ev_queue_node_t* wqueue)
{
    (void)pool;
    ev_queue_node_t* node;
    while ((node = ev_queue_pop_front(wqueue)) != NULL)
    {
        ev_threadpool_work_t* work = EV_CONTAINER_OF(node, ev_threadpool_work_t, node);
        _ev_threadpool_commit(work, EV_ECANCELED);
    }
}

static void _ev_threadpool_cancel_all(ev_threadpool_t* pool)
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(pool->work_queue); i++)
    {
        _threadpool_cancel_work_queue(pool, &pool->work_queue[i]);
    }
}

void ev_threadpool_exit(ev_threadpool_t* pool)
{
    size_t i;

    /* stop loop */
    pool->looping = 0;
    for (i = 0; i < pool->thrnum; i++)
    {
        ev_sem_post(&pool->p2w_sem);
    }

    /* exit thread */
    for (i = 0; i < pool->thrnum; i++)
    {
        if (ev_thread_exit(&pool->threads[i], EV_INFINITE_TIMEOUT) != EV_SUCCESS)
        {
            abort();
        }
    }

    /* now we can do some cleanup */
    _ev_threadpool_cancel_all(pool);

    ev_mutex_exit(&pool->mutex);
    ev_sem_exit(&pool->p2w_sem);
}

int ev__loop_submit_threadpool(ev_loop_t* loop, ev_threadpool_work_t* work,
    ev_threadpool_work_type_t type, ev_threadpool_work_cb work_cb,
    ev_threadpool_work_done_cb done_cb)
{
    ev_threadpool_t* pool = loop->threadpool.pool;
    if (pool == NULL)
    {
        return EV_ENOTHREADPOOL;
    }

    return ev_threadpool_submit(pool, loop, work, type, work_cb, done_cb);
}

int ev_loop_unlink_threadpool(ev_loop_t* loop)
{
    size_t i;
    ev_threadpool_t* pool = loop->threadpool.pool;
    if (pool == NULL)
    {
        return EV_ENOTHREADPOOL;
    }

    /* From now there should no futher work from loop incoming. */
    loop->threadpool.pool = NULL;

    /**
     * Cancel all pending task from target loop.
     * However, due to #ev_threadpool_submit() is a public API, there might be
     * some tasks which not come from loop directly. Thus they can be unexpected
     * canceled as well.
     */
    for (i = 0; i < ARRAY_SIZE(pool->work_queue); i++)
    {
        _cancel_work_queue_for_loop(pool, &pool->work_queue[i], loop);
    }

    return EV_SUCCESS;
}
