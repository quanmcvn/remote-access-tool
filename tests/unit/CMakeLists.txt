add_executable(unit_tests
	client_test.cpp
	common_test.cpp
	server_test.cpp
)
target_link_libraries(unit_tests
	PRIVATE
		client_lib
		common
		server_lib
		GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)