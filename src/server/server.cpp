#include <cstring>
#include <iostream>
#include <unistd.h>

#include "file_listing.hpp"
#include "network.hpp"
#include "process_listing.hpp"
#include "util.hpp"

#define PORT 12345

int main(int argc, char *argv[]) {
	int chosen_port = PORT;
	for (int i = 1; i < argc; ++i) {
		std::string arg = std::string(argv[i]);
		if (arg.starts_with("--server-port=")) {
			std::string server_port = arg.substr(std::string("--server-port=").length());
			int port = str_to_int(server_port);
			if (port < 0) {
				std::cerr << "--server-port: error in converting " << server_port << " to int\n";
			} else if (port == 0 || port > 65536) {
				std::cerr << "--server-port: invalid port " << port << "\n";
			} else {
				chosen_port = port;
			}
		}
	}

	if (network_init() != 0) {
		std::cout << "Network init failed\n";
		return 1;
	}
	socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		network_perror("create socket failed");
		return 1;
	}

	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(12345);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	int opt = 1;
#ifdef _WIN32
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)(&opt), sizeof(opt));
#else
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
	if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		network_perror("bind failed");
		return 1;
	}

	listen(server_socket, 1);
	std::cout << "Server listening on port " << PORT << "...\n";

	sockaddr_in clientAddr{};
	socklen_t clientSize = sizeof(clientAddr);
	int client_socket = accept(server_socket, (sockaddr *)&clientAddr, &clientSize);

	char client_address_ip[18];
	inet_ntop(AF_INET, &clientAddr.sin_addr.s_addr, client_address_ip, 18);
	std::cout << "Client connected!\n";
	std::cout << client_address_ip << " " << clientAddr.sin_port << "\n";

	while (true) {
		std::string s;
		getline(std::cin, s);
		std::cout << "we say: " << s << "\n";
		send_message(client_socket, s);
		if (s == "exit") {
			break;
		}
		if (s == "ls") {
			std::string payload = recv_message(client_socket);
			std::istringstream iss(payload, std::ios::binary);
			std::vector<FileListing> fl = read_vector_serializeable<FileListing>(iss);
			for (const auto &f : fl) {
				std::string type = "[OTHER]";
				if (f.is_directory()) {
					type = "[DIR]  ";
				}
				if (f.is_regular_file()) {
					type = "[FILE] ";
				}
				std::cout << type << " " << f.get_file_path() << "\n";
			}
			continue;
		}
		if (s.starts_with("cd ")) {
			std::string payload = recv_message(client_socket);
			std::istringstream iss(payload, std::ios::binary);
			int return_code = read_uint32(iss);
			std::cout << "Client returns: " << return_code << "\n";
			continue;
		}
		if (s.starts_with("ps")) {
			std::string payload = recv_message(client_socket);
			std::istringstream iss(payload, std::ios::binary);
			std::vector<ProcessListing> processes = read_vector_serializeable<ProcessListing>(iss);
			for (const auto &proc : processes) {
				std::cout << proc.get_pid() << " " << proc.get_proc_name() << "\n";
				for (const auto &arg : proc.get_args()) {
					std::cout << arg << " ";
				}
				std::cout << "\n";
			}
			continue;
		}
		std::string message = recv_message(client_socket);
		std::cout << "Client says: " << message << std::endl;
	}

	CLOSESOCKET(client_socket);
	CLOSESOCKET(server_socket);

	network_cleanup();
	return 0;
}