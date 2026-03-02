#ifndef COMMON_ERROR_HPP
#define COMMON_ERROR_HPP

int get_last_error();
const char* get_last_error_string(int err);
void print_error(const char* msg);


#endif