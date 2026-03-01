add_executable(client)

target_sources(client
    PRIVATE
        client.cpp
)

target_compile_features(client PRIVATE cxx_std_20)

target_link_libraries(client
    PRIVATE
        common
)

if (WIN32)
    target_link_libraries(client PRIVATE ws2_32)
endif()