#include "process.hpp"

#include <filesystem>
#include <fstream>

#include "util.hpp"

#ifdef _WIN32
std::vector<ProcessListing> get_process_running() {
	// NYI
}
#else

#include <signal.h>

static std::vector<std::string> read_cmdline(const std::filesystem::path &cmdline_path) {
	std::ifstream file(cmdline_path, std::ios::binary);
	if (!file)
		return {};

	std::vector<std::string> args;
	std::string arg;

	while (std::getline(file, arg, '\0')) {
		if (!arg.empty()) {
			args.push_back(arg);
		}
	}

	return args;
}

std::vector<ProcessListing> get_process_running() {
	// we read /proc
	std::vector<ProcessListing> processes;
	for (const auto &entry : std::filesystem::directory_iterator("/proc")) {
		if (!entry.is_directory())
			continue;

		std::string pid_string = entry.path().filename().string();
		int pid = str_to_int(pid_string);
		if (pid <= 0) {
			continue;
		}

		std::ifstream comm_file(entry.path() / "comm");
		if (!comm_file.is_open()) {
			continue;
		}

		std::string process_name;
		std::getline(comm_file, process_name);

		auto args = read_cmdline(entry.path() / "cmdline");
		if (args.empty()) {
			continue; // kernel threads or permission denied
		}

		processes.emplace_back(pid, process_name, args);
	}
	return processes;
}

int kill_process(pid_t_t pid) {
	if (kill(pid, SIGTERM) == 0) {
		return 0;
	} else {
		return 1;
	}
}

#endif