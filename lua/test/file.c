#include "test.lua.h"

TEST_F(lua, fs_file)
{
    static const char* script =
"local loop = ev.mkloop()\n"
"loop:co(function()\n"
"    local file = loop:fs_file(sample_file, ev.EV_FS_O_CREAT | ev.EV_FS_O_RDWR)\n"
"    assert(file ~= nil)\n"
"    file:write(\"hello world\")\n"
"    file:seek(ev.EV_FS_SEEK_BEG, 0)\n"
"    local data = file:read()\n"
"    assert(data == \"hello world\")\n"
"end)\n"
"loop:run()\n";

    lua_pushstring(g_test_lua.L, "sample_file_for_lua");
    lua_setglobal(g_test_lua.L, "sample_file");

    ASSERT_EQ_INT(luaL_dostring(g_test_lua.L, script), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}
