#include "test.lua.h"

TEST_F(lua, udp)
{
    static const char* script =
"local loop = ev.loop()\n"
"local p = loop:promise()\n"
"loop:co(function()\n"
"    local sock = loop:udp()\n"
"    local err = sock:bind(ev.ip_addr(test.ip, 0))\n"
"    assert(err == nil)\n"
"    local addr = sock:getsockname()\n"
"    p:set_value(ev.ip_name(addr))\n"
"    local _, data, addr = sock:recvfrom()\n"
"    assert(data == test.data)\n"
"    err = sock:send(data, addr)\n"
"    assert(err == nil)\n"
"end)\n"
"loop:co(function()\n"
"    local sock = loop:udp()\n"
"    local _, port = p:get_value()\n"
"    local err = sock:connect(ev.ip_addr(test.ip, port))\n"
"    assert(err == nil)\n"
"    err = sock:send(test.data)\n"
"    assert(err == nil)\n"
"    local _, data = sock:recv()\n"
"    assert(data == test.data)\n"
"end)\n"
"loop:run()\n";

    test_lua_set_test_string("ip", "127.0.0.1");
    test_lua_set_test_string("data", "a test data of hello world");

    ASSERT_EQ_INT(test_lua_dostring(g_test_lua.L, script), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}
