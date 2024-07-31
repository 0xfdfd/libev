#ifndef __EV_SHARED_LIBRARY_H__
#define __EV_SHARED_LIBRARY_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_SHAREDLIBRARY Shared library
 * @{
 */

/**
 * @brief Shared library handle.
 */
typedef struct ev_shdlib
{
    ev_os_shdlib_t  handle; /**< OS shared library handles */
} ev_shdlib_t;

/**
 * @brief Static initializer for #ev_shdlib_t.
 */
#define EV_SHDLIB_INVALID   { EV_OS_SHDLIB_INVALID }

/**
 * @brief Opens a shared library.
 * @param[out] lib - The opened library handle.
 * @param[in] filename - The name of the shared library. Encoding in UTF-8.
 * @param[out] errmsg - The error message if this function failed (the return
 *   value is non-zero). Use #ev_free() to release it.
 * @return #ev_errno_t
 */
int ev_dlopen(ev_shdlib_t* lib, const char* filename, char** errmsg);

/**
 * @brief Close the shared library.
 * @param[in] lib - The opened library handle.
 */
void ev_dlclose(ev_shdlib_t* lib);

/**
 * @brief Retrieves a data pointer from a dynamic library.
 * @note It is legal for a symbol to map to `NULL`.
 * @param[in] lib - The opened library handle.
 * @param[in] name - The name of the symbol.
 * @param[out] ptr - The address of the symbol.
 * @return #ev_errno_t
 */
int ev_dlsym(ev_shdlib_t* lib, const char* name, void** ptr);

/**
 * @} EV_SHAREDLIBRARY
 */

#ifdef __cplusplus
}
#endif
#endif
