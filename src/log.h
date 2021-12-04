#ifndef __EV_LOG_INTERNAL_H__
#define __EV_LOG_INTERNAL_H__

#include "ev-common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EV_LOG_TRACE(fmt, ...)  \
    ev__log(EV_LOG_TRACE, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define EV_LOG_INFO(fmt, ...)   \
    ev__log(EV_LOG_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define EV_LOG_ERROR(fmt, ...)  \
    ev__log(EV_LOG_ERROR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define EV_LOG_FATAL(fmt, ...)  \
    ev__log(EV_LOG_FATAL, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum ev_log_level
{
    EV_LOG_TRACE,
    EV_LOG_INFO,
    EV_LOG_ERROR,
    EV_LOG_FATAL,
}ev_log_level_t;

API_LOCAL void ev__log(ev_log_level_t level, const char* file, const char* func,
    int line, const char* fmt, ...);

API_LOCAL void ev__dump_hex(const void* data, size_t size, size_t width);

#ifdef __cplusplus
}
#endif
#endif
