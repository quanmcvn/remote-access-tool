add_library(server_lib)
target_sources(server_lib
	PRIVATE
		server.cpp
)
target_sources(server_lib
	PUBLIC
		FILE_SET HEADERS
		FILES
			server.hpp
)
target_link_libraries(server_lib
	PUBLIC
		common
)

add_executable(server)
target_sources(server
	PRIVATE
		main.cpp
)
target_link_libraries(server
	PUBLIC
		server_lib
)

if (WIN32)
	target_link_libraries(server_lib PUBLIC ws2_32)
endif()