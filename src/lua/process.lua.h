#ifndef __EV_LUA_PROCESS_H__
#define __EV_LUA_PROCESS_H__

#include "ev.lua.internal.h"

#ifdef __cplusplus
extern "C" {
#endif

int lev_process(lua_State* L);

int lev_getcwd(lua_State* L);

int lev_exepath(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif
