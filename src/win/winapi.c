#include "ev.h"
#include "winapi.h"
#include <assert.h>

ev_winapi_t ev_winapi = {
    NULL,
    NULL,
    NULL,
};

void ev__winapi_init(void)
{
#define GET_NTDLL_FUNC(name)  \
    do {\
        ev_winapi.name = (fn_##name)GetProcAddress(ntdll_modeule, #name);\
        assert(ev_winapi.name != NULL);\
    } while (0)

    HMODULE ntdll_modeule = GetModuleHandle("ntdll.dll");
    assert(ntdll_modeule != NULL);

    GET_NTDLL_FUNC(NtQueryInformationFile);
    GET_NTDLL_FUNC(RtlNtStatusToDosError);
    GET_NTDLL_FUNC(NtQueryVolumeInformationFile);

#undef GET_NTDLL_FUNC
}
