#include "test.lua.h"

TEST_F(lua, tcp)
{
    static const char* script =
"local loop = ev.mkloop()\n"
"local port = 0\n"
"loop:co(function()\n"
"    local sock = loop:tcp()\n"
"    sock:listen(ev.ip_addr(test.ip, port))\n"
"    _, port = ev.ip_name(sock:sockname())\n"
"    local client = sock:accept()\n"
"    local data = client:recv()\n"
"    assert(data == test.data)\n"
"    client:send(test.data)\n"
"end)\n"
"loop:co(function()\n"
"    local sock = loop:tcp()\n"
"    sock:connect(ev.ip_addr(test.ip, port))\n"
"    sock:send(test.data)\n"
"    local data = sock:recv()\n"
"    assert(data == test.data)\n"
"end)\n"
"loop:run()\n";

    test_lua_set_test_string("ip", "127.0.0.1");
    test_lua_set_test_string("data", "hello world");

    ASSERT_EQ_INT(luaL_dostring(g_test_lua.L, script), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}
