#ifndef __TEST_UTILS_HASH_H__
#define __TEST_UTILS_HASH_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t test_hash64(const void* data, size_t size, uint64_t seed);

#ifdef __cplusplus
}
#endif
#endif
