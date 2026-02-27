add_library(common)

target_sources(common
	PRIVATE
		serializeable.cpp
		file_listing.cpp
)

target_sources(common
	PUBLIC
		FILE_SET HEADERS
		FILES
			serializeable.hpp
			file_listing.hpp
)