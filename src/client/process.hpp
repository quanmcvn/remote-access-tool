#ifndef CLIENT_PROCESS_HPP
#define CLIENT_PROCESS_HPP

#include "process_listing.hpp"

class ProcessHelper {
public:
	static std::vector<ProcessListing> get_process_running();
	static int kill_process(pid_t pid);
};


#endif