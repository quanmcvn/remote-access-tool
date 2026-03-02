#include "serializeable.hpp"

#include "network.hpp"

std::uint32_t swap_endian(std::uint32_t value) {
	return ((value & 0x000000FFu) << 24) | ((value & 0x0000FF00u) << 8) |
	       ((value & 0x00FF0000u) >> 8) | ((value & 0xFF000000u) >> 24);
}

std::int32_t swap_endian(std::int32_t value) {
	std::uint32_t u = static_cast<std::uint32_t>(value);
	u = swap_endian(u);
	return static_cast<std::int32_t>(u);
}

void write_uint32(std::ostream &os, std::uint32_t value) {
	std::uint32_t be = swap_endian(value);
	os.write(reinterpret_cast<const char *>(&be), sizeof(be));
}

std::uint32_t read_uint32(std::istream &is) {
	std::uint32_t be;
	is.read(reinterpret_cast<char *>(&be), sizeof(be));
	if (!is) {
		throw std::runtime_error("read failed");
	}
	return swap_endian(be);
}

void write_string(std::ostream &os, const std::string &str) {
	if (str.size() > UINT32_MAX) {
		throw std::runtime_error("string too large");
	}

	write_uint32(os, static_cast<std::uint32_t>(str.size()));
	os.write(str.data(), str.size());
}

std::string read_string(std::istream &is) {
	std::uint32_t size = read_uint32(is);

	std::string str(size, '\0');
	is.read(str.data(), size);
	if (!is) {
		throw std::runtime_error("read failed");
	}
		
	return str;
}

void write_vector_string(std::ostream& os, const std::vector<std::string>& vec) {
	uint32_t count = static_cast<uint32_t>(vec.size());
	uint32_t net_count = swap_endian(count);

	os.write(reinterpret_cast<const char*>(&net_count), sizeof(net_count));

	for (const auto &item : vec) {
		write_string(os, item);
	}
}

std::vector<std::string> read_vector_string(std::istream& is) {
	uint32_t net_count;
	is.read(reinterpret_cast<char*>(&net_count), sizeof(net_count));
	uint32_t count = swap_endian(net_count);

	std::vector<std::string> vec;
	vec.reserve(count);

	for (uint32_t i = 0; i < count; ++i) {
		std::string item = read_string(is);
		vec.push_back(item);
	}

	return vec;
}

void send_exact(socket_t socket_fd, const void *data, size_t size) {
	const char *buffer = static_cast<const char *>(data);
	size_t total_sent = 0;

	while (total_sent < size) {
		ssize_t sent = send(socket_fd, buffer + total_sent, size - total_sent, 0);

		if (sent <= 0) {
			throw std::runtime_error("send failed");
		}

		total_sent += sent;
	}
}

void recv_exact(socket_t socket_fd, const void* data, size_t size) {
	void* buffer = const_cast<void*>(data);
	size_t total_received = 0;

	while (total_received < size) {
		ssize_t received = recv(socket_fd, static_cast<char*>(buffer) + total_received, size - total_received, 0);

		if (received <= 0) {
			throw std::runtime_error("recv failed");
		}

		total_received += received;
	}
}

void send_message(socket_t socket_fd, const std::string &payload) {
	uint32_t size = static_cast<uint32_t>(payload.size());
	uint32_t net_size = swap_endian(size);

	send_exact(socket_fd, &net_size, sizeof(net_size));

	send_exact(socket_fd, payload.data(), payload.size());
}

std::string recv_message(socket_t socket_fd) {
	uint32_t net_size;
	recv_exact(socket_fd, &net_size, sizeof(net_size));

	uint32_t size = swap_endian(net_size);

	std::string buffer(size, '\0');
	recv_exact(socket_fd, buffer.data(), size);

	return buffer;
}