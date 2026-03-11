#ifndef COMMON_UTIL_HPP
#define COMMON_UTIL_HPP

#include <chrono>
#include <cstdint>
#include <string>

// actually uint32 but we need error -1
int str_to_int(const std::string &str);

void print_progress(uint64_t total_received, uint64_t file_size,
                    std::chrono::steady_clock::time_point start_time);
#endif