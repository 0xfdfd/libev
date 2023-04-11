###############################################################################
# Dependency
###############################################################################

# Lua
find_package(Lua REQUIRED)

###############################################################################
# Setup library
###############################################################################

target_sources(ev PRIVATE
    "lua/src/ev.lua.c"
    "lua/src/fs.lua.c"
    "lua/src/misc.lua.c"
    "lua/src/pipe.lua.c"
    "lua/src/process.lua.c"
    "lua/src/tcp.lua.c"
    "lua/src/timer.lua.c"
    "lua/src/udp.lua.c")

target_include_directories(ev
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lua/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lua/src
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
        "lua/test/loop.c"
        "lua/test/process.c"
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
