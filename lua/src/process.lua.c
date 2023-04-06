#include "process.lua.h"
#include "fs.lua.h"
#include "pipe.lua.h"

#define LEV_PROCESS_NAME            "__ev_process"
#define LEV_PROCESS_WAITPID_NAME    "__ev_process_waitpid"

#if defined(_MSC_VER)
#   define strdup(s) _strdup(s)
#endif

typedef struct lev_process_s
{
    ev_process_t                process;
    ev_loop_t*                  loop;

    int                         ref_stdin;
    int                         ref_stdout;
    int                         ref_stderr;

    ev_list_t                   wait_queue;

    int                         exited;
    ev_process_exit_status_t    exit_status;
    int                         exit_code;
} lev_process_t;

typedef struct lev_process_wait_token
{
    ev_list_node_t              node;

    lua_State*                  L;
    ev_loop_t*                  loop;

    lev_process_t*              process;
} lev_process_wait_token_t;

static void _lev_process_cleanup_opt(ev_process_options_t* opt)
{
    size_t i;

    if (opt->file != NULL)
    {
        free((char*)opt->file);
        opt->file = NULL;
    }

    for (i = 0; opt->argv != NULL && opt->argv[i] != NULL; i++)
    {
        free(opt->argv[i]);
    }
    opt->argv = NULL;

    for (i = 0; opt->envp != NULL && opt->envp[i] != NULL; i++)
    {
        free(opt->envp[i]);
    }
    opt->envp = NULL;

    if (opt->cwd != NULL)
    {
        free((char*)opt->cwd);
        opt->cwd = NULL;
    }
}

static char** _lev_to_string_array(lua_State* L, ev_loop_t* loop, int idx)
{
    char** argv = NULL;
    size_t len = 1;

    lua_pushnil(L);
    while (lua_next(L, idx) != 0)
    {
        len++;

        char** new_argv = realloc(argv, sizeof(char*) * len);
        if (new_argv == NULL)
        {
            lev_error(L, loop, EV_ENOMEM, NULL);
            return NULL;
        }
        argv = new_argv;

        argv[len - 2] = strdup(luaL_checkstring(L, -1));
        argv[len - 1] = NULL;

        lua_pop(L, 1);
    }

    return argv;
}

static int _lev_process_opt_stdio(lua_State* L, int idx, lev_process_t* process, ev_process_options_t* opt)
{
#define LEV_PIPE_CHECK_STDIO_AS_PIPE(name, arr)  \
    do {\
        ev_pipe_t* pipe;\
        if (lua_getfield(L, idx, #name) == LUA_TUSERDATA) {\
            if ((pipe = lev_try_to_pipe(L, top_sp + 1)) != NULL) {\
                opt->stdios[arr].flag = EV_PROCESS_STDIO_REDIRECT_PIPE;\
                opt->stdios[arr].data.pipe = pipe;\
                lua_pushvalue(L, -1);\
                process->ref_##name = luaL_ref(L, LUA_REGISTRYINDEX);\
            }\
        }\
        lua_pop(L, 1);\
    } while (0)

#define LEV_PIPE_CHECK_STDIO_AS_FILE(name, arr) \
    do {\
        ev_file_t* file;\
        if (lua_getfield(L, idx, #name) == LUA_TUSERDATA) {\
            if ((file = lev_try_to_file(L, top_sp + 1)) != NULL) {\
                opt->stdios[arr].flag = EV_PROCESS_STDIO_REDIRECT_FD;\
                opt->stdios[arr].data.fd = file->file;\
                lua_pushvalue(L, -1);\
                process->ref_##name = luaL_ref(L, LUA_REGISTRYINDEX);\
            }\
        }\
        lua_pop(L, 1);\
    } while (0)

    int top_sp = lua_gettop(L);

    LEV_PIPE_CHECK_STDIO_AS_PIPE(stdin,  0);
    LEV_PIPE_CHECK_STDIO_AS_PIPE(stdout, 1);
    LEV_PIPE_CHECK_STDIO_AS_PIPE(stderr, 2);

    LEV_PIPE_CHECK_STDIO_AS_FILE(stdin,  0);
    LEV_PIPE_CHECK_STDIO_AS_FILE(stdout, 1);
    LEV_PIPE_CHECK_STDIO_AS_FILE(stderr, 2);

    return 0;

#undef LEV_PIPE_CHECK_STDIO_AS_PIPE
#undef LEV_PIPE_CHECK_STDIO_AS_FILE
}

static int _lev_process_opt(lua_State* L, ev_loop_t* loop, int idx,
    lev_process_t* process, ev_process_options_t* opt)
{
    int sp = lua_gettop(L);
    if (lua_type(L, idx) != LUA_TTABLE)
    {
        return 0;
    }

    if (lua_getfield(L, idx, "file") == LUA_TSTRING)
    {
        free((char*)opt->file);
        opt->file = strdup(lua_tostring(L, -1));
    }
    lua_pop(L, 1);

    if (lua_getfield(L, idx, "argv") == LUA_TTABLE) /* SP + 1 */
    {
        opt->argv = _lev_to_string_array(L, loop, sp + 1);
    }
    lua_pop(L, 1);

    if (lua_getfield(L, idx, "envp") == LUA_TTABLE)
    {
        opt->envp = _lev_to_string_array(L, loop, sp + 1);
    }
    lua_pop(L, 1);

    if (lua_getfield(L, idx, "cwd") == LUA_TSTRING)
    {
        opt->cwd = strdup(lua_tostring(L, -1));
    }
    lua_pop(L, 1);

    _lev_process_opt_stdio(L, idx, process, opt);

    return 0;
}

static int _lev_process_gc(lua_State* L)
{
    lev_process_t* process = lua_touserdata(L, 1);

    if (process->ref_stdin != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, process->ref_stdin);
        process->ref_stdin = LUA_NOREF;
    }
    if (process->ref_stdout != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, process->ref_stdout);
        process->ref_stdout = LUA_NOREF;
    }
    if (process->ref_stderr != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, process->ref_stderr);
        process->ref_stderr = LUA_NOREF;
    }

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&process->wait_queue)) != NULL)
    {
        lev_process_wait_token_t* token = EV_CONTAINER_OF(it, lev_process_wait_token_t, node);
        lev_set_state(token->L, token->loop, 1);
        token->process = NULL;
    }

    ev_process_exit(&process->process, NULL);
    ev_loop_run(process->loop, EV_LOOP_MODE_NOWAIT);

    return 0;
}

static int _lev_path_template(lua_State* L, ssize_t(*cb)(char*, size_t))
{
    size_t size = 4096;
    char* buffer = ev_malloc(size);
    if (buffer == NULL)
    {
        goto err_nomem;
    }

    ssize_t len = cb(buffer, size);
    if (len < 0)
    {
        ev_free(buffer); buffer = NULL;
        return lev_error(L, NULL, (int)len, NULL);
    }

    if ((size_t)len >= size)
    {
        char* new_buf = ev_realloc(buffer, len + 1);
        if (new_buf == NULL)
        {
            goto err_nomem;
        }
        buffer = new_buf;

        len = cb(buffer, len + 1);
    }

    lua_pushlstring(L, buffer, len);

    ev_free(buffer);
    buffer = NULL;

    return 1;

err_nomem:
    if (buffer != NULL)
    {
        ev_free(buffer);
        buffer = NULL;
    }
    return lev_error(L, NULL, EV_ENOMEM, NULL);
}

static void _lev_process_on_exit(ev_process_t* handle,
    ev_process_exit_status_t exit_status, int exit_code)
{
    lev_process_t* self = EV_CONTAINER_OF(handle, lev_process_t, process);

    self->exited = 1;
    self->exit_status = exit_status;
    self->exit_code = exit_code;

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&self->wait_queue)) != NULL)
    {
        lev_process_wait_token_t* token = EV_CONTAINER_OF(it, lev_process_wait_token_t, node);
        lev_set_state(token->L, token->loop, 1);
        token->process = NULL;
    }
}

static int _lev_process_waitpid_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_process_wait_token_t* token = (lev_process_wait_token_t*)ctx;

    if (token->process == NULL)
    {
        return 0;
    }

    lua_pushinteger(L, token->process->exit_code);
    return 1;
}

static int _lev_process_waitpid_gc(lua_State* L)
{
    lev_process_wait_token_t* token = lua_touserdata(L, 1);

    if (token->process != NULL)
    {
        ev_list_erase(&token->process->wait_queue, &token->node);
        token->process = NULL;
    }

    return 0;
}

static int _lev_process_waitpid(lua_State* L)
{
    lev_process_t* self = luaL_checkudata(L, 1, LEV_PROCESS_NAME);

    if (self->exited)
    {
        lua_pushinteger(L, self->exit_code);
        return 1;
    }

    lev_process_wait_token_t* token = lua_newuserdata(L, sizeof(lev_process_wait_token_t));
    token->L = L;
    token->loop = self->loop;
    token->process = self;

    static const luaL_Reg s_meta[] = {
         { "__gc",       _lev_process_waitpid_gc },
         { NULL,         NULL },
    };
    if (luaL_newmetatable(L, LEV_PROCESS_WAITPID_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);
    }
    lua_setmetatable(L, -2);

    ev_list_push_back(&self->wait_queue, &token->node);
    lev_set_state(L, token->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_process_waitpid_resume);
}

int lev_process(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);

    lev_process_t* process = lua_newuserdata(L, sizeof(lev_process_t));
    memset(process, 0, sizeof(*process));
    ev_list_init(&process->wait_queue);
    process->ref_stdin = LUA_NOREF;
    process->ref_stdout = LUA_NOREF;
    process->ref_stderr = LUA_NOREF;

    ev_process_options_t opt;
    memset(&opt, 0, sizeof(opt));
    opt.on_exit = _lev_process_on_exit;
    _lev_process_opt(L, loop, 2, process, &opt);
    process->loop = loop;

    int ret = ev_process_spawn(loop, &process->process, &opt);
    _lev_process_cleanup_opt(&opt);

    if (ret != 0)
    {
        return lev_error(L, loop, ret, NULL);
    }

    static const luaL_Reg s_meta[] = {
        { "__gc",       _lev_process_gc },
        { NULL,         NULL },
    };
    static const luaL_Reg s_method[] = {
        { "waitpid",    _lev_process_waitpid },
        { NULL,         NULL },
    };
    if (luaL_newmetatable(L, LEV_PROCESS_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    return 1;
}

int lev_getcwd(lua_State* L)
{
    return _lev_path_template(L, ev_getcwd);
}

int lev_exepath(lua_State* L)
{
    return _lev_path_template(L, ev_exepath);
}
