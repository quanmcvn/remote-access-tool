#include "process_listing.hpp"


ProcessListing::ProcessListing() {}

ProcessListing::ProcessListing(uint32_t n_pid, const std::string& n_proc_name, const std::vector<std::string>& n_args) : 
pid(n_pid), 
proc_name(n_proc_name),
args(n_args) {
	
}

ProcessListing::~ProcessListing() {}

void ProcessListing::serialize(std::ostream &os) const {
	write_uint32(os, this->pid);
	write_string(os, this->proc_name);
	write_vector_string(os, this->args);
}

void ProcessListing::deserialize(std::istream &is) {
	this->pid = read_uint32(is);
	this->proc_name = read_string(is);
	this->args = read_vector_string(is);
}

uint32_t ProcessListing::get_pid() const { return pid; }
std::string ProcessListing::get_proc_name() const { return proc_name; }
std::vector<std::string> ProcessListing::get_args() const { return args; }