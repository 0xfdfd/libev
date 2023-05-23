#ifndef __EV_LUA_FS_H__
#define __EV_LUA_FS_H__

#include "ev.lua.internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LEV_FILE_FLAG_MAP(xx)   \
    xx(EV_FS_O_APPEND)          \
    xx(EV_FS_O_CREAT)           \
    xx(EV_FS_O_DSYNC)           \
    xx(EV_FS_O_EXCL)            \
    xx(EV_FS_O_SYNC)            \
    xx(EV_FS_O_TRUNC)           \
    xx(EV_FS_O_RDONLY)          \
    xx(EV_FS_O_WRONLY)          \
    xx(EV_FS_O_RDWR)            \
    \
    xx(EV_FS_S_IRUSR)           \
    xx(EV_FS_S_IWUSR)           \
    xx(EV_FS_S_IXUSR)           \
    xx(EV_FS_S_IRWXU)           \
    \
    xx(EV_FS_SEEK_BEG)          \
    xx(EV_FS_SEEK_CUR)          \
    xx(EV_FS_SEEK_END)

/**
 * @brief Open a file.
 * @param[in] L     Lua stack.
 * @return          Always 1.
 */
int lev_fs_file(lua_State* L);

/**
 * @brief Open a file or directory.
 * @param[in] L     Lua stack.
 * @return          Always 1.
 */
int lev_fs_remove(lua_State* L);

/**
 * @brief Read dir.
 * @param[in] L     Lua stack.
 * @return          Always 1.
 */
int lev_fs_readdir(lua_State* L);

/**
 * @brief Try to convert file handle.
 * @param[in] L     Lua stack.
 * @param[in] idx   Stack index.
 * @return          File handle.
 */
ev_file_t* lev_try_to_file(lua_State* L, int idx);

#ifdef __cplusplus
}
#endif

#endif
