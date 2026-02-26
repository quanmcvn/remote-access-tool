#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

#define PORT 12345

int main() {
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0) {
		std::cerr << "Socket creation failed\n";
		return 1;
	}

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(12345);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		std::cerr << "Bind failed\n";
		return 1;
	}

	listen(serverSocket, 1);
	std::cout << "Server listening on port " << PORT << "...\n";

	sockaddr_in clientAddr{};
	socklen_t clientSize = sizeof(clientAddr);
	int clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientSize);

	char client_address_ip[18];
	inet_ntop(AF_INET, &clientAddr.sin_addr.s_addr, client_address_ip, 18);
	std::cout << "Client connected!\n";
	std::cout << client_address_ip <<" " << clientAddr.sin_port << "\n";

	while (true)
	{
		std::string s;
		getline(std::cin, s);
		send(clientSocket, s.c_str(), strlen(s.c_str()), 0);
		if (s == "exit") {
			break;
		}
		
		char buffer[1024] = {0};
		recv(clientSocket, buffer, 1024, 0);
		std::cout << "Client says: " << buffer << std::endl;
	}
	
	close(clientSocket);
	close(serverSocket);

	return 0;
}