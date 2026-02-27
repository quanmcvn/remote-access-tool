#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include "file_listing.hpp"

#define PORT 12345

int main() {
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		perror("create socket failed");
		return 1;
	}

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(12345);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	int opt = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if (bind(server_socket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("bind failed");
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
	std::cout << client_address_ip <<" " << clientAddr.sin_port << "\n";

	while (true)
	{
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
			std::vector<FileListing> fl = deserialize_vector<FileListing>(iss);
			for (const auto& f : fl) {
				std::cout << f.get_file_path() << "\n";
			}
		} else {
			std::string message = recv_message(client_socket);
			std::cout << "Client says: " << message << std::endl;
		}
	}
	
	close(client_socket);
	close(server_socket);

	return 0;
}