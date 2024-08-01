#include "__init__.h"
#include "test.h"

static int tool_pwd(int argc, char* argv[])
{
    (void)argc; (void)argv;

    ssize_t path_size = ev_getcwd(NULL, 0) + 1;
    char* buffer = mmc_malloc(path_size);
    if (buffer == NULL)
    {
        fprintf(stderr, "Out of memory.\n");
        return EXIT_FAILURE;
    }

    ev_getcwd(buffer, path_size);
    fprintf(stdout, "%s\n", buffer);

    mmc_free(buffer);

    return EXIT_SUCCESS;
}

const test_tool_t test_tool_pwd = {
"pwd", tool_pwd,
"Print the name of the current working directory."
};
