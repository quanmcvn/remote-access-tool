#ifndef I_SERIALIZEABLE_HPP
#define I_SERIALIZEABLE_HPP

#include <cstdint>
#include <iostream>
#include <vector>

class ISerializeable {
  public:
	virtual void serialize(std::ostream &os) const = 0;
	virtual void deserialize(std::istream &is) = 0;
};

// some functions to help with serialization

// swap endian, doesn't care if value is little or big endian (can't)
// apparently c++23 has std::byteswap
std::uint32_t swap_endian(std::uint32_t value);
std::int32_t swap_endian(std::int32_t value);

void write_uint32(std::ostream &os, std::uint32_t value);
std::uint32_t read_uint32(std::istream &is);

void write_string(std::ostream &os, const std::string &str);
std::string read_string(std::istream &is);

void send_exact(int socket_fd, const void *data, size_t size);
void recv_exact(int socket_fd, const void *data, size_t size);

void send_message(int socket_fd, const std::string &payload);
std::string recv_message(int socket_fd);

template <typename T> void serialize_vector(std::ostream &os, const std::vector<T> &vec) {
	uint32_t count = static_cast<uint32_t>(vec.size());
	uint32_t net_count = swap_endian(count);

	os.write(reinterpret_cast<const char*>(&net_count), sizeof(net_count));

	for (const auto &item : vec) {
		item.serialize(os);
	}
}

template <typename T> std::vector<T> deserialize_vector(std::istream &is) {
	uint32_t net_count;
	is.read(reinterpret_cast<char*>(&net_count), sizeof(net_count));
	uint32_t count = swap_endian(net_count);

	std::vector<T> vec;
	vec.reserve(count);

	for (uint32_t i = 0; i < count; ++i) {
		T item;
		item.deserialize(is);
		vec.push_back(item);
	}

	return vec;
}

#endif