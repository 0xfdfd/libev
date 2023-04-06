#ifndef __EV_LUA_MISC_H__
#define __EV_LUA_MISC_H__

#include "ev.lua.internal.h"

#ifdef __cplusplus
extern "C" {
#endif

int lev_hrtime(lua_State* L);
int lev_strerror(lua_State* L);

/**
 * @brief Convert value at \p idx into sockaddr_storage object.
 * @param[in] L     Lua stack.
 * @param[in] idx   Stack index.
 * @return          Address of sockaddr_storage object.
 */
struct sockaddr_storage* lev_to_sockaddr_storage(lua_State* L, int idx);

/**
 * @brief Create a sockaddr_storage object on top of \p L.
 * @param[in] L     Lua stack.
 * @return          Address of sockaddr_storage object.
 */
struct sockaddr_storage* lev_push_sockaddr_storage(lua_State* L);

/**
 * @brief Convert ip and port into network address.
 * @param[in] L     Lua stack.
 * @return          Always 1.
 */
int lev_ip_addr(lua_State* L);

/**
 * @brief Convert network address into ip and port.
 * @param[in] L     Lua stack.
 * @return          Always 2.
 */
int lev_ip_name(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif
