const char* ev_strerror(int err)
{
#define EV_EXPAND_ERRMAP(err, syserr, str) case err: return str;

    switch (err)
    {
    /* Success */
    case 0:                     return "Operation success";
    case EV_EOF:                return "End of file";
    /* posix */
    EV_ERRNO_POSIX_MAP(EV_EXPAND_ERRMAP);
    /* Unknown error */
    default:                    break;
    }
    return "Unknown error";

#undef EV_EXPAND_ERRMAP
}
