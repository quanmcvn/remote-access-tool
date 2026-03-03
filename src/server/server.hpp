#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include <string>
#include <mutex>
#include <sstream>
#include <iostream>

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

class RealCommandProcessor : public CommandProcessor {
public:
	// process the command, returns code to which the main loop decides what to do
	// 0 = ok, continue
	// 1 = not as ok, but we still continue (non fatal error)
	// else, we break
	int process(const std::string& command) override;
};

// used to test
class GetInput {
public:
	virtual std::string get_input() = 0;
};

class StdCinInput : public GetInput {
public:
	std::string get_input() override {
		std::string s;
		getline(std::cin, s);
		return s;
	}
};

// used in test
class FakeInput : public GetInput {
private:
	std::mutex input_mutex;
	std::stringstream fake_input;
public:
	std::string get_input() override {
		std::lock_guard<std::mutex> lock(input_mutex);
		std::string s;
		getline(fake_input, s);
		fake_input.clear();
		return s;
	}
	void add_input(const std::string& s) {
		std::lock_guard<std::mutex> lock(input_mutex);
		fake_input << s;
	}
};

void server_input_thread(GetInput& input_stream, CommandProcessor& processor, int server_socket);

// server main, will be called in tests
// we actually need more than just (int argc, char* argv[])
int server_main(int argc, char* argv[], GetInput& input_stream, CommandProcessor& processor);

#endif