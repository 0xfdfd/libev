#include "misc.lua.h"

#define LEV_SOCKADDR_STORAGE_NAME   "__ev_sockaddr_storage"

int lev_hrtime(lua_State* L)
{
    static uint64_t basis = 0;

    uint64_t t = ev_hrtime();
    if (basis == 0)
    {
        basis = t;
    }

    uint64_t dif = t - basis;

    lua_pushinteger(L, (lua_Integer)dif);
    return 1;
}

int lev_strerror(lua_State* L)
{
    int errcode = (int)luaL_checkinteger(L, 1);
    lua_pushstring(L, ev_strerror(errcode));
    return 1;
}

struct sockaddr_storage* lev_push_sockaddr_storage(lua_State* L)
{
    size_t malloc_size = sizeof(struct sockaddr_storage);
    struct sockaddr_storage* addr = lua_newuserdata(L, malloc_size);

    luaL_newmetatable(L, LEV_SOCKADDR_STORAGE_NAME);
    lua_setmetatable(L, -2);

    return addr;
}

struct sockaddr_storage* lev_to_sockaddr_storage(lua_State* L, int idx)
{
    return luaL_checkudata(L, idx, LEV_SOCKADDR_STORAGE_NAME);
}

int lev_ip_addr(lua_State* L)
{
    int ret;
    const char* ip = luaL_checkstring(L, 1);
    int port = (int)luaL_checkinteger(L, 2);

    size_t addr_size = sizeof(struct sockaddr_storage);
    struct sockaddr_storage* addr = lev_push_sockaddr_storage(L);
    if ((ret = ev_ip_addr(ip, port, (struct sockaddr*)addr, addr_size)) != 0)
    {
        return lev_error(L, NULL, ret, NULL);
    }

    return 1;
}

int lev_ip_name(lua_State* L)
{
    int ret;
    struct sockaddr_storage* addr = lev_to_sockaddr_storage(L, 1);

    char ip[64]; int port = 0;
    if ((ret = ev_ip_name((struct sockaddr*)addr, &port, ip, sizeof(ip))) < 0)
    {
        return lev_error(L, NULL, ret, NULL);
    }

    lua_pushstring(L, ip);
    lua_pushinteger(L, port);
    return 2;
}
