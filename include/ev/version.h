#ifndef __EV_VERSION_H__
#define __EV_VERSION_H__

#define EV_VERSION_MAJOR            0
#define EV_VERSION_MINOR            0
#define EV_VERSION_PATCH            3
#define EV_VERSION_DEV              1
#define EV_VERSION(a, b, c, d)      (((a) << 24) + ((b) << 16) + ((c) << 8) + (d))
#define EV_VERSION_CODE             \
    EV_VERSION(EV_VERSION_MAJOR, EV_VERSION_MINOR, EV_VERSION_PATCH, EV_VERSION_DEV)

#endif
