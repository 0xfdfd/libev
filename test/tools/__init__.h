#ifndef __TEST_TOOL_INIT_H__
#define __TEST_TOOL_INIT_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef struct test_tool
{
    /**
     * @brief Command
     */
    const char* cmd;

    /**
     * @brief Command entrypoint.
     */
    int (*entrypoint)(int argc, char* argv[]);

    /**
     * @brief Help string
     */
    const char* help;
} test_tool_t;

extern const test_tool_t test_tool_echoserver;
extern const test_tool_t test_tool_eolcheck;
extern const test_tool_t test_tool_help;
extern const test_tool_t test_tool_ls;
extern const test_tool_t test_tool_pwd;
extern const test_tool_t test_tool_tabcheck;

int tool_exec(int argc, char* argv[]);

void tool_foreach(int (*cb)(const test_tool_t*, void*), void* arg);

#ifdef __cplusplus
}
#endif
#endif
