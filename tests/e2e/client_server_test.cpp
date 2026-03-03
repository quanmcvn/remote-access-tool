#include <gtest/gtest.h>

#include "client.hpp"
#include "server.hpp"

#include <future>

std::mutex test_mutex;

int run_server_main(GetInput& input_stream, CommandProcessor& command_processor) {
	const char *server_name = "./server";
	char *server_name_char = const_cast<char *>(server_name);
	return server_main(1, &server_name_char, std::ref(input_stream), std::ref(command_processor));
}

int run_client_main() { return client_main(0, nullptr); }

TEST(ClientServerTest, test) {
	FakeInput fake_input;
	RealCommandProcessor command_processor;

	std::future<int> server_return = std::async(std::launch::async, run_server_main,
	                                            std::ref(fake_input), std::ref(command_processor));

	std::future<int> client_1_return = std::async(std::launch::async, run_client_main);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	fake_input.add_input("1:ls\n1:pwd\n1:cd dir1\n1:ls\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	fake_input.add_input("1:exit\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	fake_input.add_input("exit\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	EXPECT_EQ(server_return.get(), 0);
	EXPECT_EQ(client_1_return.get(), 0);
}