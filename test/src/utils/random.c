#include "random.h"
#include "test.h"

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
    ASSERT_NE_D32(ret, 0);
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
        ASSERT_GT_D32(r, 0);
        w_size += r;
    }
#else
    int s = open("/dev/urandom", O_RDONLY);
    ASSERT_GE_D32(s, 0);
    ssize_t r = read(s, buffer, size);
    ASSERT_EQ_D32(r, size);
    close(s);
#endif
}
