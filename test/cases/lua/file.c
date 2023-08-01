#define EV_TEST_NO_LUA_SETUP
#include "test.lua.h"

#define TMPFILE_PATH    "364ff706-a89b-4ad0-8819-c3896b2e302a"

TEST_FIXTURE_SETUP(lua)
{
    ev_fs_remove_sync(TMPFILE_PATH, 1);
    test_lua_setup();
}

TEST_FIXTURE_TEARDOWN(lua)
{
    test_lua_teardown();
    ev_fs_remove_sync(TMPFILE_PATH, 1);
}

TEST_F(lua, DISABLED_fs_file)
{
    static const char* script =
"local loop = ev.loop()\n"
"loop:co(function()\n"
"    local _, file = loop:fs_file(arg[1], ev.EV_FS_O_CREAT | ev.EV_FS_O_RDWR)\n"
"    assert(file ~= nil)\n"
"    file:write(\"hello world\")\n"
"    file:seek(ev.EV_FS_SEEK_BEG, 0)\n"
"    local _, data = file:read()\n"
"    assert(data == \"hello world\")\n"
"end)\n"
"loop:run()\n";

    TEST_CALL_LUA(script, TMPFILE_PATH);
}

TEST_F(lua, DISABLED_fs_file_read)
{
    static const char* s =
"local loop = ev.loop()\n"
"loop:co(function()\n"
"    local err,file = loop:fs_file(arg[1])\n"
"    assert(err == nil)\n"
"    assert(file ~= nil)\n"
"    local data = \"\"\n"
"    while true do\n"
"        local err,content = file:read()\n"
"        assert(err == nil)\n"
"        if content == nil then\n"
"            break\n"
"        end\n"
"        data = data .. content\n"
"    end\n"
"    file:close()\n"
"end)\n"
"loop:run()\n";

    TEST_CALL_LUA(s, test_get_self_exe());
}

TEST_F(lua, DISABLED_fs_readdir)
{
    static const char* script =
"local loop = ev.loop()\n"
"local dir = {}\n"
"loop:co(function()\n"
"    local err,ret = loop:fs_readdir(arg[1])\n"
"    assert(err == nil)\n"
"    dir = ret\n"
"end)\n"
"loop:run()\n"
"local flag_have_info = false\n"
"for k,v in pairs(dir) do\n"
"    if k == arg[2] then\n"
"        flag_have_info = true\n"
"    end\n"
"end\n"
"assert(flag_have_info == true)\n";

    TEST_CALL_LUA(script, test_get_self_dir(), test_self_exe_name());
}
