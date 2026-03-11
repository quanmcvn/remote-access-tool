#ifndef CLIENT_CLIENT_HPP
#define CLIENT_CLIENT_HPP

#include <string>
#include <vector>

#include "file_listing.hpp"

class ClientCommandProcessor {
private:
	static std::vector<FileListing> get_files_in_directory();
	static int change_directory(const std::string& filepath);
	static std::string get_pwd();
	static std::string download_file(const std::string& filepath);
public:
	// remote access tool behaves like a shell none the less, though without fork(), exec() etc
	static std::string run(const std::string& command);
};


// client main, will be called in tests
int client_main(int argc, char* argv[]);

#endif