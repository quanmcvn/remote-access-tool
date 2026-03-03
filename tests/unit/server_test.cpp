#include <gtest/gtest.h>

#include "server.hpp"

namespace {

class FakeCommandProcessor : public CommandProcessor {
public:
	std::vector<std::string> commands;
	int process(const std::string &command) override {
		commands.push_back(command);
		if (command == "exit") {
			return 2;
		}
		return 0;
	}
};

} // namespace

TEST(ServerTest, server_input_thread) {
	std::istringstream fake_input("1:ls\n"
	                              "2:cd dir1\n"
	                              "2:pwd\n"
	                              "1:exit\n"
	                              "exit\n"
	                              "1:ls");

	FakeCommandProcessor command_processor;

	std::thread t([&]() {
		// close socket -1, unfortunately I don't have better way rn
		server_input_thread(fake_input, command_processor, -1);
	});

	t.join();

	// should be 5 because it exited before, also count exit
	EXPECT_EQ(command_processor.commands.size(), 5);
}