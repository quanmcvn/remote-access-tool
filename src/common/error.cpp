#include "error.hpp"

#include <winsock2.h>
#include <winbase.h>
#include <iostream>
#include <format>

#ifdef _WIN32

int get_last_error() {
	return WSAGetLastError();
}

const char* get_last_error_string(int err) {
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

#else

#include <errno.h>
#include <cstring>

int get_last_error() {
	return errno;
}

const char* get_last_error_string(int err) {
	return strerror(err);
}

#endif

void print_error(const char* msg) {
	int err = get_last_error();
	std::cerr << std::format("{}: ({}) {}\n", msg, err, get_last_error_string(err));
}