#ifndef __EV_LUA_INTERNAL_H__
#define __EV_LUA_INTERNAL_H__

#include "ev.lua.h"
#include "ev.h"

#define lev_error(L, loop, errcode, fmt, ...)   \
    lev_error_ex(__FILE__, __LINE__, L, loop, errcode, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lauxlib.h"

/**
 * @brief Make a loop on top of lua stack and return the handle.
 * @param[in] L     Lua Stack.
 * @return          Event loop.
 */
ev_loop_t* lev_make_loop(lua_State* L);

/**
 * @brief Set coroutine state.
 * @param[in] L     Lua stack.
 * @param[in] loop  Event loop.
 * @param[in] busy  Is busy.
 * @return          This function does not failure.
 */
int lev_set_state(lua_State* L, ev_loop_t* loop, int busy);

/**
 * @brief Check if \p L is yieldable in event loop.
 * @param[in] L     Lua coroutine to check.
 * @param[in] loop  Event loop.
 * @return          Boolean.
 */
int lev_is_yieldable(lua_State* L, ev_loop_t* loop);

/**
 * @brief Convert value at \p idx into event loop.
 * @param[in] L     Lua Stack.
 * @param[in] idx   Stack index.
 * @return          Event loop.
 */
ev_loop_t* lev_to_loop(lua_State* L, int idx);

/**
 * @brief Generate a error.
 * @param[in] L         Lua Stack.
 * @param[in] loop      Event loop.
 * @param[in] errcode   Error code.
 * @param[in] fmt       User format.
 * @param[in] ...       Format list.
 * @return This function does not return.
 */
int lev_error_ex(const char* file, int line, lua_State* L, ev_loop_t* loop, int errcode, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
