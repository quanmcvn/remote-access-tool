#include "util.hpp"
#include <cstdint>
#include <chrono>
#include <iostream>

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

void print_progress(int64_t total_received, uint64_t file_size,
                    std::chrono::steady_clock::time_point start_time) {

	double progress = (double)total_received / file_size;
	const int BAR_WIDTH = 50;
	int pos = BAR_WIDTH * progress;

	auto now = std::chrono::steady_clock::now();
	double seconds = std::chrono::duration<double>(now - start_time).count();

	double speed = (total_received / 1024.0 / 1024.0) / seconds;

	std::cout << "\r[";
	for (int i = 0; i < BAR_WIDTH; ++i) {
		if (i < pos)
			std::cout << "=";
		else if (i == pos)
			std::cout << ">";
		else
			std::cout << " ";
	}

	std::cout << "] " << std::fixed << std::setprecision(1) << (progress * 100.0) << "% "
	          << "(" << total_received / 1024 / 1024 << " MB / " << file_size / 1024 / 1024 << " MB) "
	          << speed << " MB/s   ";

	std::cout.flush();
}
