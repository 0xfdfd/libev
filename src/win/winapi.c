#include "ev.h"
#include "winapi.h"
#include <assert.h>

fn_NtQueryInformationFile NtQueryInformationFile = NULL;

static void _ev_winapi_init_once(void)
{
#define GET_NTDLL_FUNC(name)  \
    do {\
        name = (fn_##name)GetProcAddress(ntdll_modeule, #name);\
        assert(name != NULL);\
    } while (0)

    HMODULE ntdll_modeule = GetModuleHandle("ntdll.dll");
    assert(ntdll_modeule != NULL);

    GET_NTDLL_FUNC(NtQueryInformationFile);

#undef GET_NTDLL_FUNC
}

void ev__winapi_init(void)
{
    static ev_once_t s_once = EV_ONCE_INIT;
    ev_once_execute(&s_once, _ev_winapi_init_once);
}
