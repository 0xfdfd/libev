
add_executable(ev_test
    "test/tools/echoserver.c"
    "test/tools/help.c"
    "test/tools/init.c"
    "test/tools/ls.c"
    "test/tools/pwd.c"
    "test/type/__init__.c"
    "test/type/ev_loop_t.c"
    "test/type/ssize_t.c"
    "test/utils/config.c"
    "test/utils/file.c"
    "test/utils/hash.c"
    "test/utils/memcheck.c"
    "test/utils/random.c"
    "test/utils/sockpair.c"
    "test/test.c"
    "test/main.c"
    "test/cases/c/async.c"
    "test/cases/c/buf.c"
    "test/cases/c/fs.c"
    "test/cases/c/ipv4_addr.c"
    "test/cases/c/list.c"
    "test/cases/c/loop_empty.c"
    "test/cases/c/mutex.c"
    "test/cases/c/once.c"
    "test/cases/c/pipe_close.c"
    "test/cases/c/pipe_data_mode.c"
    "test/cases/c/pipe_ipc_mode_dgram.c"
    "test/cases/c/pipe_ipc_mode_tcp_handle.c"
    "test/cases/c/pipe_make_block.c"
    "test/cases/c/process.c"
    "test/cases/c/process.redirect_file.c"
    "test/cases/c/queue.c"
    "test/cases/c/shm.c"
    "test/cases/c/tcp_close_in_middle.c"
    "test/cases/c/tcp_connect_non_exist.c"
    "test/cases/c/tcp_idle_client.c"
    "test/cases/c/tcp_listen.c"
    "test/cases/c/tcp_push_server.c"
    "test/cases/c/tcp_static_initializer.c"
    "test/cases/c/threadpool.c"
    "test/cases/c/timer_exit_in_callback.c"
    "test/cases/c/timer_normal.c"
    "test/cases/c/timer_stop_loop_in_callback.c"
    "test/cases/c/udp_bind.c"
    "test/cases/c/udp_connect.c"
    "test/cases/c/udp_multicast_interface.c"
    "test/cases/c/udp_ttl.c"
    "test/cases/c/version.c"
)

if (EV_HAVE_LUA_BINDING)
    target_sources(ev_test PRIVATE
        "test/cases/lua/file.c"
        "test/cases/lua/channel.c"
        "test/cases/lua/loop.c"
        "test/cases/lua/misc.c"
        "test/cases/lua/process.c"
        "test/cases/lua/promise.c"
        "test/cases/lua/strerror.c"
        "test/cases/lua/tcp.c"
        "test/cases/lua/test.lua.c"
        "test/cases/lua/timer.c"
        "test/cases/lua/udp.c"
    )
    target_include_directories(ev_test
        PRIVATE
            ${LUA_INCLUDE_DIR}
    )
    target_link_libraries(ev_test
        PRIVATE
            ${LUA_LIBRARIES}
    )
endif ()

target_include_directories(ev_test
    PUBLIC
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test
)

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
            $<TARGET_FILE:ev_test>
)

add_custom_target(ev_test_drmemory
    COMMAND drmemory
            -show_reachable
            -logdir $<TARGET_FILE_DIR:ev_test>
            $<TARGET_FILE:ev_test>
)
