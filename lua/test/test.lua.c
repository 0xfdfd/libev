#define EV_TEST_NO_LUA_SETUP
#include "test.lua.h"

test_lua_t g_test_lua;

void test_lua_setup(void)
{
    g_test_lua.L = luaL_newstate();
    ASSERT_NE_PTR(g_test_lua.L, NULL);

    luaL_openlibs(g_test_lua.L);

    ASSERT_EQ_INT(luaopen_ev(g_test_lua.L), 1);
    lua_setglobal(g_test_lua.L, "ev");

    lua_newtable(g_test_lua.L);
    lua_setglobal(g_test_lua.L, "test");
}

void test_lua_teardown(void)
{
    lua_close(g_test_lua.L);
    g_test_lua.L = NULL;
}

void test_lua_set_test_string(const char* key, const char* value)
{
    lua_getglobal(g_test_lua.L, "test");

    lua_pushstring(g_test_lua.L, value);
    lua_setfield(g_test_lua.L, -2, key);

    lua_pop(g_test_lua.L, 1);
}

void test_lua_set_test_integer(const char* key, int value)
{
    lua_getglobal(g_test_lua.L, "test");

    lua_pushinteger(g_test_lua.L, value);
    lua_setfield(g_test_lua.L, -2, key);

    lua_pop(g_test_lua.L, 1);
}
