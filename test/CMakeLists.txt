
add_library(ev_test_lib SHARED
    "test/cases/async.c"
    "test/cases/buf.c"
    "test/cases/fs.c"
    "test/cases/fs_mmap.c"
    "test/cases/fs_mmap_offset.c"
    "test/cases/fs_seek.c"
    "test/cases/ipv4_addr.c"
    "test/cases/list.c"
    "test/cases/misc_page_size.c"
    "test/cases/misc_random.c"
    "test/cases/mutex.c"
    "test/cases/once.c"
    "test/cases/pipe_close.c"
    "test/cases/pipe_data_mode.c"
    "test/cases/pipe_ipc_mode_dgram.c"
    "test/cases/pipe_ipc_mode_tcp_handle.c"
    "test/cases/pipe_make_block.c"
    "test/cases/process.c"
    "test/cases/process.redirect_file.c"
    "test/cases/queue.c"
    "test/cases/shdlib.c"
    "test/cases/shm.c"
    "test/cases/tcp_close_in_middle.c"
    "test/cases/tcp_connect_non_exist.c"
    "test/cases/tcp_idle_client.c"
    "test/cases/tcp_listen.c"
    "test/cases/tcp_push_server.c"
    "test/cases/tcp_static_initializer.c"
    "test/cases/threadpool.c"
    "test/cases/timer_exit_in_callback.c"
    "test/cases/timer_normal.c"
    "test/cases/timer_stop_loop_in_callback.c"
    "test/cases/udp_bind.c"
    "test/cases/udp_connect.c"
    "test/cases/udp_multicast_interface.c"
    "test/cases/udp_ttl.c"
    "test/cases/version.c"
    "test/tools/echoserver.c"
    "test/tools/eolcheck.c"
    "test/tools/help.c"
    "test/tools/__init__.c"
    "test/tools/ls.c"
    "test/tools/pwd.c"
    "test/tools/tabcheck.c"
    "test/type/__init__.c"
    "test/type/ssize_t.c"
    "test/utils/config.c"
    "test/utils/file.c"
    "test/utils/hash.c"
    "test/utils/memcheck.c"
    "test/utils/random.c"
    "test/utils/sockpair.c"
    "test/utils/str.c"
    "test/test.c"
)

target_include_directories(ev_test_lib
    PUBLIC
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test
)

target_compile_options(ev_test_lib
    PRIVATE
        -DCUTEST_BUILDING_DLL
        -DTEST_BUILDING_DLL
)

target_link_libraries(ev_test_lib
    PUBLIC
        ev
        cutest
)

ev_setup_target_wall(ev_test_lib)

set_target_properties(ev_test_lib PROPERTIES
    PREFIX ""
)

add_executable(ev_test
    "test/main.c"
)

target_link_libraries(ev_test
    PRIVATE
        ev_test_lib
)

add_test(NAME ev_unittest COMMAND $<TARGET_FILE:ev_test>)
if (EV_DEV)
    add_test(NAME ev_eol_h
        COMMAND $<TARGET_FILE:ev_test> -- eolcheck --file=${CMAKE_CURRENT_SOURCE_DIR}/ev.h --eol=LF
    )
    add_test(NAME ev_eol_c
        COMMAND $<TARGET_FILE:ev_test> -- eolcheck --file=${CMAKE_CURRENT_SOURCE_DIR}/ev.c --eol=LF
    )
    add_test(NAME ev_tab_h
        COMMAND $<TARGET_FILE:ev_test> -- tabcheck --file=${CMAKE_CURRENT_SOURCE_DIR}/ev.h
    )
    add_test(NAME ev_tab_c
        COMMAND $<TARGET_FILE:ev_test> -- tabcheck --file=${CMAKE_CURRENT_SOURCE_DIR}/ev.c
    )
endif()

if(EV_HAVE_COVERAGE)
    setup_target_for_coverage_lcov(
        NAME coverage
        EXECUTABLE $<TARGET_FILE:ev_test>
    )
endif()

if (EV_ASAN)
    target_compile_options(ev_test_lib PRIVATE -fsanitize=address -DTEST_NO_MMC)
    target_compile_options(ev_test PRIVATE -fsanitize=address -DTEST_NO_MMC)
    target_link_options(ev_test_lib PRIVATE -fsanitize=address)
    target_link_options(ev_test PRIVATE -fsanitize=address)
endif()
