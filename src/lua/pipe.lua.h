#ifndef __EV_LUA_PIPE_H__
#define __EV_LUA_PIPE_H__

#include "ev.lua.internal.h"

#ifdef __cplusplus
extern "C" {
#endif

int lev_pipe(lua_State* L);

ev_pipe_t* lev_try_to_pipe(lua_State* L, int idx);

#ifdef __cplusplus
}
#endif

#endif
