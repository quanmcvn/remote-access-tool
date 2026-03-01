#ifndef NETWORK_HPP
#define NETWORK_HPP

// cringe cross platform behavior

#ifdef _WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#pragma comment(lib, "ws2_32.lib")

typedef SOCKET socket_t;

#define CLOSESOCKET closesocket

static inline int network_init() {
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2, 2), &wsa);
}

static inline void network_cleanup() {
	WSACleanup();
}

static inline int network_last_error(void) {
	return WSAGetLastError();
}

static inline const char* network_error_string(int err) {
	static char buffer[512];

	FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buffer,
		sizeof(buffer),
		NULL
	);

	return buffer;
}

static inline void network_perror(const char* msg) {
	int err = network_last_error();
	fprintf(stderr, "%s: (%d) %s\n", msg, err, network_error_string(err));
}

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cstdio>

typedef int socket_t;

#define CLOSESOCKET close

static inline int network_init() {
	return 0;
}

static inline void network_cleanup() {}

static inline int network_last_error(void) {
	return errno;
}

static inline const char* network_error_string(int err) {
	return strerror(err);
}

static inline void network_perror(const char* msg) {
	fprintf(stderr, "%s: (%d) %s\n",
			msg,
			errno,
			strerror(errno));
}

#endif

#endif