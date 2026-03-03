#ifndef COMMON_NETWORK_HPP
#define COMMON_NETWORK_HPP

// cringe cross platform behavior

#ifdef _WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <ws2tcpip.h>
#include <cstdio>

typedef SOCKET socket_t;

#define CLOSESOCKET closesocket

static inline int network_init() {
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2, 2), &wsa);
}

static inline void network_cleanup() {
	WSACleanup();
}

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef int socket_t;

#define CLOSESOCKET(socket_fd) { shutdown(socket_fd, SHUT_RDWR); close(socket_fd); }

static inline int network_init() {
	return 0;
}

static inline void network_cleanup() {}

#endif

#endif