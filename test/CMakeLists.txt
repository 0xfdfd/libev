cmake_minimum_required(VERSION 3.2)

add_executable(ev_test
    "src/utils/hash.c"
    "src/utils/random.c"
    "src/utils/sockpair.c"
    "src/cases/async.c"
    "src/cases/buf.c"
    "src/cases/ipv4_addr.c"
    "src/cases/once.c"
    "src/cases/pipe_data_mode.c"
    "src/cases/pipe_ipc_mode_dgram.c"
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

target_include_directories(ev_test
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(ev_test PRIVATE /W4 /WX)
else ()
    target_compile_options(ev_test PRIVATE -Wall -Wextra -Werror)
endif ()

target_link_libraries(ev_test PRIVATE ev cutest)
add_test(ev_test ev_test)

add_custom_target(ev_test_valgrind
    COMMAND valgrind
            --trace-children=yes
            --track-fds=yes
            --leak-check=full
            --show-leak-kinds=all
            $<TARGET_FILE:ev_test>)

add_custom_target(ev_test_drmemory
    COMMAND drmemory
            -show_reachable
            -logdir $<TARGET_FILE_DIR:ev_test>
            $<TARGET_FILE:ev_test>)
