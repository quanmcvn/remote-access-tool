#include "process.hpp"

#include <filesystem>
#include <fstream>

#include "util.hpp"

#ifdef _WIN32

#include <windows.h>
#include <shellapi.h>
#include <wbemidl.h>
#include <wrl/client.h>

#include <vector>
#include <string>
#include <stdexcept>

using Microsoft::WRL::ComPtr;

class ComInit {
public:
	ComInit() {
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (FAILED(hr))
			throw std::runtime_error("CoInitializeEx failed");

		hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
		                          RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);

		if (FAILED(hr) && hr != RPC_E_TOO_LATE) {
			CoUninitialize();
			throw std::runtime_error("CoInitializeSecurity failed");
		}
	}

	~ComInit() { CoUninitialize(); }
};

class Variant {
public:
	VARIANT v;

	Variant() { VariantInit(&v); }

	~Variant() { VariantClear(&v); }

	VARIANT *operator&() { return &v; }

	Variant(const Variant &) = delete;
	Variant &operator=(const Variant &) = delete;
};

class BStr {
	BSTR str = nullptr;

public:
	BStr(const wchar_t *s) {
		str = SysAllocString(s);
		if (!str)
			throw std::bad_alloc();
	}

	~BStr() {
		if (str)
			SysFreeString(str);
	}

	BSTR get() const { return str; }

	BStr(const BStr &) = delete;
	BStr &operator=(const BStr &) = delete;

	BStr(BStr &&other) noexcept : str(other.str) { other.str = nullptr; }

	BStr &operator=(BStr &&other) noexcept {
		if (this != &other) {
			if (str)
				SysFreeString(str);
			str = other.str;
			other.str = nullptr;
		}
		return *this;
	}
};

static std::string wide_to_utf8(const std::wstring &wstr) {
	if (wstr.empty())
		return {};

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0,
	                                      nullptr, nullptr);

	std::string result(size_needed, 0);

	WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), result.data(), size_needed,
	                    nullptr, nullptr);

	return result;
}

std::vector<ProcessListing> get_process_running() {
	std::vector<ProcessListing> processes;
	try {

		ComInit com;

		ComPtr<IWbemLocator> locator;

		HRESULT hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
		                              IID_PPV_ARGS(&locator));
		if (FAILED(hr))
			throw std::runtime_error("CoCreateInstance failed");

		ComPtr<IWbemServices> services;

		BStr namespaceStr(L"ROOT\\CIMV2");
		hr = locator->ConnectServer(namespaceStr.get(), nullptr, nullptr, nullptr, 0, nullptr,
		                            nullptr, &services);

		if (FAILED(hr))
			throw std::runtime_error("ConnectServer failed");

		hr = CoSetProxyBlanket(services.Get(), RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
		                       RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr,
		                       EOAC_NONE);

		if (FAILED(hr))
			throw std::runtime_error("CoSetProxyBlanket failed");

		ComPtr<IEnumWbemClassObject> enumerator;

		BStr queryLang(L"WQL");
		BStr query(L"SELECT ProcessId, CommandLine FROM Win32_Process");

		hr = services->ExecQuery(queryLang.get(), query.get(),
		                         WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr,
		                         &enumerator);

		if (FAILED(hr))
			throw std::runtime_error("ExecQuery failed");

		while (true) {
			ComPtr<IWbemClassObject> obj;
			ULONG returned = 0;

			hr = enumerator->Next(WBEM_INFINITE, 1, &obj, &returned);

			if (returned == 0)
				break;

			Variant pid;
			Variant cmd;

			obj->Get(L"ProcessId", 0, &pid.v, nullptr, nullptr);
			obj->Get(L"CommandLine", 0, &cmd.v, nullptr, nullptr);

			if (cmd.v.vt == VT_BSTR && cmd.v.bstrVal) {
				std::wcout << L"PID: " << pid.v.uintVal << L"\n";
				std::wcout << L"CMD: " << cmd.v.bstrVal << L"\n\n";

				int argc = 0;
				LPWSTR *argv = CommandLineToArgvW(cmd.v.bstrVal, &argc);

				if (argv) {
					std::vector<std::string> args;
					for (int i = 0; i < argc; i++) {
						args.push_back(wide_to_utf8(argv[i]));
					}

					LocalFree(argv);

					processes.emplace_back(pid.v.uintVal, wide_to_utf8(cmd.v.bstrVal), args);
				} else {
					throw std::runtime_error("CommandLineToArgvW failed");
				}
			}
		}
	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return processes;
}

int kill_process(pid_t pid) {
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if (hProcess == NULL) {
		return 1;
	}
	if (!TerminateProcess(hProcess, 1)) {
		CloseHandle(hProcess);
		return 1;
	}
	CloseHandle(hProcess);
	return 0;
}
#else

#include <signal.h>

static std::vector<std::string> read_cmdline(const std::filesystem::path &cmdline_path) {
	std::ifstream file(cmdline_path, std::ios::binary);
	if (!file)
		return {};

	std::vector<std::string> args;
	std::string arg;

	while (std::getline(file, arg, '\0')) {
		if (!arg.empty()) {
			args.push_back(arg);
		}
	}

	return args;
}

std::vector<ProcessListing> get_process_running() {
	// we read /proc
	std::vector<ProcessListing> processes;
	for (const auto &entry : std::filesystem::directory_iterator("/proc")) {
		if (!entry.is_directory())
			continue;

		std::string pid_string = entry.path().filename().string();
		int pid = str_to_int(pid_string);
		if (pid <= 0) {
			continue;
		}

		std::ifstream comm_file(entry.path() / "comm");
		if (!comm_file.is_open()) {
			continue;
		}

		std::string process_name;
		std::getline(comm_file, process_name);

		auto args = read_cmdline(entry.path() / "cmdline");
		if (args.empty()) {
			continue; // kernel threads or permission denied
		}

		processes.emplace_back(pid, process_name, args);
	}
	return processes;
}

int kill_process(pid_t pid) {
	if (kill(pid, SIGTERM) == 0) {
		return 0;
	} else {
		return 1;
	}
}

#endif