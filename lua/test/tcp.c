#include "test.lua.h"

TEST_F(lua, tcp)
{
    static const char* script =
"local loop = ev.loop()\n"
"local promise = loop:promise()\n"
"loop:co(function()\n"
"    local sock = loop:tcp()\n"
"    local err = sock:listen(ev.ip_addr(test.ip, 0))\n"
"    assert(err == nil)\n"
"    promise:set_value(ev.ip_name(sock:sockname()))\n"
"    local _, client = sock:accept()\n"
"    local _, data = client:recv()\n"
"    assert(data == test.data)\n"
"    client:send(test.data)\n"
"end)\n"
"loop:co(function()\n"
"    local sock = loop:tcp()\n"
"    local _, port = promise:get_value()\n"
"    local err = sock:connect(ev.ip_addr(test.ip, port))\n"
"    assert(err == nil)\n"
"    err = sock:send(test.data)\n"
"    assert(err == nil)\n"
"    local _, data = sock:recv()\n"
"    assert(data == test.data)\n"
"end)\n"
"loop:run()\n";

    test_lua_set_test_string("ip", "127.0.0.1");
    test_lua_set_test_string("data", "hello world");

    ASSERT_EQ_INT(luaL_dostring(g_test_lua.L, script), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}
