#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <memory>

#define SERVER_IP "192.168.8.128"
#define SERVER_PORT 12345

class Outside {
private:
	bool is_server = true;
	int client_socket = 0;

	void init_non_local() {
		client_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (client_socket < 0) {
			std::cerr << "Socket creation failed\n";
			exit(1);
		}
		sockaddr_in serverAddr{};
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(SERVER_PORT);
		inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

		if (connect(client_socket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
			std::cerr << "Connection failed\n";
			exit(1);
		}
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
			char buffer[1024] = {0};
			recv(client_socket, buffer, 1024, 0);
			return std::string(buffer);
		}
	}
	void send_output(const std::string& message) const {
		if (this->is_server == false) {
			std::cout << message << "\n";
		} else {
			send(client_socket, message.c_str(), strlen(message.c_str()), 0);
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
		} else {
			o = std::make_unique<Outside>(true);
		}
	}

	while (true) {
		std::string buffer = o->get_input();
		std::cout << "Server says: " << buffer << std::endl;
		if (std::string(buffer) == "exit") {
			break;
		}
		std::string message = "hi from client";
		o->send_output(message);
	}

	return 0;
}