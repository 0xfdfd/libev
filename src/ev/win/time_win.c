/**
 * Frequency of the high-resolution clock.
 */
static uint64_t s_hrtime_frequency = 0;

static uint64_t _ev_hrtime_win(unsigned int scale)
{
    LARGE_INTEGER counter;
    double scaled_freq;
    double result;
    DWORD errcode;

    assert(s_hrtime_frequency != 0);
    assert(scale != 0);

    if (!QueryPerformanceCounter(&counter))
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
    assert(counter.QuadPart != 0);

    /*
     * Because we have no guarantee about the order of magnitude of the
     * performance counter interval, integer math could cause this computation
     * to overflow. Therefore we resort to floating point math.
     */
    scaled_freq = (double)s_hrtime_frequency / scale;
    result = (double)counter.QuadPart / scaled_freq;
    return (uint64_t)result;
}

void ev__time_init_win(void)
{
    DWORD errcode;
    LARGE_INTEGER perf_frequency;

    /*
     * Retrieve high-resolution timer frequency and pre-compute its reciprocal.
     */
    if (QueryPerformanceFrequency(&perf_frequency))
    {
        s_hrtime_frequency = perf_frequency.QuadPart;
    }
    else
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", errcode);
    }
}

uint64_t ev_hrtime(void)
{
    ev__init_once_win();
#define EV__NANOSEC 1000000000
    return _ev_hrtime_win(EV__NANOSEC);
#undef EV__NANOSEC
}