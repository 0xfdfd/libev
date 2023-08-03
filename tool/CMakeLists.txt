
add_executable(amalgamate
    "tool/amalgamate.c"
    "tool/function.c"
    "tool/sha256.c")

target_link_libraries(amalgamate
    PRIVATE
        liblua54)

ev_setup_target_wall(amalgamate)
