#include <stdlib.h>
#include <stdio.h>
#include "function.h"

#define CRLF "\n"

typedef struct global_ctx
{
    lua_State* L;
} global_ctx_t;

static global_ctx_t _G = { NULL };

static const char* _script =
"local opt = {" CRLF
"    no_line = false" CRLF
"}" CRLF

"local out_path = nil" CRLF
"local out_data = \"\"" CRLF

"local function split_string (inputstr)" CRLF
"    local sep = \",\"" CRLF
"    local t={}" CRLF
"    for str in string.gmatch(inputstr, \"([^\" .. sep .. \"]+)\") do" CRLF
"        table.insert(t, str)" CRLF
"    end" CRLF
"    return t" CRLF
"end" CRLF

"-- Read file and return content" CRLF
"local function read_file(path)" CRLF
"    local file_dat = am.readfile(path)" CRLF
"    local lines = am.split_line(file_dat)" CRLF
"    local data = \"\"" CRLF
"    for _,v in ipairs(lines) do" CRLF
"        data = data .. v .. \"\\n\"" CRLF
"    end" CRLF
"    return {" CRLF
"        data = data," CRLF
"        path = path," CRLF
"        sha256 = am.sha256(data)," CRLF
"    }" CRLF
"end" CRLF

"local function append_source(paths)" CRLF
"    local path_list = split_string(paths)" CRLF
"    for _,v in ipairs(path_list) do" CRLF
"        local info = read_file(v)" CRLF
"        local pattern = \"#%s*include%s+\\\"([-_%w%./]+)%.h\\\"\"" CRLF
"        local replace = \"/* AMALGAMATE: %0 */\"" CRLF
"        local content = string.gsub(info.data, pattern, replace)" CRLF
"        out_data = out_data .. \"////////////////////////////////////////////////////////////////////////////////\\n\"" CRLF
"        out_data = out_data .. \"// FILE:    \" .. info.path .. \"\\n\"" CRLF
"        out_data = out_data .. \"// SIZE:    \" .. string.format(\"%q\", #info.data) .. \"\\n\"" CRLF
"        out_data = out_data .. \"// SHA-256: \" .. info.sha256 .. \"\\n\"" CRLF
"        out_data = out_data .. \"////////////////////////////////////////////////////////////////////////////////\\n\"" CRLF
"        if opt.no_line == false then" CRLF
"            out_data = out_data .. \"#line 1 \\\"\" .. v .. \"\\\"\\n\"" CRLF
"        end" CRLF
"        out_data = out_data .. content .. \"\\n\"" CRLF
"    end" CRLF
"end" CRLF

"local function append_commit(path)" CRLF
"    out_data = out_data .. \"/**\\n\"" CRLF
"    local file_dat = am.readfile(path)" CRLF
"    local data = am.split_line(file_dat)" CRLF
"    for _,v in ipairs(data) do" CRLF
"        out_data = out_data .. \" * \" .. v .. \"\\n\"" CRLF
"    end" CRLF
"    out_data = out_data .. \" */\\n\"" CRLF
"end" CRLF

"local function append_string(str)" CRLF
"    out_data = out_data .. str .. \"\\n\"" CRLF
"end" CRLF

"for i,v in ipairs(arg) do" CRLF
"    if v == \"--no_line\" then" CRLF
"        opt.no_line = true" CRLF
"    end" CRLF
"" CRLF
"    if v == \"--out\" then" CRLF
"        out_path = arg[i+1]" CRLF
"    end" CRLF
"" CRLF
"    if v == \"--commit\" then" CRLF
"        append_commit(arg[i+1])" CRLF
"    end" CRLF
"" CRLF
"    if v == \"--source\" then" CRLF
"        append_source(arg[i+1])" CRLF
"    end" CRLF
"" CRLF
"    if v == \"--string\" then" CRLF
"        append_string(arg[i+1])" CRLF
"    end" CRLF
"end" CRLF

"assert(out_path ~= nil)" CRLF

"-- Write file" CRLF
"am.writefile(out_path, out_data)" CRLF
;

static void _generate_arg_table(lua_State* L, int argc, char* argv[])
{
    int i;

    lua_newtable(L);
    for (i = 0; i < argc; i++)
    {
        lua_pushstring(L, argv[i]);
        lua_seti(L, -2, i + 1);
    }
    lua_setglobal(L, "arg");
}

static void _open_libs(lua_State* L)
{
    static const luaL_Reg api_list[] = {
        { "sha256",     am_sha256 },
        { "readfile",   am_readfile },
        { "split_line", am_split_line },
        { "writefile",  am_writefile },
        { NULL,     NULL },
    };
    luaL_newlib(L, api_list);
    lua_setglobal(L, "am");
}

static int _pmain(lua_State* L)
{
    int argc = (int)lua_tointeger(L, 1);
    char** argv = lua_touserdata(L, 2);

    luaL_openlibs(L);
    _open_libs(L);
    _generate_arg_table(L, argc, argv);

    if (luaL_loadstring(L, _script) != LUA_OK)
    {
        return lua_error(L);
    }
    lua_call(L, 0, LUA_MULTRET);

    return 0;
}

/**
 * @brief Global cleanup hook.
 */
static void _at_exit(void)
{
    if (_G.L != NULL)
    {
        lua_close(_G.L);
        _G.L = NULL;
    }
}

/**
 * @brief Lua error traceback helper.
 * @param[in] L     Lua VM.
 * @return          Always 1.
 */
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

int main(int argc, char* argv[])
{
    atexit(_at_exit);

    if ((_G.L = luaL_newstate()) == NULL)
    {
        fprintf(stderr, "create Lua VM failed.\n");
        abort();
    }

    lua_pushcfunction(_G.L, _msg_handler);
    lua_pushcfunction(_G.L, _pmain);
    lua_pushinteger(_G.L, argc);
    lua_pushlightuserdata(_G.L, argv);
    if (lua_pcall(_G.L, 2, 0, 1) != LUA_OK)
    {
        fprintf(stderr, "%s", lua_tostring(_G.L, -1));
        exit(EXIT_FAILURE);
    }

    return 0;
}
