#include "test.lua.h"

TEST_F(lua, channel)
{
    static const char* script =
"local loop = ev.mkloop()\n"
"local chan = loop:channel()\n"
"loop:co(function()\n"
"    for i=1,10 do\n"
"        chan:send(i)\n"
"    end\n"
"    chan:close()\n"
"end)\n"
"loop:co(function()\n"
"    for i=1,10 do\n"
"        local e, v = chan:recv()\n"
"        assert(e == nil)\n"
"        assert(v == i)\n"
"    end\n"
"    local e, v = chan:recv()\n"
"    assert(e ~= nil)\n"
"    assert(v == nil)\n"
"end)\n"
"loop:run()\n";

    ASSERT_EQ_INT(luaL_dostring(g_test_lua.L, script), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}
