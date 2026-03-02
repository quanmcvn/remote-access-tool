add_executable(client)

target_sources(client
    PRIVATE
        client.cpp
        process.cpp
)

target_sources(client
	PRIVATE
		FILE_SET HEADERS
		FILES
			process.hpp
)

target_compile_features(client PRIVATE cxx_std_20)

target_link_libraries(client
    PRIVATE
        common
)

if (WIN32)
    target_link_libraries(client 
        PRIVATE 
            ws2_32
            ole32
            oleaut32
            wbemuuid
            shell32
)
endif()