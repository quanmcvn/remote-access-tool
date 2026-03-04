#include "network.hpp"

#ifdef _WIN32

typedef SOCKET socket_t;

int network_init() {
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2, 2), &wsa);
}

void network_cleanup() {
	WSACleanup();
}

void network_close_socket(socket_t socket) {
	closesocket(socket);
}

#else

typedef int socket_t;

int network_init() {
	return 0;
}

void network_cleanup() {}

void network_close_socket(socket_t socket) {
	shutdown(socket, SHUT_RDWR);
	close(socket);
}

#endif