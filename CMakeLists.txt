cmake_minimum_required(VERSION 3.15)

project(rcon-server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENDSTONE_API_VERSION "0.11" CACHE STRING "Endstone API version")

add_compile_options(-include cstdlib)

include(FetchContent)
FetchContent_Declare(
    endstone
    GIT_REPOSITORY https://github.com/EndstoneMC/endstone.git
    GIT_TAG        v${ENDSTONE_API_VERSION}
)
FetchContent_MakeAvailable(endstone)

endstone_add_plugin(${PROJECT_NAME}
    src/plugin.cpp
    src/rcon/packet.cpp
    src/rcon/session.cpp
    src/rcon/server.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()
