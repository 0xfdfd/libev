cmake_minimum_required(VERSION 3.2)

add_executable(unittest
    "src/utils/random.c"
    "src/cases/async.c"
    "src/cases/buf.c"
    "src/cases/ipv4_addr.c"
    "src/cases/once.c"
    "src/cases/pipe.c"
    "src/cases/tcp_listen.c"
    "src/cases/tcp_push_server.c"
    "src/cases/timer.c"
    "src/cases/timer_exit_in_callback.c"
    "src/cases/timer_stop_loop_in_callback.c"
    "src/main.c")

target_include_directories(unittest
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(unittest PRIVATE ev cutest)
add_test(unittest unittest)
