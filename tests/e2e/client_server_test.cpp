#include <gtest/gtest.h>

#include "client.hpp"
#include "server.hpp"
#include "network.hpp"

#include <format>
#include <future>

bool is_port_available(int port) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		std::cerr << "Error opening socket\n";
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	int result = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	network_close_socket(sockfd);

	return result == 0;
}

class Args {
private:
	int argc;
	char **argv;

public:
	Args(const std::vector<std::string> &vs) {
		argc = vs.size();
		argv = new char *[argc];
		for (int i = 0; i < argc; ++i) {
			argv[i] = new char[vs[i].size() + 1];
			for (int j = 0; j < vs[i].size(); ++j) {
				argv[i][j] = vs[i][j];
			}
			argv[i][vs[i].size()] = '\0';
		}
	}
	int get_argc() const { return argc; }
	char **get_argv() const { return argv; }
	~Args() {
		for (int i = 0; i < argc; ++i) {
			delete[] argv[i];
		}
		delete[] argv;
	}
};

int run_server_main(int port, GetInput &input_stream, CommandProcessor &command_processor) {
	std::vector<std::string> vs;
	vs.push_back("./server");
	vs.push_back(std::format("--server-port={}", port));
	Args args(vs);
	return server_main(args.get_argc(), args.get_argv(), std::ref(input_stream),
	                   std::ref(command_processor));
}

int run_client_main(int port) {
	std::vector<std::string> vs;
	vs.push_back("./client");
	vs.push_back(std::format("--server-port={}", port));
	Args args(vs);
	return client_main(args.get_argc(), args.get_argv());
}

TEST(ClientServerTest, test_2_client) {
	FakeInput fake_input;
	RealCommandProcessor command_processor;
	int port = -1;
	for (int i = 12345; i <= 13000; ++i) {
		if (is_port_available(i)) {
			port = i;
			break;
		}
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::future<int> server_return = std::async(std::launch::async, run_server_main, port,
	                                            std::ref(fake_input), std::ref(command_processor));

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::future<int> client_1_return = std::async(std::launch::async, run_client_main, port);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::future<int> client_2_return = std::async(std::launch::async, run_client_main, port);

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	fake_input.add_input("1:ls\n1:pwd\n1:cd dir1\n1:ls\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	fake_input.add_input("2:ls\n2:pwd\n2:cd ..\n2:ls\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	fake_input.add_input("2:exit\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	fake_input.add_input("1:exit\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	fake_input.add_input("exit\n");

	EXPECT_EQ(server_return.get(), 0);
	EXPECT_EQ(client_1_return.get(), 0);
	EXPECT_EQ(client_2_return.get(), 0);
}

// TEST(ClientServerTest, test) {
// 	FakeInput fake_input;
// 	RealCommandProcessor command_processor;
// 	int port = -1;
// 	for (int i = 12345; i <= 13000; ++ i) {
// 		if (is_port_available(i)) {
// 			port = i;
// 			break;
// 		}
// 	}
// 	std::future<int> server_return = std::async(std::launch::async, run_server_main, port,
// 		std::ref(fake_input), std::ref(command_processor));

// 	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 	std::future<int> client_1_return = std::async(std::launch::async, run_client_main, port);

// 	std::this_thread::sleep_for(std::chrono::milliseconds(100));
// 	fake_input.add_input("1:ls\n1:pwd\n1:cd dir1\n1:ls\n");
// 	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 	fake_input.add_input("1:exit\n");
// 	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 	fake_input.add_input("exit\n");
// 	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

// 	EXPECT_EQ(server_return.get(), 0);
// 	EXPECT_EQ(client_1_return.get(), 0);
// }