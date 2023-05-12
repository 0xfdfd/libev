#include "random.h"
#include "test.h"
#include <stdint.h>
#include <string.h>

static uint64_t _random(uint64_t seed)
{
    return 6364136223846793005ULL * seed + 1;
}

void test_random(void* buffer, size_t size)
{
    uint64_t seed = _random(0);

    /* Calculate */
    uint8_t* head_addr = buffer;
    uint64_t* body_addr = (uint64_t*)ALIGN_ADDR(head_addr, sizeof(uint64_t));
    const size_t align_size = ALIGN_ADDR(size, sizeof(uint64_t));
    uint8_t* tail_addr = size == align_size ? head_addr + align_size : head_addr + align_size - sizeof(uint64_t);

    /* If buffer not align to uint64_t, handle it */
    ptrdiff_t addr_diff = (uintptr_t)body_addr - (uintptr_t)head_addr;
    if (addr_diff != 0)
    {
        seed = _random(seed);
        memcpy(buffer, &seed, addr_diff);
    }

    for (; (uintptr_t)body_addr < (uintptr_t)tail_addr; body_addr++)
    {
        seed = _random(seed);
        *body_addr = seed;
    }

    if (size != align_size)
    {
        seed = _random(seed);
        memcpy(tail_addr, &seed, head_addr + size - tail_addr);
    }
}
