#include <atomic>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <thread>
#include <unistd.h>

#include "error.hpp"
#include "file_listing.hpp"
#include "network.hpp"
#include "process_listing.hpp"
#include "server.hpp"
#include "util.hpp"

#define PORT 12345

namespace {

std::atomic<bool> server_running(true);
int client_counter = 0;

struct ClientSession {
	socket_t socket;
	int id;
	std::queue<std::string> message_queue;
};

class ConnectionManager {
private:
	std::mutex client_mutex;
	std::map<int, ClientSession> clients;

public:
	void exit_all() {
		std::lock_guard<std::mutex> lock(client_mutex);
		for (auto &[id, client] : clients) {
			client.message_queue.push(std::string("exit"));
		}
	}
	bool is_no_client() {
		std::lock_guard<std::mutex> lock(client_mutex);
		return clients.empty();
	}
	void remove_client(int client_id) {
		std::lock_guard<std::mutex> lock(client_mutex);
		clients.erase(client_id);
	}
	void add_new_client(ClientSession client_session) {
		std::lock_guard<std::mutex> lock(client_mutex);
		clients[client_session.id] = std::move(client_session);
	}
	int send_to_queue(const std::string &command) {
		size_t pos = command.find(':');
		if (pos == std::string::npos) {
			std::cerr << "format: client_id:message: " << command << "\n";
			return 1;
		}

		int target_id = str_to_int(command.substr(0, pos));
		std::string message = command.substr(pos + 1);

		std::lock_guard<std::mutex> lock(client_mutex);

		auto it = clients.find(target_id);
		if (it == clients.end()) {
			std::cerr << "can't find client with id " << target_id << " (from "
			          << command.substr(0, pos) << ")\n";
			return 1;
		}
		it->second.message_queue.push(message);
		return 0;
	}
	std::string get_message_from_queue(int client_id) {
		std::lock_guard<std::mutex> lock(client_mutex);
		auto &message_queue = clients[client_id].message_queue;
		if (message_queue.empty()) {
			return "";
		}
		std::string command = message_queue.front();
		message_queue.pop();
		return command;
	}
};

ConnectionManager connection_manager;

} // namespace

int RealCommandProcessor::process(const std::string &command) {
	if (command.empty()) {
		std::cout << "command empty!\n";
		std::this_thread::sleep_for(std::chrono::seconds(1));
		return 1;
	}
	if (command == "exit") {
		connection_manager.exit_all();
		return 2;
	}

	return connection_manager.send_to_queue(command);
}

class ClientHandler {
public:
	static void handle_client(socket_t client_socket, int client_id);
};
void ClientHandler::handle_client(socket_t client_socket, int client_id) {
	try {
		while (true) {
			// polling for message
			// not the best way to do this
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::string command = connection_manager.get_message_from_queue(client_id);
			if (command.empty())
				continue;
			std::cout << "to client #" << client_id << " we say: " << command << "\n";
			SerializableHelper::send_message(client_socket, command);
			if (command == "exit") {
				break;
			}
			if (command == "ls") {
				std::string payload = SerializableHelper::recv_message(client_socket);
				std::istringstream iss(payload, std::ios::binary);
				std::vector<FileListing> fl =
				    SerializableHelper::read_vector_serializeable<FileListing>(iss);
				if (fl.empty()) {
					std::cout << "[EMPTY]\n";
				} else {
					for (const auto &f : fl) {
						std::string type = "[OTHER]";
						if (f.is_directory()) {
							type = "[DIR]  ";
						}
						if (f.is_regular_file()) {
							type = "[FILE] ";
						}
						std::cout << type << " " << f.get_file_path() << "\n";
					}
				}
				continue;
			}
			if (command.starts_with("cd ")) {
				std::string payload = SerializableHelper::recv_message(client_socket);
				std::istringstream iss(payload, std::ios::binary);
				int return_code = SerializableHelper::read_uint32(iss);
				std::cout << "client returns: " << return_code << "\n";
				continue;
			}
			if (command == "pwd") {
				std::string payload = SerializableHelper::recv_message(client_socket);
				std::istringstream iss(payload, std::ios::binary);
				std::string pwd = SerializableHelper::read_string(iss);
				std::cout << pwd << "\n";
				continue;
			}
			if (command.starts_with("ps")) {
				std::string payload = SerializableHelper::recv_message(client_socket);
				std::istringstream iss(payload, std::ios::binary);
				std::vector<ProcessListing> processes =
				    SerializableHelper::read_vector_serializeable<ProcessListing>(iss);
				for (const auto &proc : processes) {
					std::cout << proc.get_pid() << " " << proc.get_proc_name() << "\n";
					for (const auto &arg : proc.get_args()) {
						std::cout << arg << " ";
					}
					std::cout << "\n";
				}
				continue;
			}
			if (command.starts_with("kill")) {
				std::string payload = SerializableHelper::recv_message(client_socket);
				std::istringstream iss(payload, std::ios::binary);
				int return_code = SerializableHelper::read_uint32(iss);
				std::string error_code = SerializableHelper::read_string(iss);
				std::cout << "client returns: " << return_code << " " << error_code << "\n";
				continue;
			}
			if (command.starts_with("download ")) {
				std::string arg = command.substr(std::string("download ").length());
				arg = std::filesystem::path(arg).filename();
				uint64_t net_size;
				SerializableHelper::recv_exact(client_socket, &net_size, sizeof(net_size));
				if (net_size == 0) {
					std::string payload = SerializableHelper::recv_message(client_socket);
					std::istringstream iss(payload, std::ios::binary);
					std::string error_code = SerializableHelper::read_string(iss);
					std::cout << "client says error: " << error_code << "\n";
					continue;
				}
				uint64_t file_size = SerializableHelper::swap_endian(net_size);
				std::ofstream out("downloaded_" + arg, std::ios::binary);

				const uint64_t BUFFER_SIZE = 8192;
				char buffer[BUFFER_SIZE];
				uint64_t total_received = 0;

				auto start_time = std::chrono::steady_clock::now();

				while (total_received < file_size) {
					ssize_t bytes = recv(client_socket, buffer,
					                     std::min(BUFFER_SIZE, file_size - total_received), 0);
					if (bytes <= 0)
						break;

					out.write(buffer, bytes);
					total_received += bytes;

					print_progress(total_received, file_size, start_time);
				}
				out.close();
				std::cout << "download completed\n";
				// receive "ok"
				std::string payload = SerializableHelper::recv_message(client_socket);
				std::istringstream iss(payload, std::ios::binary);
				std::string error_code = SerializableHelper::read_string(iss);
				std::cout << "client says: " << error_code << "\n";
				continue;
			}
			std::string message = SerializableHelper::recv_message(client_socket);
			std::cout << "client says: " << message << std::endl;
		}
	} catch (std::exception &err) {
		std::cerr << "Error: " << err.what() << "\n";
	}
	connection_manager.remove_client(client_id);
	network_close_socket(client_socket);
}

void ServerCLI::server_input_thread(GetInput &input_stream, CommandProcessor &processor,
                                    socket_t server_socket) {
	while (true) {
		std::string input = input_stream.get_input();
		int ret = processor.process(input);
		if (ret == 0)
			continue;
		if (ret == 1)
			continue;
		break;
	}
	while (true) {
		// polling #2
		// only exit when every client exited
		std::cout << "waiting for clients...\n";
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (connection_manager.is_no_client())
			break;
	}
	server_running = false;
	// have to close socket here because accept() blocks...
	network_close_socket(server_socket);
}

class NetworkListener {
public:
	static void listen_for_client(socket_t server_socket) {
		while (server_running) {
			sockaddr_in client_addr{};
			socklen_t client_size = sizeof(client_addr);
			socket_t client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_size);
			if (!server_running || client_socket < 0) {
				break;
			}
			char client_address_ip[18];
			inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_address_ip, 18);
			std::cout << "client connected!\n";
			std::cout << client_address_ip << " " << client_addr.sin_port << "\n";
			ClientSession client;
			client.id = client_counter++;
			client.socket = client_socket;
			std::cout << "this will be client #" << client.id << "\n";
			connection_manager.add_new_client(std::move(client));

			std::thread(ClientHandler::handle_client, client.socket, client.id).detach();
		}
	}
};

int server_main(int argc, char *argv[], GetInput &input_stream,
                CommandProcessor &command_processor) {
	int chosen_port = PORT;
	for (int i = 1; i < argc; ++i) {
		std::string arg = std::string(argv[i]);
		if (arg.starts_with("--server-port=")) {
			std::string server_port = arg.substr(std::string("--server-port=").length());
			int port = str_to_int(server_port);
			if (port < 0) {
				std::cerr << "--server-port: error in converting " << server_port << " to int\n";
			} else if (port < 1024 || port > 65536) {
				std::cerr << "--server-port: invalid port " << port << "\n";
			} else {
				chosen_port = port;
			}
		}
	}

	std::cout << "choosing port: " << chosen_port << "\n";

	if (network_init() != 0) {
		print_error("network init failed");
		return 1;
	}
	socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		print_error("create socket failed");
		return 1;
	}

	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(chosen_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	int opt = 1;
#ifdef _WIN32
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)(&opt), sizeof(opt));
#else
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
	if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		print_error("bind failed");
		return 1;
	}

	listen(server_socket, 1);
	std::cout << "server listening on port " << chosen_port << "...\n";

	std::thread input_thread(ServerCLI::server_input_thread, std::ref(input_stream),
	                         std::ref(command_processor), server_socket);

	int client_counter = 1;

	NetworkListener::listen_for_client(server_socket);

	input_thread.join();

	network_cleanup();
	return 0;
}