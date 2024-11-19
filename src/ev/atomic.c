#if defined(EV_ATOMIC_WIN32)

int ev_atomic32_compare_exchange_strong(volatile ev_atomic32_t* obj,
    int32_t* expected, int32_t desired)
{
    int32_t previous = InterlockedCompareExchange(obj, desired, *expected);
    if (previous == *expected)
    {
        return 1;
    }

    *expected = previous;
    return 0;
}

int ev_atomic64_compare_exchange_strong(volatile ev_atomic64_t* obj,
    int64_t* expected, int64_t desired)
{
    int64_t previous = InterlockedCompareExchange64(obj, desired, *expected);
    if (previous == *expected)
    {
        return 1;
    }

    *expected = previous;
    return 0;
}

#elif defined(EV_ATOMIC_LOCK_SIMULATION)

static ev_mutex_t s_atomic_lock;

static void _ev_atomic_lock_init2(void)
{
    ev_mutex_init(&s_atomic_lock, 0);
}

static void _ev_atomic_lock_init_once(void)
{
    static ev_once_t token = EV_ONCE_INIT;
    ev_once_execute(&token, _ev_atomic_lock_init2);
}

EV_LOCAL void ev__atomic_exit(void)
{
    ev_mutex_exit(&s_atomic_lock);
}

void ev_atomic32_init(volatile ev_atomic32_t* obj, int32_t desired)
{
    _ev_atomic_lock_init_once();
    ev_atomic32_store(obj, desired);
}

void ev_atomic64_init(volatile ev_atomic64_t* obj, int64_t desired)
{
    _ev_atomic_lock_init_once();
    ev_atomic64_store(obj, desired);
}

void ev_atomic32_store(volatile ev_atomic32_t* obj, int32_t desired)
{
    ev_mutex_enter(&s_atomic_lock);
    {
        *obj = desired;
    }
    ev_mutex_leave(&s_atomic_lock);
}

void ev_atomic64_store(volatile ev_atomic64_t* obj, int64_t desired)
{
    ev_mutex_enter(&s_atomic_lock);
    {
        *obj = desired;
    }
    ev_mutex_leave(&s_atomic_lock);
}

int32_t ev_atomic32_load(volatile ev_atomic32_t* obj)
{
    int32_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

int64_t ev_atomic64_load(volatile ev_atomic64_t* obj)
{
    int64_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

int32_t ev_atomic32_exchange(volatile ev_atomic32_t* obj, int32_t desired)
{
    int32_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
        *obj = desired;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

int64_t ev_atomic64_exchange(volatile ev_atomic64_t* obj, int64_t desired)
{
    int64_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
        *obj = desired;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

int ev_atomic32_compare_exchange_strong(volatile ev_atomic32_t* obj,
    int32_t* expected, int32_t desired)
{
    int ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        int32_t previous = *obj;
        if (previous == *expected)
        {
            *obj = desired;
            ret = 1;
        }
        else
        {
            *expected = previous;
            ret = 0;
        }
    }
    ev_mutex_leave(&s_atomic_lock);
    return ret;
}

int ev_atomic64_compare_exchange_strong(volatile ev_atomic64_t* obj,
    int64_t* expected, int64_t desired)
{
    int ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        int64_t previous = *obj;
        if (previous == *expected)
        {
            *obj = desired;
            ret = 1;
        }
        else
        {
            *expected = previous;
            ret = 0;
        }
    }
    ev_mutex_leave(&s_atomic_lock);
    return ret;
}

int32_t ev_atomic32_fetch_add(volatile ev_atomic32_t* obj, int32_t arg)
{
    int32_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
        *obj += arg;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

int64_t ev_atomic64_fetch_add(volatile ev_atomic64_t* obj, int64_t arg)
{
    int64_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
        *obj += arg;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

int32_t ev_atomic32_fetch_sub(volatile ev_atomic32_t* obj, int32_t arg)
{
    int32_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
        *obj -= arg;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

int64_t ev_atomic64_fetch_sub(volatile ev_atomic64_t* obj, int64_t arg)
{
    int64_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
        *obj -= arg;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

int32_t ev_atomic32_fetch_or(volatile ev_atomic32_t* obj, int32_t arg)
{
    int32_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
        *obj |= arg;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

int64_t ev_atomic64_fetch_or(volatile ev_atomic64_t* obj, int64_t arg)
{
    int64_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
        *obj |= arg;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

int32_t ev_atomic32_fetch_xor(volatile ev_atomic32_t* obj, int32_t arg)
{
    int32_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
        *obj ^= arg;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

int64_t ev_atomic32_fetch_xor(volatile ev_atomic64_t* obj, int64_t arg)
{
    int64_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
        *obj ^= arg;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

int32_t ev_atomic32_fetch_and(volatile ev_atomic32_t* obj, int32_t arg)
{
    int32_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
        *obj &= arg;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

int64_t ev_atomic64_fetch_and(volatile ev_atomic64_t* obj, int64_t arg)
{
    int64_t ret;
    ev_mutex_enter(&s_atomic_lock);
    {
        ret = *obj;
        *obj &= arg;
    }
    ev_mutex_leave(&s_atomic_lock);

    return ret;
}

#else

EV_LOCAL void ev__atomic_exit(void)
{
}

#endif
