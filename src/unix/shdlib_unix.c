#include "ev.h"
#include "allocator.h"
#include "misc.h"
#include <dlfcn.h>

int ev_dlopen(ev_shdlib_t* lib, const char* filename, char** errmsg)
{
    /* Reset error status. */
    dlerror();

    if ((lib->handle = dlopen(filename, RTLD_LAZY)) != NULL)
    {
        return 0;
    }

    const char* dlerrmsg = dlerror();
    if (dlerrmsg == NULL)
    {
        return 0;
    }

    if (errmsg != NULL)
    {
        *errmsg = ev__strdup(dlerrmsg);
    }

    return EV_EINVAL;
}

void ev_dlclose(ev_shdlib_t* lib)
{
    if (lib->handle != EV_OS_SHDLIB_INVALID)
    {
        dlclose(lib->handle);
        lib->handle = EV_OS_SHDLIB_INVALID;
    }
}

int ev_dlsym(ev_shdlib_t* lib, const char* name, void** ptr)
{
    /* Reset error status. */
    dlerror();

    /* Resolve symbol. */
    if ((*ptr = dlsym(lib->handle, name)) != NULL)
    {
        return 0;
    }

    /* Check for error message. */
    const char* errmsg = dlerror();
    if (errmsg == NULL)
    {
        return 0;
    }

    return EV_ENOENT;
}
