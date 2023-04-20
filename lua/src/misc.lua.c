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

int lev_arg_pack(lua_State* L)
{
    int i;
    int sp = lua_gettop(L);

    lua_newtable(L); // sp + 1
    for (i = 1; i <= sp; i++)
    {
        lua_pushvalue(L, i); // sp + 2
        lua_rawseti(L, sp + 1, i);
    }

    return 1;
}

int lev_arg_unpack(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    int sp = lua_gettop(L);
    lua_State* tmp = lua_newthread(L); // sp + 1

    lua_pushnil(L); // sp + 2
    while (lua_next(L, 1) != 0)
    {// key at sp+2, value at sp+3
        lua_xmove(L, tmp, 1); // sp + 2
    }

    int tmp_sp = lua_gettop(tmp);
    lua_xmove(tmp, L, tmp_sp);
    lua_remove(L, sp + 1);

    return tmp_sp;
}
