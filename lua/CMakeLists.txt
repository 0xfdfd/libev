###############################################################################
# Dependency
###############################################################################

# Lua
find_package(Lua REQUIRED)

###############################################################################
# Setup library
###############################################################################

target_compile_options(${PROJECT_NAME} PRIVATE -DEV_HAVE_LUA_BINDING)

target_include_directories(ev
    PRIVATE
        ${LUA_INCLUDE_DIR})

target_link_libraries(ev
    PRIVATE
        ${LUA_LIBRARIES})

###############################################################################
# Setup test
###############################################################################

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    target_sources(ev_test PRIVATE
        "lua/test/file.c"
        "lua/test/channel.c"
        "lua/test/loop.c"
        "lua/test/misc.c"
        "lua/test/process.c"
        "lua/test/promise.c"
        "lua/test/strerror.c"
        "lua/test/tcp.c"
        "lua/test/test.lua.c"
        "lua/test/timer.c"
        "lua/test/udp.c")
    target_include_directories(ev_test
        PRIVATE
            ${LUA_INCLUDE_DIR})
    target_link_libraries(ev_test
        PRIVATE
            ${LUA_LIBRARIES})
endif ()
