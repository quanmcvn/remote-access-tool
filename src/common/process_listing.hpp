#ifndef COMMON_PROCESS_LISTING_HPP
#define COMMON_PROCESS_LISTING_HPP

#include <cstdint>
#include <vector>
#include <string>

#include "serializeable.hpp"

class ProcessListing : ISerializeable {
private:
	uint32_t pid;
	std::string proc_name;
	std::vector<std::string> args;
public:
	ProcessListing();
	ProcessListing(uint32_t n_pid, const std::string& n_proc_name, const std::vector<std::string>& n_args);
	~ProcessListing() override;
	void serialize(std::ostream &os) const override;
	void deserialize(std::istream &is) override;

	uint32_t get_pid() const;
	std::string get_proc_name() const;
	std::vector<std::string> get_args() const;

};

#endif