#include "random.h"
#include <stdint.h>
#include <string.h>

static uint64_t _random(uint64_t seed)
{
    return 6364136223846793005ULL * seed + 1;
}

void test_random(void* buffer, size_t size)
{
    uint64_t seed = 0;
    if (size < sizeof(uint64_t))
    {
        memcpy(buffer, &seed, size);
        return;
    }

    size_t pos;
    for (pos = 0; pos < size - sizeof(uint64_t); pos += sizeof(uint64_t))
    {
        seed = _random(seed);
        memcpy((uint8_t*)buffer + pos, &seed, sizeof(uint64_t));
    }

    seed = _random(seed);
    memcpy((uint8_t*)buffer + pos, &seed, size - pos);
}
