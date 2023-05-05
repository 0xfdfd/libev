
add_executable(ev_test
    "test/src/tools/echoserver.c"
    "test/src/tools/help.c"
    "test/src/tools/init.c"
    "test/src/tools/ls.c"
    "test/src/tools/pwd.c"
    "test/src/type/__init__.c"
    "test/src/type/ev_loop_t.c"
    "test/src/type/ssize_t.c"
    "test/src/utils/config.c"
    "test/src/utils/file.c"
    "test/src/utils/hash.c"
    "test/src/utils/memcheck.c"
    "test/src/utils/random.c"
    "test/src/utils/sockpair.c"
    "test/src/cases/async.c"
    "test/src/cases/buf.c"
    "test/src/cases/fs.c"
    "test/src/cases/ipv4_addr.c"
    "test/src/cases/list.c"
    "test/src/cases/loop_empty.c"
    "test/src/cases/mutex.c"
    "test/src/cases/once.c"
    "test/src/cases/pipe_close.c"
    "test/src/cases/pipe_data_mode.c"
    "test/src/cases/pipe_ipc_mode_dgram.c"
    "test/src/cases/pipe_ipc_mode_tcp_handle.c"
    "test/src/cases/pipe_make_block.c"
    "test/src/cases/process.c"
    "test/src/cases/process.redirect_file.c"
    "test/src/cases/queue.c"
    "test/src/cases/shm.c"
    "test/src/cases/tcp_close_in_middle.c"
    "test/src/cases/tcp_connect_non_exist.c"
    "test/src/cases/tcp_idle_client.c"
    "test/src/cases/tcp_listen.c"
    "test/src/cases/tcp_push_server.c"
    "test/src/cases/tcp_static_initializer.c"
    "test/src/cases/threadpool.c"
    "test/src/cases/timer_exit_in_callback.c"
    "test/src/cases/timer_normal.c"
    "test/src/cases/timer_stop_loop_in_callback.c"
    "test/src/cases/udp_bind.c"
    "test/src/cases/udp_connect.c"
    "test/src/cases/udp_multicast_interface.c"
    "test/src/cases/udp_ttl.c"
    "test/src/cases/version.c"
    "test/src/test.c"
    "test/src/main.c")

target_include_directories(ev_test
    PUBLIC
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/src)

ev_setup_target_wall(ev_test)

target_link_libraries(ev_test PRIVATE ev cutest)
add_test(NAME ev_test COMMAND $<TARGET_FILE:ev_test>)

if(EV_HAVE_COVERAGE)
    setup_target_for_coverage_lcov(
        NAME coverage
        EXECUTABLE $<TARGET_FILE:ev_test>
    )
endif()

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
