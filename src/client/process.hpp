#ifndef CLIENT_PROCESS_HPP
#define CLIENT_PROCESS_HPP

#include "process_listing.hpp"

std::vector<ProcessListing> get_process_running();
int kill_process(pid_t pid);

#endif