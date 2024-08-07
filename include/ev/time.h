#ifndef __EV_TIME_H__
#define __EV_TIME_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_TIME Time
 * @{
 */

/**
 * @brief Infinite timeout.
 */
#define EV_INFINITE_TIMEOUT         ((uint32_t)-1)

/**
 * @brief Time spec
 */
struct ev_timespec_s;

/**
 * @brief Typedef of #ev_timespec_s.
 */
typedef struct ev_timespec_s ev_timespec_t;

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
 * @brief Returns the current high-resolution real time in nanoseconds.
 * @return Time in nanoseconds.
 */
EV_API uint64_t ev_hrtime(void);

/**
 * @} EV_TIME
 */

#ifdef __cplusplus
}
#endif
#endif
