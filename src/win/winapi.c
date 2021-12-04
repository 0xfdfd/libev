#include "ev.h"
#include "winapi.h"
#include "loop.h"
#include <assert.h>

void ev__winapi_init(void)
{
#define GET_NTDLL_FUNC(name)  \
    do {\
        g_ev_loop_win_ctx.name = (fn_##name)GetProcAddress(ntdll_modeule, #name);\
        assert(g_ev_loop_win_ctx.name != NULL);\
    } while (0)

    HMODULE ntdll_modeule = GetModuleHandle("ntdll.dll");
    assert(ntdll_modeule != NULL);

    GET_NTDLL_FUNC(NtQueryInformationFile);
    GET_NTDLL_FUNC(RtlNtStatusToDosError);

#undef GET_NTDLL_FUNC
}
