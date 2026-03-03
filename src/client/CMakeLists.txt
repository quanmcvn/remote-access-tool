add_library(client_lib)
target_sources(client_lib
	PRIVATE
		client.cpp
		process.cpp
)
target_sources(client_lib
	PUBLIC
		FILE_SET HEADERS
		FILES
			client.hpp
			process.hpp
)
target_link_libraries(client_lib
	PUBLIC
		common
)

add_executable(client)
target_sources(client
	PRIVATE
		main.cpp
)
target_link_libraries(client
	PRIVATE
		client_lib
)

if (WIN32)
	target_link_libraries(client_lib
		PUBLIC
			ws2_32
			ole32
			oleaut32
			wbemuuid
			shell32
	)
endif()