add_executable(amalgamate
    "utils/file.cpp"
    "utils/sha256.c"
    "utils/sha256.cpp"
    "utils/string.cpp"
    "main.cpp"
)
ev_setup_target_wall(amalgamate)
