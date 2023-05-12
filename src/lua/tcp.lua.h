#ifndef __EV_LUA_TCP_H__
#define __EV_LUA_TCP_H__

#include "ev.lua.internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new tcp socket.
 * @param[in] L     Lua stack.
 * @return          Always 1.
 */
int lev_tcp(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif
