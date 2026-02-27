#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <memory>
#include <sstream>
#include <format>

#include "file_listing.hpp"

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
			close(client_socket);
		}
	}
};

int main(int argc, char* argv[]) {
	std::unique_ptr<Outside> o;
	if (argc > 1) {
		if (std::string(argv[1]) == "local") {
			o = std::make_unique<Outside>(false);
		}
	}
	if (o.get() == nullptr) {
		o = std::make_unique<Outside>(true);
	}

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
		o->send_output(std::format("{}: not recognized", buffer.c_str()));
	}

	return 0;
}