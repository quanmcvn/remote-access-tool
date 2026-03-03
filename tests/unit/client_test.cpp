#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "client.hpp"
#include "file_listing.hpp"

class ShellFsTest : public ::testing::Test {
protected:
	std::filesystem::path temp_dir;

	void SetUp() override {
		temp_dir = std::filesystem::temp_directory_path() / "shell_test";
		std::filesystem::create_directory(temp_dir);
		std::filesystem::current_path(temp_dir);
	}

	void TearDown() override { std::filesystem::remove_all(temp_dir); }
};

TEST_F(ShellFsTest, ls) {
	std::vector<std::string> filenames_to_test;
	filenames_to_test.push_back("a.txt");
	filenames_to_test.push_back("b.c");
	filenames_to_test.push_back("c");
	for (const auto& f : filenames_to_test) {
		std::ofstream file(f); // make empty file, should be same as `touch`
	}
	// changing name to absolute path as that's what our `ls` returns
	for (auto& f : filenames_to_test) {
		f = (temp_dir / f).string();
	}

	std::string result = run("ls");

	std::istringstream iss(result, std::ios::binary);
	std::vector<FileListing> fl = read_vector_serializeable<FileListing>(iss);

	std::set<std::string> filenames;
	for (const auto& f : fl) {
		filenames.insert(f.get_file_path());
	}
	for (const auto& f : filenames_to_test) {
		EXPECT_NE(filenames.find(f), filenames.end());
	}
}

TEST_F(ShellFsTest, cd_and_pwd) {
	std::filesystem::create_directory(temp_dir / "dir1");
	
	std::string result_cd = run("cd dir1");
	std::istringstream iss(result_cd, std::ios::binary);
	uint32_t ret = read_uint32(iss);
	EXPECT_EQ(ret, 0);

	std::string result_pwd = run("pwd");
	iss = std::istringstream(result_pwd, std::ios::binary);
	std::string path = read_string(iss);

	EXPECT_EQ(path, (temp_dir / "dir1").string());
}

TEST_F(ShellFsTest, cd_failed_and_pwd) {
	// no dir1 to cd to
	// std::filesystem::create_directory(temp_dir / "dir1");
	
	std::string result_cd = run("cd dir1");
	std::istringstream iss(result_cd, std::ios::binary);
	uint32_t ret = read_uint32(iss);
	EXPECT_EQ(ret, 1);

	std::string result_pwd = run("pwd");
	iss = std::istringstream(result_pwd, std::ios::binary);
	std::string path = read_string(iss);

	EXPECT_EQ(path, (temp_dir).string());
}