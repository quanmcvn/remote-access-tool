#ifndef CLIENT_CLIENT_HPP
#define CLIENT_CLIENT_HPP

#include <string>

// remote access tool behaves like a shell none the less, though without fork(), exec() etc
std::string run(const std::string& command);

// client main, will be called in tests
int client_main(int argc, char* argv[]);

#endif