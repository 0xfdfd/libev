#include <time.h>

uint64_t ev_hrtime(void)
{
    int errcode;
    struct timespec t;

    if (clock_gettime(g_ev_loop_unix_ctx.hwtime_clock_id, &t) != 0)
    {
        errcode = errno;
        EV_ABORT("errno:%d", errcode);
    }

    return t.tv_sec * (uint64_t) 1e9 + t.tv_nsec;
}
