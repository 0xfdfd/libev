#ifndef __EV_ALLOCATOR_INTERNAL_H__
#define __EV_ALLOCATOR_INTERNAL_H__

#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Same as [strdup(3)](https://man7.org/linux/man-pages/man3/strdup.3.html)
 */
EV_LOCAL char* ev__strdup(const char* str);

#ifdef __cplusplus
}
#endif

#endif
