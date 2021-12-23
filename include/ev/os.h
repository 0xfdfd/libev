#ifndef __EV_OS_H__
#define __EV_OS_H__

#if defined(_WIN32)
#   include "ev/os_win.h"
#else
#   include "ev/os_unix.h"
#endif

#endif
