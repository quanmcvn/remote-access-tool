add_executable(integration_tests
	client_server_test.cpp
)
target_link_libraries(integration_tests
	PRIVATE
		client_lib
		common
		server_lib
		GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(integration_tests)