#include "test.lua.h"

TEST_F(lua, promise)
{
    static const char* script =
"local loop = ev.loop()\n"
"local promise = loop:promise()\n"
"loop:co(function()\n"
"    local v = promise:get_value()\n"
"    assert(v == test.value)\n"
"end)\n"
"loop:co(function()\n"
"    promise:set_value(test.value)\n"
"end)\n"
"loop:run()\n";

    test_lua_set_test_string("value", "a7366500-985c-45c9-9ea8-8775a9cf745b");

    ASSERT_EQ_INT(luaL_dostring(g_test_lua.L, script), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}
