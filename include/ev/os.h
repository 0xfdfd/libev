/**
 * @file
 */
#ifndef __EV_OS_H__
#define __EV_OS_H__

#if defined(_WIN32)
#   include "ev/os_win.h"
#else
#   include "ev/os_unix.h"
#endif

/**
 * @brief Infinite timeout.
 */
#define EV_INFINITE_TIMEOUT    ((uint32_t)-1)

#endif
