#include "file_listing.hpp"

FileListing::FileListing() {}

FileListing::FileListing(const std::filesystem::directory_entry& path) {
	file_path = path.path().string();
}

void FileListing::serialize(std::ostream& os) const {
	write_string(os, file_path);
	write_uint32(os, static_cast<std::uint32_t>(file_flag));
}

void FileListing::deserialize(std::istream& is) {
	file_path = read_string(is);
	file_flag = read_uint32(is);
}

std::string FileListing::get_file_path() const {
	return file_path;
}