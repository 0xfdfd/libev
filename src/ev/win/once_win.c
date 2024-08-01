
static BOOL WINAPI _ev_once_proxy(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *Context)
{
    (void)InitOnce; (void)Context;

    ((ev_once_cb)Parameter)();
    return TRUE;
}

void ev_once_execute(ev_once_t* guard, ev_once_cb cb)
{
    DWORD errcode;
    if (InitOnceExecuteOnce(&guard->guard, _ev_once_proxy, (PVOID)cb, NULL) == 0)
    {
        errcode = GetLastError();
        EV_ABORT("GetLastError:%lu", (unsigned long)errcode);
    }
}
