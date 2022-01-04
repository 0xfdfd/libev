#include "ev-common.h"

static void _ev_threadpool_on_loop(ev_todo_t* todo)
{
    ev_threadpool_work_t* work = container_of(todo, ev_threadpool_work_t, token);
    ev__handle_exit(&work->base, 0);
    work->data.done_cb(work, work->data.status);
}

static void _ev_threadpool_submit_to_loop(ev_threadpool_work_t* work)
{
    ev__loop_submit_task_mt(work->data.loop, &work->token, _ev_threadpool_on_loop);
}

static ev_threadpool_work_t* _ev_threadpool_get_work_locked(ev_threadpool_t* pool)
{
    ev_cycle_list_node_t* it;
    ev_mutex_enter(&pool->mutex);
    {
        it = ev_cycle_list_pop_front(&pool->cpu_queue);
        if (it == NULL)
        {
            it = ev_cycle_list_pop_front(&pool->io_fast_queue);
        }
        if (it == NULL)
        {
            it = ev_cycle_list_pop_front(&pool->io_slow_queue);
        }

        /* For #ev_threadpool_cancel() */
        if (it != NULL)
        {
            ev_cycle_list_init(it);
        }
    }
    ev_mutex_leave(&pool->mutex);

    if (it == NULL)
    {
        return NULL;
    }

    return container_of(it, ev_threadpool_work_t, node);
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
    work->data.status = EV_SUCCESS;

    _ev_threadpool_submit_to_loop(work);
}

static void _ev_threadpool_cleanup(ev_threadpool_t* pool)
{
    ev_threadpool_work_t* work;
    while ((work = _ev_threadpool_get_work_locked(pool)) != NULL)
    {
        work->data.status = EV_ECANCELED;
        _ev_threadpool_submit_to_loop(work);
    }
}

static void _ev_threadpool_worker(void* arg)
{
    ev_threadpool_t* pool = arg;
    ev_sem_wait(&pool->ent_sem);
    ev_sem_post(&pool->w2p_sem);

    while (pool->looping)
    {
        ev_sem_wait(&pool->p2w_sem);
        _ev_threadpool_do_work(pool);
    }
    _ev_threadpool_cleanup(pool);
}

int ev_threadpool_init(ev_threadpool_t* pool, const ev_thread_opt_t* opt,
    ev_os_thread_t* storage, size_t num)
{
    int ret;
    size_t i, idx;

    ev_cycle_list_init(&pool->cpu_queue);
    ev_cycle_list_init(&pool->io_fast_queue);
    ev_cycle_list_init(&pool->io_slow_queue);
    pool->threads = storage;
    pool->thrnum = num;
    pool->looping = 1;

    if ((ret = ev_mutex_init(&pool->mutex, 0)) != EV_SUCCESS)
    {
        return ret;
    }
    if ((ret = ev_sem_init(&pool->ent_sem, 0)) != EV_SUCCESS)
    {
        goto err_release_mutex;
    }
    if ((ret = ev_sem_init(&pool->p2w_sem, 0)) != EV_SUCCESS)
    {
        ev_sem_exit(&pool->ent_sem);
        goto err_release_mutex;
    }
    if ((ret = ev_sem_init(&pool->w2p_sem, 0)) != EV_SUCCESS)
    {
        ev_sem_exit(&pool->ent_sem);
        ev_sem_exit(&pool->p2w_sem);
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

    for (i = 0; i < idx; i++)
    {
        ev_sem_post(&pool->ent_sem);
    }
    for (i = 0; i < idx; i++)
    {
        ev_sem_wait(&pool->w2p_sem);
    }

    return EV_SUCCESS;

err_release_thread:
    pool->looping = 0;
    for (i = 0; i < idx; i++)
    {
        ev_sem_post(&pool->ent_sem);
    }
    for (i = 0; i < idx; i++)
    {
        ev_thread_exit(&storage[i], EV_THREAD_WAIT_INFINITE);
    }
    ev_sem_exit(&pool->ent_sem);
    ev_sem_exit(&pool->p2w_sem);
    ev_sem_exit(&pool->w2p_sem);
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

    ev__handle_init(loop, &work->base, EV_ROLE_EV_WORK, NULL);
    work->data.pool = pool;
    work->data.loop = loop;
    work->data.status = EV_ELOOP;
    work->data.work_cb = work_cb;
    work->data.done_cb = done_cb;

    ev_mutex_enter(&pool->mutex);
    switch (type)
    {
    case EV_THREADPOOL_WORK_CPU:
        ev_cycle_list_push_back(&pool->cpu_queue, &work->node);
        break;

    case EV_THREADPOOL_WORK_IO_FAST:
        ev_cycle_list_push_back(&pool->io_fast_queue, &work->node);
        break;

    default:
        ev_cycle_list_push_back(&pool->io_slow_queue, &work->node);
        break;
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
        cancelled = !ev_cycle_list_empty(&work->node);
        if (cancelled)
        {
            ev_cycle_list_erase(&work->node);
        }
    }
    ev_mutex_leave(&pool->mutex);

    if (!cancelled)
    {
        return EV_EBUSY;
    }

    work->data.status = EV_ECANCELED;
    _ev_threadpool_submit_to_loop(work);

    return EV_SUCCESS;
}

void ev_threadpool_exit(ev_threadpool_t* pool)
{
    pool->looping = 0;

    size_t i;
    for (i = 0; i < pool->thrnum; i++)
    {
        ev_sem_post(&pool->p2w_sem);
    }
    for (i = 0; i < pool->thrnum; i++)
    {
        if (ev_thread_exit(&pool->threads[i], EV_THREAD_WAIT_INFINITE) != EV_SUCCESS)
        {
            abort();
        }
    }

    ev_mutex_exit(&pool->mutex);
    ev_sem_exit(&pool->ent_sem);
    ev_sem_exit(&pool->p2w_sem);
    ev_sem_exit(&pool->w2p_sem);
}
