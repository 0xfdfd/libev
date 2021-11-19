cmake_minimum_required(VERSION 3.2)

add_executable(unittest
    "utils/random.c"
    "test_async.c"
    "test_buf.c"
    "test_ipv4_addr.c"
    "test_once.c"
    "test_pipe.c"
    "test_tcp_listen.c"
    "test_tcp_push_server.c"
    "test_timer.c"
    "test_timer_exit_in_callback.c"
    "test_timer_stop_loop_in_callback.c"
    "main.c")
target_link_libraries(unittest PRIVATE ev cutest)
add_test(unittest unittest)
