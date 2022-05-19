#include "loop.h"
#include "fs.h"
#include "allocator.h"
#include "loop_win.h"
#include "win/winapi.h"

#define MILLION ((int64_t) 1000 * 1000)
#define BILLION ((int64_t) 1000 * 1000 * 1000)
#define NSEC_IN_SEC (1 * 1000 * 1000 * 1000)

#ifndef S_IFLNK
#   define S_IFLNK 0xA000
#endif

typedef struct file_open_info_win_s
{
    DWORD   access;
    DWORD   share;
    DWORD   attributes;
    DWORD   disposition;
}file_open_info_win_t;

/**
 * @brief Maps a character string to a UTF-16 (wide character) string.
 * @param[out] dst  Pointer to store wide string. Use #ev__free() to release it.
 * @param[in] src   Source string.
 * @return          The number of characters (not bytes) of \p dst, or #ev_errno_t if error.
 */
static ssize_t _ev_fs_utf8_to_wide(WCHAR** dst, const char* src)
{
    int errcode;
    ssize_t pathw_len = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
    if (pathw_len == 0)
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    size_t buf_sz = pathw_len * sizeof(WCHAR);
    WCHAR* buf = ev__malloc(buf_sz);
    if (buf == NULL)
    {
        return EV_ENOMEM;
    }

    int cchwidechar = (int)pathw_len;

    // check if truncate happen.
    assert((ssize_t)cchwidechar == pathw_len);

    DWORD r = MultiByteToWideChar(CP_UTF8, 0, src, -1, buf, cchwidechar);
    assert(r == (DWORD)pathw_len); (void)r;

    *dst = buf;

    return pathw_len;
}

static int _ev_fs_wide_to_utf8(WCHAR* w_source_ptr, DWORD w_source_len,
    char** target_ptr, uint64_t* target_len_ptr)
{
    int r;
    int target_len;
    char* target;
    target_len = WideCharToMultiByte(CP_UTF8,
        0,
        w_source_ptr,
        w_source_len,
        NULL,
        0,
        NULL,
        NULL);

    if (target_len == 0)
    {
        return -1;
    }

    if (target_len_ptr != NULL)
    {
        *target_len_ptr = target_len;
    }

    if (target_ptr == NULL)
    {
        return 0;
    }

    target = ev__malloc(target_len + 1);
    if (target == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return -1;
    }

    r = WideCharToMultiByte(CP_UTF8,
        0,
        w_source_ptr,
        w_source_len,
        target,
        target_len,
        NULL,
        NULL);
    assert(r == target_len);
    target[target_len] = '\0';
    *target_ptr = target;
    return 0;
}

static int _ev_file_get_open_attributes(int flags, int mode, file_open_info_win_t* info)
{
    info->access = 0;
    info->share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    info->attributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS;
    info->disposition = 0;

    switch (flags & (EV_FS_O_RDONLY | EV_FS_O_WRONLY | EV_FS_O_RDWR))
    {
    case EV_FS_O_RDONLY:
        info->access = FILE_GENERIC_READ;
        break;
    case EV_FS_O_WRONLY:
        info->access = FILE_GENERIC_WRITE;
        break;
    case EV_FS_O_RDWR:
        info->access = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
        break;
    default:
        return EV_EINVAL;
    }

    if (flags & EV_FS_O_APPEND)
    {
        info->access &= ~FILE_WRITE_DATA;
        info->access |= FILE_APPEND_DATA;
    }

    switch (flags & (EV_FS_O_CREAT | EV_FS_O_EXCL | EV_FS_O_TRUNC))
    {
    case 0:
    case EV_FS_O_EXCL:
        info->disposition = OPEN_EXISTING;
        break;
    case EV_FS_O_CREAT:
        info->disposition = OPEN_ALWAYS;
        break;
    case EV_FS_O_CREAT | EV_FS_O_EXCL:
    case EV_FS_O_CREAT | EV_FS_O_TRUNC | EV_FS_O_EXCL:
        info->disposition = CREATE_NEW;
        break;
    case EV_FS_O_TRUNC:
    case EV_FS_O_TRUNC | EV_FS_O_EXCL:
        info->disposition = TRUNCATE_EXISTING;
        break;
    case EV_FS_O_CREAT | EV_FS_O_TRUNC:
        info->disposition = CREATE_ALWAYS;
        break;
    default:
        return EV_EINVAL;
    }

    if ((flags & EV_FS_O_CREAT) && !(mode & _S_IWRITE))
    {
        info->attributes |= FILE_ATTRIBUTE_READONLY;
    }

    switch (flags & (EV_FS_O_DSYNC | EV_FS_O_SYNC))
    {
    case 0:
        break;
    case EV_FS_O_DSYNC:
#if EV_FS_O_SYNC != EV_FS_O_DSYNC
    case EV_FS_O_SYNC:
#endif
        info->attributes |= FILE_FLAG_WRITE_THROUGH;
        break;
    default:
        return EV_EINVAL;
    }

    return EV_SUCCESS;
}

int ev__fs_open(ev_os_file_t* file, const char* path, int flags, int mode)
{
    int ret;
    DWORD errcode;

    file_open_info_win_t info;
    ret = _ev_file_get_open_attributes(flags, mode, &info);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    HANDLE filehandle = CreateFile(path, info.access, info.share, NULL,
        info.disposition, info.attributes, NULL);
    if (filehandle == INVALID_HANDLE_VALUE)
    {
        errcode = GetLastError();
        if (errcode == ERROR_FILE_EXISTS && (flags & EV_FS_O_CREAT) && !(flags & EV_FS_O_EXCL))
        {
            return EV_EISDIR;
        }

        return ev__translate_sys_error(errcode);
    }

    *file = filehandle;
    return EV_SUCCESS;
}

ssize_t ev__fs_preadv(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf, ssize_t offset)
{
    if (file == INVALID_HANDLE_VALUE)
    {
        return ev__translate_sys_error(ERROR_INVALID_HANDLE);
    }

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));

    size_t idx;
    LARGE_INTEGER offset_;
    DWORD bytes = 0;
    BOOL read_ret = TRUE;
    for (idx = 0; read_ret && idx < nbuf; idx++)
    {
        offset_.QuadPart = offset + bytes;
        overlapped.Offset = offset_.LowPart;
        overlapped.OffsetHigh = offset_.HighPart;

        DWORD incremental_bytes;
        read_ret = ReadFile(file, bufs[idx].data, bufs[idx].size,
            &incremental_bytes, &overlapped);
        bytes += incremental_bytes;
    }

    if (read_ret || bytes > 0)
    {
        return bytes;
    }

    DWORD err = GetLastError();
    if (err == ERROR_HANDLE_EOF || err == ERROR_BROKEN_PIPE)
    {
        return bytes;
    }

    if (err == ERROR_ACCESS_DENIED)
    {
        err = ERROR_INVALID_FLAGS;
    }
    return ev__translate_sys_error(err);
}

ssize_t ev__fs_pwritev(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf, ssize_t offset)
{
    if (file == INVALID_HANDLE_VALUE)
    {
        return ev__translate_sys_error(ERROR_INVALID_HANDLE);
    }

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));

    size_t idx;
    LARGE_INTEGER offset_;
    DWORD bytes = 0;
    BOOL write_ret = TRUE;
    for (idx = 0; write_ret && idx < nbuf; idx++)
    {
        offset_.QuadPart = offset + bytes;
        overlapped.Offset = offset_.LowPart;
        overlapped.OffsetHigh = offset_.HighPart;

        DWORD incremental_bytes;
        write_ret = WriteFile(file, bufs[idx].data,
            bufs[idx].size, &incremental_bytes, &overlapped);
        bytes += incremental_bytes;
    }

    if (write_ret || bytes > 0)
    {
        return bytes;
    }

    DWORD err = GetLastError();
    if (err == ERROR_ACCESS_DENIED)
    {
        err = ERROR_INVALID_FLAGS;
    }
    return ev__translate_sys_error(err);
}

static int _ev_fs_readlink_handle(HANDLE handle, char** target_ptr,
    uint64_t* target_len_ptr)
{
    char buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
    REPARSE_DATA_BUFFER* reparse_data = (REPARSE_DATA_BUFFER*)buffer;
    WCHAR* w_target;
    DWORD w_target_len;
    DWORD bytes;
    size_t i;
    size_t len;

    if (!DeviceIoControl(handle,
        FSCTL_GET_REPARSE_POINT,
        NULL,
        0,
        buffer,
        sizeof buffer,
        &bytes,
        NULL)) {
        return -1;
    }

    if (reparse_data->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
        /* Real symlink */
        w_target = reparse_data->SymbolicLinkReparseBuffer.PathBuffer +
            (reparse_data->SymbolicLinkReparseBuffer.SubstituteNameOffset /
                sizeof(WCHAR));
        w_target_len =
            reparse_data->SymbolicLinkReparseBuffer.SubstituteNameLength /
            sizeof(WCHAR);

        /* Real symlinks can contain pretty much everything, but the only thing we
         * really care about is undoing the implicit conversion to an NT namespaced
         * path that CreateSymbolicLink will perform on absolute paths. If the path
         * is win32-namespaced then the user must have explicitly made it so, and
         * we better just return the unmodified reparse data. */
        if (w_target_len >= 4 &&
            w_target[0] == L'\\' &&
            w_target[1] == L'?' &&
            w_target[2] == L'?' &&
            w_target[3] == L'\\') {
            /* Starts with \??\ */
            if (w_target_len >= 6 &&
                ((w_target[4] >= L'A' && w_target[4] <= L'Z') ||
                    (w_target[4] >= L'a' && w_target[4] <= L'z')) &&
                w_target[5] == L':' &&
                (w_target_len == 6 || w_target[6] == L'\\')) {
                /* \??\<drive>:\ */
                w_target += 4;
                w_target_len -= 4;

            }
            else if (w_target_len >= 8 &&
                (w_target[4] == L'U' || w_target[4] == L'u') &&
                (w_target[5] == L'N' || w_target[5] == L'n') &&
                (w_target[6] == L'C' || w_target[6] == L'c') &&
                w_target[7] == L'\\') {
                /* \??\UNC\<server>\<share>\ - make sure the final path looks like
                 * \\<server>\<share>\ */
                w_target += 6;
                w_target[0] = L'\\';
                w_target_len -= 6;
            }
        }

    }
    else if (reparse_data->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
        /* Junction. */
        w_target = reparse_data->MountPointReparseBuffer.PathBuffer +
            (reparse_data->MountPointReparseBuffer.SubstituteNameOffset /
                sizeof(WCHAR));
        w_target_len = reparse_data->MountPointReparseBuffer.SubstituteNameLength /
            sizeof(WCHAR);

        /* Only treat junctions that look like \??\<drive>:\ as symlink. Junctions
         * can also be used as mount points, like \??\Volume{<guid>}, but that's
         * confusing for programs since they wouldn't be able to actually
         * understand such a path when returned by uv_readlink(). UNC paths are
         * never valid for junctions so we don't care about them. */
        if (!(w_target_len >= 6 &&
            w_target[0] == L'\\' &&
            w_target[1] == L'?' &&
            w_target[2] == L'?' &&
            w_target[3] == L'\\' &&
            ((w_target[4] >= L'A' && w_target[4] <= L'Z') ||
                (w_target[4] >= L'a' && w_target[4] <= L'z')) &&
            w_target[5] == L':' &&
            (w_target_len == 6 || w_target[6] == L'\\'))) {
            SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
            return -1;
        }

        /* Remove leading \??\ */
        w_target += 4;
        w_target_len -= 4;

    }
    else if (reparse_data->ReparseTag == IO_REPARSE_TAG_APPEXECLINK) {
        /* String #3 in the list has the target filename. */
        if (reparse_data->AppExecLinkReparseBuffer.StringCount < 3) {
            SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
            return -1;
        }
        w_target = reparse_data->AppExecLinkReparseBuffer.StringList;
        /* The StringList buffer contains a list of strings separated by "\0",   */
        /* with "\0\0" terminating the list. Move to the 3rd string in the list: */
        for (i = 0; i < 2; ++i) {
            len = wcslen(w_target);
            if (len == 0) {
                SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
                return -1;
            }
            w_target += len + 1;
        }
        w_target_len = (DWORD)wcslen(w_target);
        if (w_target_len == 0) {
            SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
            return -1;
        }
        /* Make sure it is an absolute path. */
        if (!(w_target_len >= 3 &&
            ((w_target[0] >= L'a' && w_target[0] <= L'z') ||
                (w_target[0] >= L'A' && w_target[0] <= L'Z')) &&
            w_target[1] == L':' &&
            w_target[2] == L'\\')) {
            SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
            return -1;
        }

    }
    else {
        /* Reparse tag does not indicate a symlink. */
        SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
        return -1;
    }

    return _ev_fs_wide_to_utf8(w_target, w_target_len, target_ptr, target_len_ptr);
}

static void _ev_filetime_to_timespec(ev_timespec_t* ts, int64_t filetime)
{
    filetime -= 116444736 * BILLION;
    ts->tv_sec = (long)(filetime / (10 * MILLION));
    ts->tv_nsec = (long)((filetime - ts->tv_sec * 10 * MILLION) * 100U);
    if (ts->tv_nsec < 0)
    {
        ts->tv_sec -= 1;
        ts->tv_nsec += NSEC_IN_SEC;
    }
}

static int _ev_file_wrap_fstat_win(HANDLE handle, ev_fs_stat_t* statbuf, int do_lstat)
{
    FILE_ALL_INFORMATION file_info;
    FILE_FS_VOLUME_INFORMATION volume_info;
    NTSTATUS nt_status;
    IO_STATUS_BLOCK io_status;

    nt_status = ev_winapi.NtQueryInformationFile(handle,
        &io_status,
        &file_info,
        sizeof(file_info),
        FileAllInformation);

    /* Buffer overflow (a warning status code) is expected here. */
    if (NT_ERROR(nt_status))
    {
        SetLastError(ev_winapi.RtlNtStatusToDosError(nt_status));
        return -1;
    }

    nt_status = ev_winapi.NtQueryVolumeInformationFile(handle,
        &io_status,
        &volume_info,
        sizeof volume_info,
        FileFsVolumeInformation);

    /* Buffer overflow (a warning status code) is expected here. */
    if (io_status.Status == STATUS_NOT_IMPLEMENTED)
    {
        statbuf->st_dev = 0;
    }
    else if (NT_ERROR(nt_status))
    {
        SetLastError(ev_winapi.RtlNtStatusToDosError(nt_status));
        return -1;
    }
    else
    {
        statbuf->st_dev = volume_info.VolumeSerialNumber;
    }

    /* Todo: st_mode should probably always be 0666 for everyone. We might also
     * want to report 0777 if the file is a .exe or a directory.
     *
     * Currently it's based on whether the 'readonly' attribute is set, which
     * makes little sense because the semantics are so different: the 'read-only'
     * flag is just a way for a user to protect against accidental deletion, and
     * serves no security purpose. Windows uses ACLs for that.
     *
     * Also people now use uv_fs_chmod() to take away the writable bit for good
     * reasons. Windows however just makes the file read-only, which makes it
     * impossible to delete the file afterwards, since read-only files can't be
     * deleted.
     *
     * IOW it's all just a clusterfuck and we should think of something that
     * makes slightly more sense.
     *
     * And uv_fs_chmod should probably just fail on windows or be a total no-op.
     * There's nothing sensible it can do anyway.
     */
    statbuf->st_mode = 0;

    /*
    * On Windows, FILE_ATTRIBUTE_REPARSE_POINT is a general purpose mechanism
    * by which filesystem drivers can intercept and alter file system requests.
    *
    * The only reparse points we care about are symlinks and mount points, both
    * of which are treated as POSIX symlinks. Further, we only care when
    * invoked via lstat, which seeks information about the link instead of its
    * target. Otherwise, reparse points must be treated as regular files.
    */
    if (do_lstat &&
        (file_info.BasicInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
    {
        /*
         * If reading the link fails, the reparse point is not a symlink and needs
         * to be treated as a regular file. The higher level lstat function will
         * detect this failure and retry without do_lstat if appropriate.
         */
        if (_ev_fs_readlink_handle(handle, NULL, &statbuf->st_size) != 0)
            return -1;
        statbuf->st_mode |= S_IFLNK;
    }

    if (statbuf->st_mode == 0) {
        if (file_info.BasicInformation.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            statbuf->st_mode |= _S_IFDIR;
            statbuf->st_size = 0;
        }
        else {
            statbuf->st_mode |= _S_IFREG;
            statbuf->st_size = file_info.StandardInformation.EndOfFile.QuadPart;
        }
    }

    if (file_info.BasicInformation.FileAttributes & FILE_ATTRIBUTE_READONLY)
        statbuf->st_mode |= _S_IREAD | (_S_IREAD >> 3) | (_S_IREAD >> 6);
    else
        statbuf->st_mode |= (_S_IREAD | _S_IWRITE) | ((_S_IREAD | _S_IWRITE) >> 3) |
        ((_S_IREAD | _S_IWRITE) >> 6);

    _ev_filetime_to_timespec(&statbuf->st_atim,
        file_info.BasicInformation.LastAccessTime.QuadPart);
    _ev_filetime_to_timespec(&statbuf->st_ctim,
        file_info.BasicInformation.ChangeTime.QuadPart);
    _ev_filetime_to_timespec(&statbuf->st_mtim,
        file_info.BasicInformation.LastWriteTime.QuadPart);
    _ev_filetime_to_timespec(&statbuf->st_birthtim,
        file_info.BasicInformation.CreationTime.QuadPart);

    statbuf->st_ino = file_info.InternalInformation.IndexNumber.QuadPart;

    /* st_blocks contains the on-disk allocation size in 512-byte units. */
    statbuf->st_blocks =
        (uint64_t)file_info.StandardInformation.AllocationSize.QuadPart >> 9;

    statbuf->st_nlink = file_info.StandardInformation.NumberOfLinks;

    /* The st_blksize is supposed to be the 'optimal' number of bytes for reading
     * and writing to the disk. That is, for any definition of 'optimal' - it's
     * supposed to at least avoid read-update-write behavior when writing to the
     * disk.
     *
     * However nobody knows this and even fewer people actually use this value,
     * and in order to fill it out we'd have to make another syscall to query the
     * volume for FILE_FS_SECTOR_SIZE_INFORMATION.
     *
     * Therefore we'll just report a sensible value that's quite commonly okay
     * on modern hardware.
     *
     * 4096 is the minimum required to be compatible with newer Advanced Format
     * drives (which have 4096 bytes per physical sector), and to be backwards
     * compatible with older drives (which have 512 bytes per physical sector).
     */
    statbuf->st_blksize = 4096;

    /* Todo: set st_flags to something meaningful. Also provide a wrapper for
     * chattr(2).
     */
    statbuf->st_flags = 0;

    /* Windows has nothing sensible to say about these values, so they'll just
     * remain empty.
     */
    statbuf->st_gid = 0;
    statbuf->st_uid = 0;
    statbuf->st_rdev = 0;
    statbuf->st_gen = 0;

    return 0;
}

static int _ev_file_fstat_win(HANDLE handle, ev_fs_stat_t* statbuf, int do_lstat)
{
    int errcode;
    if (_ev_file_wrap_fstat_win(handle, statbuf, do_lstat) != 0)
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    return EV_SUCCESS;
}

static ev_dirent_type_t _ev_fs_get_dirent_type_win(WIN32_FIND_DATAA* info)
{
    if (info->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        return EV_DIRENT_DIR;
    }
    if (info->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
    {
        return EV_DIRENT_LINK;
    }
    if (info->dwFileAttributes & FILE_ATTRIBUTE_DEVICE)
    {
        return EV_DIRENT_CHR;
    }
    return EV_DIRENT_FILE;
}

static int _ev_fs_wmkdir(WCHAR* path, int mode)
{
    (void)mode;

    wchar_t* p;
    wchar_t backup;
    DWORD errcode;
    const wchar_t* path_delim = L"\\/";

    for (p = wcspbrk(path, path_delim); p != NULL; p = wcspbrk(p + 1, path_delim))
    {
        backup = *p;
        *p = L'\0';
        if (!CreateDirectoryW(path, NULL))
        {
            errcode = GetLastError();
            if (errcode != ERROR_ALREADY_EXISTS)
            {
                *p = backup;
                return ev__translate_sys_error(errcode);
            }
        }
        *p = backup;
    }

    if (CreateDirectoryW(path, NULL))
    {
        return EV_SUCCESS;
    }

    errcode = GetLastError();
    if (errcode == ERROR_INVALID_NAME || errcode == ERROR_DIRECTORY)
    {
        return EV_EINVAL;
    }

    return ev__translate_sys_error(errcode);
}

int ev__fs_fstat(ev_os_file_t file, ev_fs_stat_t* statbuf)
{
    return _ev_file_fstat_win(file, statbuf, 0);
}

int ev__fs_close(ev_os_file_t file)
{
    DWORD errcode;
    if (!CloseHandle(file))
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }
    return EV_SUCCESS;
}

int ev__fs_readdir(const char* path, ev_fs_readdir_cb cb, void* arg)
{
    DWORD errcode;
    WIN32_FIND_DATAA info;
    ev_dirent_t dirent_info;

    HANDLE dir_handle = FindFirstFile(path, &info);
    if (dir_handle == INVALID_HANDLE_VALUE)
    {
        errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    int ret = 0;
    do 
    {
        if (strcmp(info.cFileName, ".") == 0 || strcmp(info.cFileName, "..") == 0)
        {
            continue;
        }

        dirent_info.name = info.cFileName;
        dirent_info.type = _ev_fs_get_dirent_type_win(&info);
        ret++;

        if (cb(&dirent_info, arg) != 0)
        {
            break;
        }
    } while (FindNextFile(dir_handle, &info));

    return ret;
}

int ev__fs_mkdir(const char* path, int mode)
{
    WCHAR* copy_wpath;
    ssize_t ret = _ev_fs_utf8_to_wide(&copy_wpath, path);
    if (ret < 0)
    {
        return (int)ret;
    }

    ret = _ev_fs_wmkdir(copy_wpath, mode);
    ev__free(copy_wpath);

    return (int)ret;
}
