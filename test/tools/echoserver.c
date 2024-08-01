#include "__init__.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#if defined(_WIN32)
#   include <windows.h>
#else
#   include <unistd.h>
#endif

static int tool_echoserver(int argc, char* argv[])
{
    (void)argc; (void)argv;

    char buffer[1024];

#if defined(_WIN32)
    BOOL bSuccess;
    DWORD errcode;
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdout == INVALID_HANDLE_VALUE || hStdin == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "invalid stdio handle.\n");
        return EXIT_SUCCESS;
    }

    for (;;)
    {
        DWORD dwread;
        bSuccess = ReadFile(hStdin, buffer, sizeof(buffer), &dwread, NULL);
        if (!bSuccess)
        {
            errcode = GetLastError();
            fprintf(stderr, "ReadFile failed: %ld.\n", (long)errcode);
            return EXIT_SUCCESS;
        }

        DWORD dwwrite;
        bSuccess = WriteFile(hStdout, buffer, dwread, &dwwrite, NULL);
        if (!bSuccess)
        {
            errcode = GetLastError();
            fprintf(stderr, "WriteFile failed: %ld.\n", (long)errcode);
            return EXIT_SUCCESS;
        }
    }
#else
    for (;;)
    {
        ssize_t read_size = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (read_size == 0)
        {
            fprintf(stderr, "stdin EOF.\n");
            return EXIT_SUCCESS;
        }
        if (read_size < 0)
        {
            fprintf(stderr, "stdin error: %s(%d).\n", strerror(errno), errno);
            return EXIT_SUCCESS;
        }

        ssize_t write_size = write(STDOUT_FILENO, buffer, read_size);
        if (write_size < 0)
        {
            fprintf(stderr, "stdout error: %s(%d).\n", strerror(errno), errno);
            return EXIT_SUCCESS;
        }
    }

#endif
}

const test_tool_t test_tool_echoserver = {
"echoserver", tool_echoserver,
"Start a stdio echo server.\n"
"The echo server will get everything from stdin and print to stdout."
};
