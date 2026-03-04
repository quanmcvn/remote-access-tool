#ifndef COMMON_NETWORK_HPP
#define COMMON_NETWORK_HPP

#ifdef _WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <ws2tcpip.h>
#include <cstdio>

typedef SOCKET socket_t;

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef int socket_t;

#endif

int network_init();

void network_cleanup();

void network_close_socket(socket_t socket);

#endif