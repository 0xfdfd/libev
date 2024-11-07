#include <dlfcn.h>

#if defined(__NetBSD__)

static int _ev_random_sysctl_netbsd(void* buf, size_t len)
{
    static int name[] = { CTL_KERN, KERN_ARND };
    size_t count, req;
    unsigned char* p;

    p = buf;
    while (len)
    {
        req = len < 32 ? len : 32;
        count = req;

        if (sysctl(name, ARRAY_SIZE(name), p, &count, NULL, 0) == -1)
        {
            return ev__translate_posix_sys_error(errno);
        }

        if (count != req)
        {
            return EV_EIO;  /* Can't happen. */
        }

        p += count;
        len -= count;
    }

    return 0;
}

EV_LOCAL int ev__random(void* buf, size_t len)
{
    return _ev_random_sysctl_netbsd(buf, len);
}

#elif defined(__FreeBSD__) || defined(__linux__)

typedef struct ev__sysctl_args
{
  int* name;
  int nlen;
  void* oldval;
  size_t* oldlenp;
  void* newval;
  size_t newlen;
  unsigned long unused[4];
} ev__sysctl_args_t;

/**
 * @brief getrandom() protocol type.
 */
typedef ssize_t (*ev__getrandom_fn)(void *, size_t, unsigned);

static ev__getrandom_fn ev__getrandom = NULL;

static void _ev_random_getrandom_init(void)
{
    ev__getrandom = (ev__getrandom_fn)dlsym(RTLD_DEFAULT, "getrandom");
}

static int _ev_random_getrandom(void* buf, size_t len)
{
    static ev_once_t token = EV_ONCE_INIT;
    ev_once_execute(&token, _ev_random_getrandom_init);

    if (ev__getrandom == NULL)
    {
        return EV_ENOSYS;
    }

    size_t pos; ssize_t n;
    for (pos = 0; pos < len; pos += n)
    {
        n = len - pos;
        if (n > 256)
        {
            n = 256;
        }

        do
        {
            n = ev__getrandom((char*)buf + pos, n, 0);
        } while (n == -1 && errno == EINTR);
        
        if (n < 0)
        {
            return ev__translate_posix_sys_error(errno);
        }
        else if (n == 0)
        {
            return EV_EIO;
        }
    }

    return 0;
}

static int _ev_random_sysctl_linux(void* buf, size_t len)
{
    static int name[] = {1 /*CTL_KERN*/, 40 /*KERN_RANDOM*/, 6 /*RANDOM_UUID*/};
    ev__sysctl_args_t args;
    char uuid[16];
    char* p;
    char* pe;
    size_t n;

    p = buf;
    pe = p + len;

    while (p < pe)
    {
        memset(&args, 0, sizeof(args));

        args.name = name;
        args.nlen = ARRAY_SIZE(name);
        args.oldval = uuid;
        args.oldlenp = &n;
        n = sizeof(uuid);

        /* Emits a deprecation warning with some kernels but that seems like
        * an okay trade-off for the fallback of the fallback: this function is
        * only called when neither getrandom(2) nor /dev/urandom are available.
        * Fails with ENOSYS on kernels configured without CONFIG_SYSCTL_SYSCALL.
        * At least arm64 never had a _sysctl system call and therefore doesn't
        * have a SYS__sysctl define either.
        */
    #ifdef SYS__sysctl
        if (syscall(SYS__sysctl, &args) == -1)
        {
            return UV__ERR(errno);
        }
    #else
        {
            (void) &args;
            return EV_ENOSYS;
        }
    #endif

        if (n != sizeof(uuid))
        {
            return EV_EIO;  /* Can't happen. */
        }

        /* uuid[] is now a type 4 UUID. Bytes 6 and 8 (counting from zero) contain
        * 4 and 5 bits of entropy, respectively. For ease of use, we skip those
        * and only use 14 of the 16 bytes.
        */
        uuid[6] = uuid[14];
        uuid[8] = uuid[15];

        n = pe - p;
        if (n > 14)
        {
            n = 14;
        }

        memcpy(p, uuid, n);
        p += n;
    }

    return 0;
}

EV_LOCAL int ev__random(void* buf, size_t len)
{
    int rc = _ev_random_getrandom(buf, len);
#if defined(__linux__)
    switch (rc)
    {
    case EV_EACCES:
    case EV_EIO:
    case EV_ELOOP:
    case EV_EMFILE:
    case EV_ENFILE:
    case EV_ENOENT:
    case EV_EPERM:
        rc = _ev_random_sysctl_linux(buf, len);
        break;
    }
#endif
    return rc;
}

#else

static int _ev_open_cloexec(const char* path, int flags)
{
#if defined(O_CLOEXEC)
    int fd = open(path, flags | O_CLOEXEC);
    if (fd < 0)
    {
        int errcode = errno;
        return ev__translate_posix_sys_error(errcode);
    }
    return fd;
#else
    int fd = open(path, flags);
    if (fd < 0)
    {
        int errcode = errno;
        return ev__translate_posix_sys_error(errcode);
    }
    int err = ev__cloexec(fd, 1);
    if (err != 0)
    {
        close(fd);
        return err;
    }
    return fd;
#endif
}

static int _ev_random_readpath(const char* path, void* buf, size_t len)
{
    int fd = _ev_open_cloexec(path, O_RDONLY);
    if (fd < 0)
    {
        return fd;
    }

    size_t pos;
    ssize_t read_sz;
    for (pos = 0; pos < len; pos += read_sz)
    {
        do
        {
            read_sz = read(fd, (char*)buf + pos, len - pos);
        } while (read_sz == -1 && errno == EINTR);

        if (read_sz < 0)
        {
            int errcode = errno;
            close(fd);
            return ev__translate_posix_sys_error(errcode);
        }

        if (read_sz == 0)
        {
            close(fd);
            return EV_EIO;
        }
    }

    close(fd);
    return 0;
}

#if defined(__PASE__)

EV_LOCAL int ev__random(void* buf, size_t len)
{
    return _ev_random_readpath("/dev/urandom", buf, len);
}

#elif defined(_AIX) || defined(__QNX__)

EV_LOCAL int ev__random(void* buf, size_t len)
{
    return _ev_random_readpath("/dev/random", buf, len);
}

#else

static int _ev_random_dev_urandom_status = 0;

static void _ev_random_dev_urandom_init(void)
{
    char c;

    /*
     * Linux's random(4) man page suggests applications should read at least
     * once from /dev/random before switching to /dev/urandom in order to seed
     * the system RNG. Reads from /dev/random can of course block indefinitely
     * until entropy is available but that's the point.
     */
    _ev_random_dev_urandom_status = _ev_random_readpath("/dev/random", &c, 1);
}

static int _ev_random_dev_urandom(void* buf, size_t len)
{
    static ev_once_t token = EV_ONCE_INIT;
    ev_once_execute(&token, _ev_random_dev_urandom_init);

    if (_ev_random_dev_urandom_status != 0)
    {
        return _ev_random_dev_urandom_status;
    }

    return _ev_random_readpath("/dev/urandom", buf, len);
}

#if defined(__APPLE__) || defined(__OpenBSD__) || \
     (defined(__ANDROID_API__) && __ANDROID_API__ >= 28)

typedef int (*ev__getentropy_fn)(void*, size_t);

static ev__getentropy_fn ev__getentropy = NULL;

static void _ev_random_getentropy_init(void)
{
    ev__getentropy = (ev__getentropy_fn) dlsym(RTLD_DEFAULT, "getentropy");
}

static int _ev_random_getentropy(void* buf, size_t len)
{
    static ev_once_t token = EV_ONCE_INIT;
    ev_once_execute(&token, _ev_random_getentropy_init);

    if (ev__getentropy == NULL)
    {
        return EV_ENOSYS;
    }

    size_t pos, stride;
    for (pos = 0, stride = 256; pos + stride < len; pos += stride)
    {
        if (ev__getentropy((char*)buf + pos, len - pos))
        {
            return ev__translate_posix_sys_error(errno);
        }
    }
    if (ev__getentropy((char*)buf + pos, len - pos))
    {
        return ev__translate_posix_sys_error(errno);
    }

    return 0;
}

EV_LOCAL int ev__random(void* buf, size_t len)
{
    int rc = _ev_random_getentropy(buf, len);
    if (rc == EV_ENOSYS)
    {
        rc = _ev_random_dev_urandom(buf, len);
    }
    return rc;
}

#else

EV_LOCAL int ev__random(void* buf, size_t len)
{
    return _ev_random_dev_urandom(buf, len);
}

#endif

#endif

#endif
