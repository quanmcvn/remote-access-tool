#ifndef FILE_LISTING_HPP
#define FILE_LISTING_HPP

#include <filesystem>

#include "serializeable.hpp"

class FileListing : ISerializeable {
private:
	std::string file_path;
	std::uint32_t file_flag;
public:
	FileListing();
	FileListing(const std::filesystem::directory_entry& path);
	void serialize(std::ostream& os) const override;
	void deserialize(std::istream& is) override;

	std::string get_file_path() const;
	bool is_regular_file() const;
	bool is_directory() const;
	std::uint32_t get_file_flag() const;
};

#endif