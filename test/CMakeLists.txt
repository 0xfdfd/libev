cmake_minimum_required(VERSION 3.2)
project(ev_test)

function(ev_add_test name)
	add_executable(${name} ${name}.c)
	target_link_libraries(${name} PRIVATE ev)
	add_test(${name} ${name})
endfunction()

ev_add_test(test-once)
