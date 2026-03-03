#include "error.hpp"
#include "network.hpp"
#include "server.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
	if (network_init() != 0) {
		print_error("network init failed");
		return 1;
	}
	StdCinInput input;
	RealCommandProcessor real_command_processor;
	int return_code = server_main(argc, argv, std::ref(input), std::ref(real_command_processor));
	network_cleanup();
	return return_code;
}