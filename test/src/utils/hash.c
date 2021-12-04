#include "hash.h"

uint64_t test_hash64(const void* data, size_t size, uint64_t seed)
{
    const unsigned char* p_dat = (const unsigned char*)data;

    size_t i;
    for (i = 0; i < size; i++)
    {
        seed = seed * 131313 + p_dat[i];
    }

    return seed;
}
