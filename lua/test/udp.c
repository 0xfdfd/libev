#include "test.lua.h"

TEST_F(lua, udp)
{
    static const char* script =
"local loop = ev.mkloop()\n"
"local port = 0\n"
"loop:co(function()\n"
"    local sock = loop:udp()\n"
"    sock:bind(ev.ip_addr(test.ip, port))\n"
"    local addr = sock:getsockname()\n"
"    _, port = ev.ip_name(addr)\n"
"    local data, addr = sock:recvfrom()\n"
"    assert(data == test.data)\n"
"    sock:send(data, addr)\n"
"end)\n"
"loop:co(function()\n"
"    local sock = loop:udp()\n"
"    sock:connect(ev.ip_addr(test.ip, port))\n"
"    sock:send(test.data)\n"
"    local data = sock:recv()\n"
"    assert(data == test.data)\n"
"end)\n"
"loop:run()\n";

    test_lua_set_test_string("ip", "127.0.0.1");
    test_lua_set_test_string("data", "a test data of hello world");

    ASSERT_EQ_INT(luaL_dostring(g_test_lua.L, script), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}
