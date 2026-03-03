#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include <string>

// to inject dependency
// DO NOT ABBREVIATE
class CommandProcessor {
public:
	// process the command, returns code to which the main loop decides what to do
	// 0 = ok, continue
	// 1 = not as ok, but we still continue (non fatal error)
	// else, we break
	virtual int process(const std::string& command) = 0;
};

void server_input_thread(std::istream& input_stream, CommandProcessor& processor, int server_socket) ;

// server main, will be called in tests
int server_main(int argc, char* argv[]);

#endif