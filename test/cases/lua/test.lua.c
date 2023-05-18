#define EV_TEST_NO_LUA_SETUP
#include "test.lua.h"

test_lua_t g_test_lua = { NULL };

static int _msg_handler(lua_State* L)
{
    const char* msg = lua_tostring(L, 1);
    if (msg == NULL)
    {  /* is error object not a string? */
        if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
            lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
        {
            return 1;  /* that is the message */
        }
        else
        {
            msg = lua_pushfstring(L, "(error object is a %s value)",
                luaL_typename(L, 1));
        }
    }
    luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
    return 1;  /* return the traceback */
}

static void _test_lua_setup_arg(lua_State* L, const char** args)
{
    static const char* script =
        "for _, s in pairs(arg) do\n"
        "    for k,v in string.gmatch(s, \"--(%w+)=(.+)\") do\n"
        "        arg[k] = v\n"
        "    end\n"
        "end\n";

    int i;
    lua_newtable(L);
    for (i = 1; args != NULL && args[i] != NULL; i++)
    {
        lua_pushstring(L, args[i]);
        lua_rawseti(L, -2, i);
    }
    lua_setglobal(L, "arg");

    luaL_dostring(L, script);
}

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

int test_lua_dostring(lua_State* L, const char* s, const char** args)
{
    int ret;
    int sp = lua_gettop(L);

    _test_lua_setup_arg(L, args);

    lua_pushcfunction(L, _msg_handler); // sp + 1
    if ((ret = luaL_loadbuffer(L, s, strlen(s), s)) != LUA_OK) // sp + 2
    {
        return ret;
    }

    return  lua_pcall(L, 0, LUA_MULTRET, sp + 1);
}
