cmake_minimum_required(VERSION 3.2)
project(ev_test)

function(EV_ADD_TEST name)
	add_executable(${name} ${name}.c)
	target_link_libraries(${name} PRIVATE ev)
	add_test(${name} ${name})
endfunction()

EV_ADD_TEST(test_async)
EV_ADD_TEST(test_timer_stop_loop)
EV_ADD_TEST(test_once)
EV_ADD_TEST(test_timer)
