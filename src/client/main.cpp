#include "client.hpp"
#include "error.hpp"
#include "network.hpp"

int main(int argc, char *argv[]) {
	if (network_init() != 0) {
		print_error("network init failed");
		return 1;
	}
	int return_code = client_main(argc, argv);
	network_cleanup();
}