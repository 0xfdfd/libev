#ifndef __EV_BACKEND_H__
#define __EV_BACKEND_H__

#if defined(_WIN32)
#   include "ev/backend_win.h"
#else
#   include "ev/backend_unix.h"
#endif

#endif
