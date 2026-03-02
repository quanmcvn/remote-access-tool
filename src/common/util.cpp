#include "util.hpp"

int str_to_int(const std::string& str) {
	int ret = 0;
	for (const char& c : str) {
		if ('0' <= c && c <= '9') {
			ret = ret * 10 + (c - '0');
		} else {
			return -1;
		}
	}
	return ret;
}