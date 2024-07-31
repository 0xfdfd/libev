#include "test.h"

#ifdef _WIN32
#define LIB_SUFFIXES    ".dll"
#else
#define LIB_SUFFIXES    ".so"
#endif

#define SHD_LIB_PATH    "./ev_test_lib" LIB_SUFFIXES

TEST_FIXTURE_SETUP(shdlib)
{
}

TEST_FIXTURE_TEARDOWN(shdlib)
{
}

TEST_EXPOSE_API int test_shdlib_export_add(int a, int b)
{
    return a + b;
}

TEST_F(shdlib, open)
{
    ev_shdlib_t lib = EV_SHDLIB_INVALID;
    char* errmsg = NULL;
    ASSERT_EQ_INT(ev_dlopen(&lib, SHD_LIB_PATH, &errmsg), 0,
        "%s", errmsg);

    void* addr = NULL;
    ASSERT_EQ_INT(ev_dlsym(&lib, "test_shdlib_export_add", &addr), 0);
    ASSERT_EQ_PTR(addr, (void*)test_shdlib_export_add);

    ev_dlclose(&lib);
}
