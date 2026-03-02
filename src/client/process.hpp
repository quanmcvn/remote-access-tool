#ifndef CLIENT_PROCESS_HPP
#define CLIENT_PROCESS_HPP

#include "process_listing.hpp"

#ifdef _WIN32
#else
	typedef pid_t pid_t_t;
#endif

std::vector<ProcessListing> get_process_running();
int kill_process(pid_t_t pid);

#endif