#ifndef __EV_BACKEND_H__
#define __EV_BACKEND_H__

#if defined(_WIN32)
#   include "ev/win.h"
#else
#   include "ev/unix.h"
#endif

#endif
