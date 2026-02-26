#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

#define SERVER_IP "192.168.8.128"
#define SERVER_PORT 12345

int main() {
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket < 0) {
		std::cerr << "Socket creation failed\n";
		return 1;
	}

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

	if (connect(clientSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		std::cerr << "Connection failed\n";
		return 1;
	}

	while (true) {
		char buffer[1024] = {0};
		recv(clientSocket, buffer, 1024, 0);
		std::cout << "Server says: " << buffer << std::endl;
		if (std::string(buffer) == "exit") {
			break;
		}
		const char *message = "Hello from client!";
		send(clientSocket, message, strlen(message), 0);
	}

	close(clientSocket);

	return 0;
}