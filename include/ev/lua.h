#ifndef __EV_LUA_H__
#define __EV_LUA_H__

#ifdef __cplusplus
extern "C" {
#endif

struct lua_State;

/**
 * @brief Load libev package.
 * @param[in] L     Lua Stack.
 * @return          Always 1.
 */
int luaopen_ev(struct lua_State* L);

#ifdef __cplusplus
}
#endif
#endif
