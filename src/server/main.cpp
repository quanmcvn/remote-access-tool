#include "error.hpp"
#include "network.hpp"
#include "server.hpp"

int main(int argc, char *argv[]) {
	if (network_init() != 0) {
		print_error("network init failed");
		return 1;
	}
	int return_code = server_main(argc, argv);
	network_cleanup();
	return return_code;
}