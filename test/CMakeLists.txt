cmake_minimum_required(VERSION 3.2)

add_executable(ev_test
    "src/utils/hash.c"
    "src/utils/random.c"
    "src/utils/sockpair.c"
    "src/cases/async.c"
    "src/cases/buf.c"
    "src/cases/ipv4_addr.c"
    "src/cases/loop_empty.c"
    "src/cases/mutex.c"
    "src/cases/once.c"
    "src/cases/pipe_data_mode.c"
    "src/cases/pipe_ipc_mode_dgram.c"
    "src/cases/pipe_ipc_mode_tcp_handle.c"
    "src/cases/shm.c"
    "src/cases/tcp_connect_non_exist.c"
    "src/cases/tcp_idle_client.c"
    "src/cases/tcp_listen.c"
    "src/cases/tcp_push_server.c"
    "src/cases/tcp_static_initializer.c"
    "src/cases/threadpool.c"
    "src/cases/timer_exit_in_callback.c"
    "src/cases/timer_normal.c"
    "src/cases/timer_stop_loop_in_callback.c"
    "src/cases/udp_bind.c"
    "src/cases/udp_connect.c"
    "src/cases/udp_multicast_interface.c"
    "src/cases/udp_ttl.c"
    "src/test.c"
    "src/main.c")

target_include_directories(ev_test
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(ev_test PRIVATE /W4 /WX)
else ()
    target_compile_options(ev_test PRIVATE -Wall -Wextra -Werror)
endif ()

function(EV_SETUP_EXPORT_PGO name)
    get_target_property(TEMP ${name} LINK_FLAGS)
    if(TEMP STREQUAL "TEMP-NOTFOUND")
        SET(TEMP "") # Set to empty string
    else()
        SET(TEMP "${TEMP} ") # A space to cleanly separate from existing content
    endif()
    # Append our values
    SET(TEMP "${TEMP}-fprofile-generate" )
    set_target_properties(${name} PROPERTIES LINK_FLAGS ${TEMP} )
endfunction()

if (EV_PGO_EXPORT)
    if (CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
        EV_SETUP_EXPORT_PGO(ev_test)
    elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    endif ()
endif ()

target_link_libraries(ev_test PRIVATE ev cutest)
add_test(ev_test ev_test)

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
