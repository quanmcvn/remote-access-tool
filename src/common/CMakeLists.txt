add_library(common)

target_sources(common
	PRIVATE
		file_listing.cpp
		serializeable.cpp
		util.cpp
)

target_sources(common
	PUBLIC
		FILE_SET HEADERS
		FILES
			file_listing.hpp
			network.hpp
			serializeable.hpp
			util.hpp
)