#include <cstring>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <unistd.h>

#include "client.hpp"
#include "error.hpp"
#include "file_listing.hpp"
#include "network.hpp"
#include "process.hpp"
#include "util.hpp"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345

namespace {

class Outside {
private:
	std::string server_ip;
	int server_port;
	socket_t client_socket;
	sockaddr_in server_addr;

	void init_non_local() {
		client_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (client_socket < 0) {
			std::cerr << "Socket creation failed\n";
			exit(1);
		}
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(server_port);
		inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

		if (connect(client_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
			std::cerr << "Connection failed\n";
			exit(1);
		}
		std::cerr << "init done\n";
	}

public:
	Outside(std::string n_server_ip, int n_server_port)
	    : server_ip(n_server_ip), server_port(n_server_port) {
		init_non_local();
	}
	std::string get_input() const { return SerializableHelper::recv_message(client_socket); }
	void send_output(const std::string &message) const { SerializableHelper::send_message(client_socket, message); }
	int get_client_socket() const { return client_socket; }
	~Outside() { network_close_socket(client_socket); }
};

std::unique_ptr<Outside> o;

} // namespace

std::vector<FileListing> ClientCommandProcessor::get_files_in_directory() {
	std::vector<FileListing> fl;
	for (const auto &entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
		fl.emplace_back(entry);
	}
	return fl;
}

int ClientCommandProcessor::change_directory(const std::string &filepath) {
	try {
		std::filesystem::current_path(filepath);
		return 0;
	} catch (const std::filesystem::filesystem_error &e) {
		return 1;
	}
}
std::string ClientCommandProcessor::get_pwd() { return std::filesystem::current_path().string(); }
std::string ClientCommandProcessor::download_file(const std::string &filepath) {
	int client_socket = o->get_client_socket();
	// doing special packet sending due to big file (may or may not)
	if (!std::filesystem::is_regular_file(std::filesystem::path(filepath))) {
		uint64_t size = 0;
		// doesn't need to swap endian because 0
		SerializableHelper::send_exact(client_socket, &size, sizeof(size));
		return std::format("{} does not exist or is not regular file!", filepath.c_str());
	}
	std::ifstream file(filepath, std::ios::binary);
	if (!file) {
		uint64_t size = 0;
		SerializableHelper::send_exact(client_socket, &size, sizeof(size));
		return std::format("can't open {}!", filepath.c_str());
	}

	uint64_t file_size = std::filesystem::file_size(std::filesystem::path(filepath));
	if (file_size == 0) {
		SerializableHelper::send_exact(client_socket, &file_size, sizeof(file_size));
		return std::format("file {} size is 0!", filepath.c_str());
	}

	uint64_t net_file_size = SerializableHelper::swap_endian(file_size);
	SerializableHelper::send_exact(client_socket, &net_file_size, sizeof(net_file_size));

	const uint64_t BUFFER_SIZE = 8192;
	char buffer[BUFFER_SIZE];

	while (file) {
		file.read(buffer, BUFFER_SIZE);
		std::streamsize bytes_read = file.gcount();
		if (bytes_read > 0) {
			SerializableHelper::send_exact(client_socket, buffer, bytes_read);
		}
	}
	return "ok";
}

std::string ClientCommandProcessor::run(const std::string &command) {
	if (command == "ls") {
		std::ostringstream oss(std::ios::binary);
		std::vector<FileListing> fl = ClientCommandProcessor::get_files_in_directory();
		SerializableHelper::write_vector_serializeable(oss, fl);
		return oss.str();
	}
	if (command.starts_with("cd ")) {
		std::string arg = command.substr(std::string("cd ").length());
		std::ostringstream oss(std::ios::binary);
		int ret = ClientCommandProcessor::change_directory(arg);
		SerializableHelper::write_uint32(oss, static_cast<uint32_t>(ret));
		return oss.str();
	}
	if (command == "pwd") {
		std::string arg = command.substr(std::string("pwd").length());
		std::ostringstream oss(std::ios::binary);
		SerializableHelper::write_string(oss, ClientCommandProcessor::get_pwd());
		return oss.str();
	}
	if (command == "ps") {
		std::vector<ProcessListing> processes = ProcessHelper::get_process_running();
		std::ostringstream oss(std::ios::binary);
		SerializableHelper::write_vector_serializeable(oss, processes);
		return oss.str();
	}
	if (command.starts_with("kill ")) {
		std::string arg = command.substr(std::string("kill ").length());
		std::ostringstream oss(std::ios::binary);
		int pid = str_to_int(arg);
		if (pid <= 0) {
			SerializableHelper::write_uint32(oss, 1u);
			SerializableHelper::write_string(oss, std::format("can't parse pid {} to int or invalid", pid));
		} else {
			int return_code = ProcessHelper::kill_process(pid);
			SerializableHelper::write_uint32(oss, return_code);
			if (return_code == 0) {
				SerializableHelper::write_string(oss, "ok");
			} else {
				SerializableHelper::write_string(oss, std::format("kill: {}", get_last_error_string(get_last_error())));
			}
		}
		return oss.str();
	}
	if (command.starts_with("download ")) {
		std::string arg = command.substr(std::string("download ").length());
		std::ostringstream oss(std::ios::binary);
		std::string return_string = ClientCommandProcessor::download_file(arg);
		SerializableHelper::write_string(oss, return_string);
		return oss.str();
	}

	return std::format("{}: not recognized", command.c_str());
}

int client_main(int argc, char *argv[]) {
	std::string chosen_ip = SERVER_IP;
	int chosen_port = SERVER_PORT;
	for (int i = 1; i < argc; ++i) {
		std::string arg = std::string(argv[i]);
		if (arg.starts_with("--server-ip=")) {
			std::string server_ip = arg.substr(std::string("--server-ip=").length());
			chosen_ip = server_ip;
		} else if (arg.starts_with("--server-port=")) {
			std::string server_port = arg.substr(std::string("--server-port=").length());
			int port = str_to_int(server_port);
			if (port < 0) {
				std::cerr << "--server-port: error in converting " << server_port << " to int\n";
			} else if (port == 0 || port > 65536) {
				std::cerr << "--server-port: invalid port " << port << "\n";
			} else {
				chosen_port = port;
			}
		}
	}
	std::cout << "chosing " << chosen_ip << ":" << chosen_port << "\n";
	o = std::make_unique<Outside>(chosen_ip, chosen_port);

	while (true) {
		std::string buffer = o->get_input();
		std::cout << "server says: " << buffer << std::endl;
		if (buffer == "exit") {
			break;
		}
		std::string message = ClientCommandProcessor::run(buffer);
		o->send_output(message);
	}
	return 0;
}