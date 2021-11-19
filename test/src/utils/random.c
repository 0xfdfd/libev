#include "random.h"

#include <assert.h>
#include <errno.h>

#if defined(_WIN32)
#   include <windows.h>
#   include <ntsecapi.h>
#else
#   if defined(__GLIBC__) && __GLIBC__ >= 2 && defined(__GLIBC_MINOR__) && __GLIBC_MINOR__ >= 25
#       include <sys/random.h>
#       define TEST_EV_HAVE_GETRANDOM   1
#   else
#       include <sys/types.h>
#       include <sys/stat.h>
#       include <fcntl.h>
#   endif
#endif

void test_random(void* buffer, size_t size)
{
#if defined(_WIN32)
    BOOLEAN ret = RtlGenRandom(buffer, (ULONG)size);
    assert(ret);
#elif defined(TEST_EV_HAVE_GETRANDOM)
    size_t w_size = 0;
    ssize_t r;

    while (w_size < size)
    {
        r = getrandom(buffer + w_size, size - w_size, 0);
        if (r < 0 && (errno == EAGAIN || errno == EINTR))
        {
            continue;
        }
        assert(r > 0);
        w_size += r;
    }
#else
    int s = open("/dev/urandom", O_RDONLY);
    assert(s >= 0);
    ssize_t r = read(s, buffer, size);
    assert(r == size);
    close(s);
#endif
}
