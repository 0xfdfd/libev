#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h>    /* For O_* constants */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int s_ev_shm_init(ev_shmem_t *shm, const char *key, size_t size)
{
    int err;
    shm->size = size;

    int ret = snprintf(shm->backend.name, sizeof(shm->backend.name), "%s", key);
    if (ret >= (int)sizeof(shm->backend.name))
    {
        return EV_ENOMEM;
    }
    memset(&shm->backend.mask, 0, sizeof(shm->backend.mask));

    shm->backend.map_file =
        shm_open(key, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (shm->backend.map_file == -1)
    {
        err = errno;
        goto err_shm_open;
    }

    if (ftruncate(shm->backend.map_file, size) != 0)
    {
        err = errno;
        goto err_ftruncate;
    }

    shm->addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                     shm->backend.map_file, 0);
    if (shm->addr == NULL)
    {
        err = errno;
        goto err_ftruncate;
    }

    return 0;

err_ftruncate:
    close(shm->backend.map_file);
err_shm_open:
    return ev__translate_sys_error(err);
}

static int s_ev_shm_open(ev_shmem_t *shm, const char *key)
{
    int err;
    int ret = snprintf(shm->backend.name, sizeof(shm->backend.name), "%s", key);
    if (ret >= (int)sizeof(shm->backend.name))
    {
        return EV_ENOMEM;
    }
    memset(&shm->backend.mask, 0, sizeof(shm->backend.mask));

    shm->backend.mask.is_open = 1;
    shm->backend.map_file = shm_open(key, O_RDWR, 0);
    if (shm->backend.map_file == -1)
    {
        err = errno;
        goto err_shm_open;
    }

    struct stat statbuf;
    if (fstat(shm->backend.map_file, &statbuf) != 0)
    {
        err = errno;
        goto err_fstat;
    }
    shm->size = statbuf.st_size;

    shm->addr = mmap(NULL, shm->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                     shm->backend.map_file, 0);
    if (shm->addr == NULL)
    {
        err = errno;
        goto err_fstat;
    }

    return 0;

err_fstat:
    close(shm->backend.map_file);
err_shm_open:
    return ev__translate_sys_error(err);
}

int ev_shmem_init(ev_shmem_t **shm, const char *key, size_t size)
{
    ev_shmem_t *handle = ev_malloc(sizeof(ev_shmem_t));
    if (handle == NULL)
    {
        return EV_ENOMEM;
    }

    int ret = s_ev_shm_init(handle, key, size);
    if (ret != 0)
    {
        ev_free(handle);
        return ret;
    }

    *shm = handle;
    return 0;
}

int ev_shmem_open(ev_shmem_t **shm, const char *key)
{
    ev_shmem_t *handle = ev_malloc(sizeof(ev_shmem_t));
    if (handle == NULL)
    {
        return EV_ENOMEM;
    }

    int ret = s_ev_shm_open(handle, key);
    if (ret != 0)
    {
        ev_free(handle);
        return ret;
    }

    *shm = handle;
    return 0;
}

void ev_shmem_exit(ev_shmem_t *shm)
{
    if (!shm->backend.mask.is_open)
    {
        shm_unlink(shm->backend.name);
    }

    int ret = munmap(shm->addr, shm->size);
    assert(ret == 0);
    (void)ret;

    close(shm->backend.map_file);
    ev_free(shm);
}
