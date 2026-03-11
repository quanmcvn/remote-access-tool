#ifndef COMMON_I_SERIALIZEABLE_HPP
#define COMMON_I_SERIALIZEABLE_HPP

#include "network.hpp"
#include <cstdint>
#include <iostream>
#include <vector>

class ISerializeable {
public:
	virtual void serialize(std::ostream &os) const = 0;
	virtual void deserialize(std::istream &is) = 0;
	virtual ~ISerializeable() {};
};

// some functions to help with serialization

// swap endian, doesn't care if value is little or big endian (can't)
// apparently c++23 has std::byteswap

class SerializableHelper {
public:
	static std::uint32_t swap_endian(std::uint32_t value);
	static std::int32_t swap_endian(std::int32_t value);
	static std::uint64_t swap_endian(std::uint64_t value);

	static void write_uint32(std::ostream &os, std::uint32_t value);
	static std::uint32_t read_uint32(std::istream &is);

	static void write_string(std::ostream &os, const std::string &str);
	static std::string read_string(std::istream &is);

	static void write_vector_string(std::ostream &os, const std::vector<std::string> &vec);
	static std::vector<std::string> read_vector_string(std::istream &is);

	template <typename T>
	static void write_vector_serializeable(std::ostream &os, const std::vector<T> &vec) {
		uint32_t count = static_cast<uint32_t>(vec.size());
		uint32_t net_count = swap_endian(count);

		os.write(reinterpret_cast<const char *>(&net_count), sizeof(net_count));

		for (const auto &item : vec) {
			item.serialize(os);
		}
	}

	template <typename T>
	static std::vector<T> read_vector_serializeable(std::istream &is) {
		uint32_t net_count;
		is.read(reinterpret_cast<char *>(&net_count), sizeof(net_count));
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

	static void send_exact(socket_t socket_fd, const void *data, size_t size);
	static void recv_exact(socket_t socket_fd, const void *data, size_t size);

	static void send_message(socket_t socket_fd, const std::string &payload);
	static std::string recv_message(socket_t socket_fd);
};
#endif