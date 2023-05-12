#ifndef __EV_LUA_UDP_H__
#define __EV_LUA_UDP_H__

#include "ev.lua.internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LEV_UDP_FLAG_MAP(xx)    \
    xx(AF_INET)                 \
    xx(AF_INET6)                \
    xx(AF_UNSPEC)               \
    \
    xx(EV_UDP_IPV6_ONLY)        \
    xx(EV_UDP_REUSEADDR)        \
    \
    xx(EV_UDP_LEAVE_GROUP)      \
    xx(EV_UDP_ENTER_GROUP)

int lev_udp(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif
