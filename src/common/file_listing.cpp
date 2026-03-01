#include "file_listing.hpp"

FileListing::FileListing() {}

FileListing::FileListing(const std::filesystem::directory_entry& path) : file_path(path.path().string()), file_flag(0) {
	if (path.is_regular_file()) {
		file_flag |= (1u);
	}
	if (path.is_directory()) {
		file_flag |= (1u << 1);
	}
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

bool FileListing::is_regular_file() const {
	return ((file_flag & 0x1) > 0);
}

bool FileListing::is_directory() const {
	return ((file_flag & (1u << 1)) > 0);
}

std::uint32_t FileListing::get_file_flag() const {
	return file_flag;
}