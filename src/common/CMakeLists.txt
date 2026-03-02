add_library(common)

target_sources(common
	PRIVATE
		error.cpp
		file_listing.cpp
		serializeable.cpp
		process_listing.cpp
		util.cpp
)

target_sources(common
	PUBLIC
		FILE_SET HEADERS
		FILES
			error.hpp
			file_listing.hpp
			network.hpp
			serializeable.hpp
			process_listing.hpp
			util.hpp
)

target_compile_features(common PRIVATE cxx_std_20)