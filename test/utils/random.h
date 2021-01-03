#ifndef __UTILS_RANDOM_H__
#define __UTILS_RANDOM_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#if defined(_WIN32)
#	include <ntsecapi.h>
#else
#	include <sys/random.h>
#endif

void test_random(void* buffer, size_t size)
{
#if defined(_WIN32)
	BOOLEAN ret = RtlGenRandom(buffer, size);
	assert(ret);
#else
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
#endif
}

#ifdef __cplusplus
}
#endif
#endif
