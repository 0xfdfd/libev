
int ev_dlopen(ev_shdlib_t* lib, const char* filename, char** errmsg)
{
    WCHAR* filename_w = NULL;
    ssize_t wide_sz = ev__utf8_to_wide(&filename_w, filename);
    if (wide_sz < 0)
    {
        return (int)wide_sz;
    }

    lib->handle = LoadLibraryExW(filename_w, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    ev_free(filename_w);

    if (lib->handle != EV_OS_SHDLIB_INVALID)
    {
        return 0;
    }

    DWORD errcode = GetLastError();
    if (errmsg == NULL)
    {
        goto finish;
    }

    DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    DWORD dwLanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

    char* tmp_errmsg = NULL;
    DWORD res = FormatMessageA(dwFlags, NULL, errcode, dwLanguageId, (LPSTR)&tmp_errmsg, 0, NULL);
    if (res == 0)
    {
        DWORD fmt_errcode = GetLastError();
        if (fmt_errcode == ERROR_MUI_FILE_NOT_FOUND || fmt_errcode == ERROR_RESOURCE_TYPE_NOT_FOUND)
        {
            res = FormatMessageA(dwFlags, NULL, errcode, 0, (LPSTR)&tmp_errmsg, 0, NULL);
        }
        if (res == 0)
        {
            *errmsg = NULL;
            goto finish;
        }
    }

    *errmsg = ev__strdup(tmp_errmsg);
    LocalFree(tmp_errmsg);

finish:
    return ev__translate_sys_error(errcode);
}

void ev_dlclose(ev_shdlib_t* lib)
{
    if (lib->handle != EV_OS_SHDLIB_INVALID)
    {
        FreeLibrary(lib->handle);
        lib->handle = EV_OS_SHDLIB_INVALID;
    }
}

int ev_dlsym(ev_shdlib_t* lib, const char* name, void** ptr)
{
    *ptr = (void*)(uintptr_t)GetProcAddress(lib->handle, name);
    if (*ptr == NULL)
    {
        DWORD errcode = GetLastError();
        return ev__translate_sys_error(errcode);
    }

    return 0;
}
