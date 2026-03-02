#include "network.hpp"
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <memory>
#include <sstream>
#include <format>

#include "file_listing.hpp"
#include "util.hpp"

#define SERVER_IP "192.168.8.128"
#define SERVER_PORT 12345

class Outside {
private:
	bool is_server = true;
	int client_socket = 0;
	sockaddr_in serverAddr;

	void init_non_local() {
		client_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (client_socket < 0) {
			std::cerr << "Socket creation failed\n";
			exit(1);
		}
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(SERVER_PORT);
		inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

		if (connect(client_socket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
			std::cerr << "Connection failed\n";
			exit(1);
		}
		std::cerr << "init done\n";
	}
public:
	Outside(bool is_server_) : is_server(is_server_) {
		if (is_server) {
			init_non_local();
		}
	}
	std::string get_input() const {
		if (this->is_server == false) {
			std::string s;
			getline(std::cin, s);
			return s;
		} else {
			return recv_message(client_socket);
		}
	}
	void send_output(const std::string& message) const {
		if (this->is_server == false) {
			std::cout << message << "\n";
		} else {
			send_message(client_socket, message);
		}
	}
	~Outside() {
		if (this->is_server) {
			CLOSESOCKET(client_socket);
		}
	}
};

int main(int argc, char* argv[]) {
	network_init();
	std::unique_ptr<Outside> o = std::make_unique<Outside>(true);
	std::string chosen_ip = SERVER_IP;
	int chosen_port = SERVER_PORT;
	for (int i = 1; i < argc; ++ i) {
		std::string arg = std::string(argv[i]);
		if (arg.starts_with("--server-ip=")) {
			std::string server_ip = arg.substr(std::string("--server-ip=").length());
			chosen_ip = server_ip;
		} else if (arg.starts_with("--server-port=")) {
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
	std::cout << "chosing " << chosen_ip << ":" << chosen_port << "\n";

	while (true) {
		std::string buffer = o->get_input();
		std::cout << "Server says: " << buffer << std::endl;
		if (buffer == "exit") {
			break;
		}
		if (buffer == "ls") {
			std::ostringstream oss(std::ios::binary);
			std::vector<FileListing> fl;
			for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
				fl.emplace_back(entry);
			}
			serialize_vector(oss, fl);
			o->send_output(oss.str());
			continue;
		}
		if (buffer.starts_with("cd ")) {
			std::string arg = buffer.substr(3);
			std::ostringstream oss(std::ios::binary);
			try {
				std::filesystem::current_path(arg);
				write_uint32(oss, 0);
			} catch (const std::filesystem::filesystem_error& e) {
				write_uint32(oss, 1);
			}
			std::cerr << oss.str() << "\n";
			o->send_output(oss.str());
			continue;
		}
		o->send_output(std::format("{}: not recognized", buffer.c_str()));
	}
	network_cleanup();
	return 0;
}