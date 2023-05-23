#include "fs.lua.h"

#define LEV_FILE_NAME   "__ev_file"

typedef struct lev_file_s
{
    ev_file_t   file;
    ev_loop_t*  loop;

    int         closed;
    lua_State*  close_L;    /**< Lua state for close */
} lev_file_t;

typedef struct lev_file_open
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
} lev_file_open_t;

typedef struct lev_file_seek
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
} lev_file_seek_t;

typedef struct lev_file_read
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
    size_t      chunk_sz;

#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4200)
#endif
    char        buf[];
#if defined(_MSC_VER)
#    pragma warning(pop)
#endif
} lev_file_read_t;

typedef struct lev_file_write
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
} lev_file_write_t;

typedef struct lev_file_stat
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
} lev_file_stat_t;

typedef struct lev_file_remove
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
} lev_file_remove_t;

typedef struct lev_file_readdir
{
    ev_fs_req_t req;
    lua_State*  L;
    ev_loop_t*  loop;
} lev_file_readdir_t;

static void _lev_file_on_open(ev_fs_req_t* req)
{
    lev_file_open_t* token = EV_CONTAINER_OF(req, lev_file_open_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_file_on_open_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;

    lev_file_open_t* token = (lev_file_open_t*)ctx;
    int result = (int)token->req.result;
    ev_fs_req_cleanup(&token->req);

    if (result != 0)
    {
        lua_pushinteger(L, result);
        lua_pushnil(L);
        return 2;
    }

    lua_pop(L, 1); /* Remove token. Now file is on top of stack. */
    lua_pushnil(L);
    lua_insert(L, lua_gettop(L) - 1);

    return 2;
}

static int _lev_file_gc(lua_State* L)
{
    lev_file_t* file = lua_touserdata(L, 1);

    if (!file->closed)
    {
        ev_file_exit(&file->file, NULL);
        ev_loop_run(file->loop, EV_LOOP_MODE_NOWAIT);
        file->closed = 1;
    }

    return 0;
}

static void _lev_file_on_read_done(ev_fs_req_t* req)
{
    lev_file_read_t* token = EV_CONTAINER_OF(req, lev_file_read_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_file_on_read_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_file_read_t* token = (lev_file_read_t*)ctx;

    ssize_t result = token->req.result;
    ev_fs_req_cleanup(&token->req);

    if (result < 0)
    {
        lua_pushinteger(L, result);
        lua_pushnil(L);
        return 2;
    }

    lua_pushnil(L);
    if (result != 0)
    {
        lua_pushlstring(L, token->buf, result);
    }
    else
    {
        lua_pushnil(L);
    }
    return 2;
}

static int _lev_file_read_ex(lua_State* L, lev_file_t* file, size_t size)
{
    int ret;
    size_t malloc_size = sizeof(lev_file_read_t) + size;
    lev_file_read_t* token = lua_newuserdata(L, malloc_size);
    token->chunk_sz = size;

    ev_buf_t buf = ev_buf_make(token->buf, size);
    ret = ev_file_read(&file->file, &token->req, &buf, 1, _lev_file_on_read_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->L = L;
    token->loop = file->loop;
    lev_set_state(L, file->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_file_on_read_resume);
}

static int _lev_file_read(lua_State* L)
{
    lev_file_t* file = luaL_checkudata(L, 1, LEV_FILE_NAME);

    size_t read_size = 64 * 1024;
    if (lua_type(L, 2) == LUA_TNUMBER)
    {
        read_size = lua_tointeger(L, 2);
    }

    return _lev_file_read_ex(L, file, read_size);
}

static void _lev_file_on_write_done(ev_fs_req_t* req)
{
    lev_file_write_t* token = EV_CONTAINER_OF(req, lev_file_write_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_file_on_write_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_file_write_t* token = (lev_file_write_t*)ctx;

    ssize_t result = token->req.result;
    ev_fs_req_cleanup(&token->req);

    if (result < 0)
    {
        lua_pushinteger(L, result);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static int _lev_file_write(lua_State* L)
{
    int ret;
    lev_file_t* file = luaL_checkudata(L, 1, LEV_FILE_NAME);

    size_t data_sz = 0;
    const char* data = luaL_checklstring(L, 2, &data_sz);

    lev_file_write_t* token = lua_newuserdata(L, sizeof(lev_file_write_t));

    ev_buf_t buf = ev_buf_make((void*)data, data_sz);
    ret = ev_file_write(&file->file, &token->req, &buf, 1, _lev_file_on_write_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        return 1;
    }

    token->L = L;
    token->loop = file->loop;
    lev_set_state(L, token->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_file_on_write_resume);
}

static void _lev_file_on_seek_done(ev_fs_req_t* req)
{
    lev_file_seek_t* token = EV_CONTAINER_OF(req, lev_file_seek_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_file_on_seek_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_file_seek_t* token = (lev_file_seek_t*)ctx;

    ssize_t result = token->req.result;
    ev_fs_req_cleanup(&token->req);

    if (result < 0)
    {
        lua_pushinteger(L, result);
        lua_pushnil(L);
        return 2;
    }

    lua_pushnil(L);
    lua_pushinteger(L, result);
    return 2;
}

static int _lev_file_seek(lua_State* L)
{
    int ret;
    lev_file_t* file = luaL_checkudata(L, 1, LEV_FILE_NAME);
    int whence = (int)luaL_checkinteger(L, 2);
    ssize_t offset = luaL_checkinteger(L, 3);

    lev_file_seek_t* token = lua_newuserdata(L, sizeof(lev_file_seek_t));
    ret = ev_file_seek(&file->file, &token->req, whence, offset, _lev_file_on_seek_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->L = L;
    token->loop = file->loop;
    lev_set_state(L, token->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_file_on_seek_resume);
}

static void _lev_file_on_stat_done(ev_fs_req_t* req)
{
    lev_file_stat_t* token = EV_CONTAINER_OF(req, lev_file_stat_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_file_on_stat_resume(lua_State* L, int status, lua_KContext ctx)
{
#define PUSH_FIELD_UINT64(L, stat, field)  \
    do {\
        lua_pushinteger(L, (stat)->field);\
        lua_setfield(L, -2, #field);\
    } while (0)

#define PUSH_FILED_TIMESPEC(L, stat, field) \
    do {\
        lua_newtable(L);\
        lua_pushinteger(L, (stat)->field.tv_sec);\
        lua_setfield(L, -2, "tv_sec");\
        lua_pushinteger(L, (stat)->field.tv_nsec);\
        lua_setfield(L, -2, "tv_nsec");\
        lua_setfield(L, -2, #field);\
    } while (0)

    (void)status;
    lev_file_stat_t* token = (lev_file_stat_t*)ctx;

    int result = (int)token->req.result;
    ev_fs_req_cleanup(&token->req);

    if (result < 0)
    {
        lua_pushinteger(L, result);
        lua_pushnil(L);
        return 2;
    }
    
    ev_fs_stat_t* stat = ev_fs_get_statbuf(&token->req);

    lua_pushnil(L);
    lua_newtable(L);

    PUSH_FIELD_UINT64(L, stat, st_dev);
    PUSH_FIELD_UINT64(L, stat, st_ino);
    PUSH_FIELD_UINT64(L, stat, st_mode);
    PUSH_FIELD_UINT64(L, stat, st_nlink);
    PUSH_FIELD_UINT64(L, stat, st_uid);
    PUSH_FIELD_UINT64(L, stat, st_gid);
    PUSH_FIELD_UINT64(L, stat, st_rdev);
    PUSH_FIELD_UINT64(L, stat, st_size);
    PUSH_FIELD_UINT64(L, stat, st_blksize);
    PUSH_FIELD_UINT64(L, stat, st_blocks);
    PUSH_FIELD_UINT64(L, stat, st_flags);
    PUSH_FIELD_UINT64(L, stat, st_gen);

    PUSH_FILED_TIMESPEC(L, stat, st_atim);
    PUSH_FILED_TIMESPEC(L, stat, st_mtim);
    PUSH_FILED_TIMESPEC(L, stat, st_ctim);
    PUSH_FILED_TIMESPEC(L, stat, st_birthtim);

    return 2;

#undef PUSH_FIELD_UINT64
#undef PUSH_FILED_TIMESPEC
}

static int _lev_file_stat(lua_State* L)
{
    int ret;
    lev_file_t* file = luaL_checkudata(L, 1, LEV_FILE_NAME);
    lev_file_stat_t* token = lua_newuserdata(L, sizeof(lev_file_stat_t));

    ret = ev_file_stat(&file->file, &token->req, _lev_file_on_stat_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->L = L;
    token->loop = file->loop;
    lev_set_state(L, token->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_file_on_stat_resume);
}

static void _lev_file_on_close_done(ev_file_t* file)
{
    lev_file_t* impl = EV_CONTAINER_OF(file, lev_file_t, file);

    lev_set_state(impl->close_L, impl->loop, 1);
    impl->close_L = NULL;
}

static int _lev_file_on_close_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)L; (void)status; (void)ctx;
    return 0;
}

static int _lev_file_close(lua_State* L)
{
    lev_file_t* file = luaL_checkudata(L, 1, LEV_FILE_NAME);

    if (file->closed)
    {
        return 0;
    }

    file->closed = 1;
    file->close_L = L;
    ev_file_exit(&file->file, _lev_file_on_close_done);
    lev_set_state(L, file->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)NULL, _lev_file_on_close_resume);
}

static void _lev_fs_on_remove_done(ev_fs_req_t* req)
{
    lev_file_remove_t* token = EV_CONTAINER_OF(req, lev_file_remove_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_fs_on_remove_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_file_remove_t* token = (lev_file_remove_t*)ctx;

    int result = (int)token->req.result;
    ev_fs_req_cleanup(&token->req);

    if (result < 0)
    {
        lua_pushinteger(L, result);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static void _lev_fs_on_readdir_done(ev_fs_req_t* req)
{
    lev_file_readdir_t* token = EV_CONTAINER_OF(req, lev_file_readdir_t, req);
    lev_set_state(token->L, token->loop, 1);
}

static const char* _lev_fs_dirent_type_tostring(ev_dirent_type_t type)
{
    switch (type)
    {
    case EV_DIRENT_FILE:    return "file";
    case EV_DIRENT_DIR:     return "dir";
    case EV_DIRENT_LINK:    return "link";
    case EV_DIRENT_FIFO:    return "fifo";
    case EV_DIRENT_SOCKET:  return "socket";
    case EV_DIRENT_CHR:     return "chr";
    case EV_DIRENT_BLOCK:   return "block";
    default:                return "unknown";
    }
}

static int _lev_fs_on_readdir_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;

    lev_file_readdir_t* token = (lev_file_readdir_t*)ctx;
    if (token->req.result < 0)
    {
        lua_pushinteger(L, token->req.result);
        lua_pushnil(L);
        goto finish;
    }

    lua_pushnil(L);
    lua_newtable(L);

    ev_dirent_t* dir = ev_fs_get_first_dirent(&token->req);
    for (; dir != NULL; dir = ev_fs_get_next_dirent(dir))
    {
        lua_pushstring(L, _lev_fs_dirent_type_tostring(dir->type));
        lua_setfield(L, -2, dir->name);
    }

finish:
    ev_fs_req_cleanup(&token->req);
    return 2;
}

int lev_fs_file(lua_State* L)
{
    int ret;
    ev_loop_t* loop = lev_to_loop(L, 1);
    const char* path = luaL_checkstring(L, 2);
    int flags = EV_FS_O_RDONLY;
    if (lua_type(L, 3) == LUA_TNUMBER)
    {
        flags = (int)lua_tointeger(L, 3);
    }

    int mode = EV_FS_S_IRUSR | EV_FS_S_IWUSR;
    if (lua_type(L, 4) == LUA_TNUMBER)
    {
        mode = (int)lua_tointeger(L, 4);
    }

    lev_file_t* file = lua_newuserdata(L, sizeof(lev_file_t)); /* SP + 1 */
    file->closed = 0;
    file->loop = loop;

    if ((ret = ev_file_init(loop, &file->file)) != 0)
    {
        file->closed = 1;
        return lev_error(L, loop, ret, NULL);
    }

    static const luaL_Reg s_meta[] = {
        { "__gc",   _lev_file_gc },
        { NULL,     NULL },
    };
    static const luaL_Reg s_method[] = {
        { "close",  _lev_file_close },
        { "read",   _lev_file_read },
        { "seek",   _lev_file_seek },
        { "stat",   _lev_file_stat },
        { "write",  _lev_file_write },
        { NULL,     NULL },
    };
    if (luaL_newmetatable(L, LEV_FILE_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    lev_file_open_t* token = lua_newuserdata(L, sizeof(lev_file_open_t)); /* SP + 2 */
    ret = ev_file_open(&file->file, &token->req, path, flags, mode, _lev_file_on_open);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->L = L;
    token->loop = loop;
    lev_set_state(L, loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_file_on_open_resume);
}

int lev_fs_remove(lua_State* L)
{
    int ret;
    ev_loop_t* loop = lev_to_loop(L, 1);
    const char* path = luaL_checkstring(L, 2);
    int recursion = 0;
    if (lua_type(L, 3) == LUA_TNUMBER)
    {
        recursion = (int)lua_tonumber(L, 3);
    }

    lev_file_remove_t* token = lua_newuserdata(L, sizeof(lev_file_remove_t));

    ret = ev_fs_remove(loop, &token->req, path, recursion, _lev_fs_on_remove_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        return 1;
    }

    token->loop = loop;
    token->L = L;
    lev_set_state(L, loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_fs_on_remove_resume);
}

int lev_fs_readdir(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);
    const char* path = luaL_checkstring(L, 2);

    lev_file_readdir_t* token = lua_newuserdata(L, sizeof(lev_file_readdir_t));

    int ret = ev_fs_readdir(loop, &token->req, path, _lev_fs_on_readdir_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->loop = loop;
    token->L = L;
    lev_set_state(L, loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_fs_on_readdir_resume);
}

ev_file_t* lev_try_to_file(lua_State* L, int idx)
{
    lev_file_t* self = luaL_testudata(L, idx, LEV_FILE_NAME);
    return self != NULL ? &self->file : NULL;
}
