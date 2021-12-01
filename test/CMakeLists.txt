cmake_minimum_required(VERSION 3.2)

add_executable(unittest
    "src/utils/random.c"
    "src/utils/sockpair.c"
    "src/cases/async.c"
    "src/cases/buf.c"
    "src/cases/ipv4_addr.c"
    "src/cases/once.c"
    "src/cases/pipe_data_mode.c"
    "src/cases/pipe_ipc_transfer_tcp_in_same_process.c"
    "src/cases/tcp.c"
    "src/cases/tcp_idle_client.c"
    "src/cases/tcp_listen.c"
    "src/cases/tcp_push_server.c"
    "src/cases/tcp_static_initializer.c"
    "src/cases/timer.c"
    "src/cases/timer_exit_in_callback.c"
    "src/cases/timer_stop_loop_in_callback.c"
    "src/test.c"
    "src/main.c")

target_include_directories(unittest
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(unittest PRIVATE /W4 /WX)
else ()
    target_compile_options(unittest PRIVATE -Wall -Wextra -Werror)
endif ()

target_link_libraries(unittest PRIVATE ev cutest)
add_test(unittest unittest)

add_custom_target(test_valgrind
    COMMAND valgrind
            --trace-children=yes
            --track-fds=all
            --leak-check=full
            --show-leak-kinds=all
            $<TARGET_FILE:unittest>)
