
#define _TOSTR(x)       #x
#define TOSTR(x)        _TOSTR(x)

#define EV_VERSION_STR   \
    TOSTR(EV_VERSION_MAJOR) "." TOSTR(EV_VERSION_MINOR) "." TOSTR(EV_VERSION_PATCH)

const char* ev_version_str(void)
{
    return EV_VERSION_STR;
}

unsigned ev_version_code(void)
{
    return EV_VERSION_CODE;
}
