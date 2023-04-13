#ifndef __EV_LUA_PROMISE_H__
#define __EV_LUA_PROMISE_H__

#include "ev.lua.internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lev_promise_s lev_promise_t;

/**
 * @brief Get value callback.
 * @param[in] L         Lua stack.
 * @param[in] promise   Promise handle.
 * @param[in] narg      The number of values on stack.
 * @param[in] data      User defined argument.
 * @return              The number of values on stack.
 */
typedef int (*lev_promise_cb)(lua_State* L, lev_promise_t* promise, int narg, void* data);

/**
 * @brief Create a new promise and push on top of Lua stack.
 * @param[in] L     Lua stack.
 * @return          Always 1.
 */
int lev_promise(lua_State* L);

/**
 * @brief Create a new promise and push on top of Lua stack, and return it's handle.
 * @param[in] L     Lua stack.
 * @param[in] loop  Event loop.
 * @return          Promise handle.
 */
lev_promise_t* lev_promise_make(lua_State* L, ev_loop_t* loop);

/**
 * @brief Set value for promise.
 * @param[in] L         Lua stack.
 * @param[in] promise   The promise handle.
 * @param[in] narg      The number of arguments. Arguments should on top of Lua
 *   stack in order.
 * @return              #ev_errno_t
 */
int lev_promise_set_value(lua_State* L, lev_promise_t* promise, int narg);

/**
 * @brief Get value for promise.
 * @warning This function should be called only in a tail call.
 * @param[in] L         Lua stack.
 * @param[in] promise   Promise handle.
 * @param[in] cb        Value callback.
 * @param[in] data      Private data.
 * @return              The same value from \p cb.
 */
int lev_promise_get_value(lua_State* L, lev_promise_t* promise, lev_promise_cb cb, void* data);

#ifdef __cplusplus
}
#endif
#endif
