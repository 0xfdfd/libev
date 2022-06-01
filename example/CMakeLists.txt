
function(ev_example_setup_target name file)
    add_executable(${name} ${file})
    ev_setup_target_wall(${name})
    target_link_libraries(${name} PRIVATE ev)
endfunction()

ev_example_setup_target(tcp_echo_server tcp_echo_server.c)
