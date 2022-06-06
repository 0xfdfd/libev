#ifndef __EV_TIME_H__
#define __EV_TIME_H__

#include "ev/time_forward.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup EV_TIME
 * @{
 */

/**
 * @brief High-accuracy time type.
 */
struct ev_timespec_s
{
    uint64_t    tv_sec;     /**< seconds */
    uint32_t    tv_nsec;    /**< nanoseconds */
};
#define EV_TIMESPEC_INVALID \
    {\
        0,\
        0,\
    }

/**
 * @} EV_TIME
 */

#ifdef __cplusplus
}
#endif
#endif
