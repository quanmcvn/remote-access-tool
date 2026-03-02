add_executable(server)

target_sources(server
    PRIVATE
        server.cpp
)

target_compile_features(server PRIVATE cxx_std_20)

target_link_libraries(server
    PRIVATE
        common
)

if (WIN32)
    target_link_libraries(server PRIVATE ws2_32)
endif()