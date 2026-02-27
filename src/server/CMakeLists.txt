add_executable(server)

target_sources(server
    PRIVATE
        server.cpp
)

target_link_libraries(server
    PRIVATE
        common
)