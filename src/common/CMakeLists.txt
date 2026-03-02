add_library(common)

target_sources(common
	PRIVATE
		file_listing.cpp
		serializeable.cpp
		process_listing.cpp
		util.cpp
)

target_sources(common
	PUBLIC
		FILE_SET HEADERS
		FILES
			file_listing.hpp
			network.hpp
			serializeable.hpp
			process_listing.hpp
			util.hpp
)