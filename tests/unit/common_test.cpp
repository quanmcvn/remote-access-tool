#include <gtest/gtest.h>

#include "serializeable.hpp"
#include "file_listing.hpp"

TEST(CommonTest, swap_endian) {
	EXPECT_EQ(swap_endian(0x12345678u), 0x78563412u);
	EXPECT_EQ(swap_endian(0x1234567890abcdefu), 0xefcdab9078563412u);
}

TEST(CommonTest, read_write_uint32_t) {
	std::ostringstream oss(std::ios::binary);

	uint32_t number = 0x12345678u;
	write_uint32(oss, number);
	
	std::istringstream iss(oss.str(), std::ios::binary);
	uint32_t number_back = read_uint32(iss);

	EXPECT_EQ(number, number_back);
}

TEST(CommonTest, read_write_string) {
	std::ostringstream oss(std::ios::binary);

	std::string string = "hihihaha 0x1234512839 __##!@#!@ ~!@#$%^&*()_+`1234567890-=\\|][}{,.<>;':\"/? \n\tthe quick brown fox jumps over the lazy dog\n";
	write_string(oss, string);

	std::istringstream iss(oss.str(), std::ios::binary);
	std::string string_back = read_string(iss);

	EXPECT_EQ(string, string_back);
}

TEST(CommonTest, read_write_vector_string) {
	std::ostringstream oss(std::ios::binary);

	std::vector<std::string> vector;
	vector.push_back("hihihaha 0x1234512839 __##!@#!@ ~!@#$%^&*()_+`1234567890-=\\|][}{,.<>;':\"/? \n\tthe quick brown fox jumps over the lazy dog\n");
	vector.push_back("1231231232131321");
	vector.push_back("sadklaskdlas");
	vector.push_back("./...lklakdaldklag//fsd/.f./sdfsdka90e21k 'o4560323o");
	write_vector_string(oss, vector);

	std::istringstream iss(oss.str(), std::ios::binary);
	std::vector<std::string> vector_back = read_vector_string(iss);

	EXPECT_EQ(vector, vector_back);
}

TEST(CommonTest, read_write_vector_serializable) {
	std::ostringstream oss(std::ios::binary);

	std::vector<FileListing> vector;
	vector.emplace_back(std::filesystem::directory_entry(std::filesystem::current_path()));
	vector.emplace_back(std::filesystem::directory_entry(std::filesystem::current_path() / "fake_dir1")); // still works
	vector.emplace_back(std::filesystem::directory_entry(std::filesystem::current_path() / "fake_dir2")); // still works
	write_vector_serializeable(oss, vector);

	std::istringstream iss(oss.str(), std::ios::binary);
	std::vector<FileListing> vector_back = read_vector_serializeable<FileListing>(iss);

	// check == by hand :d
	EXPECT_EQ(vector.size(), vector_back.size());
	for (size_t i = 0; i < vector.size(); ++ i) {
		EXPECT_EQ(vector[i].get_file_path(), vector_back[i].get_file_path());
		EXPECT_EQ(vector[i].get_file_flag(), vector_back[i].get_file_flag());
	}
}
