cmake_minimum_required(VERSION 3.16)
project(MultithreadedChatServer C)

set(CMAKE_C_COMPILER clang)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
		src/main.c 
		src/server.c 
		src/logger.c 
)

add_executable(chat_server ${SOURCES})

find_package(Threads REQUIRED)
target_link_libraries(chat_server PRIVATE Threads::Threads)

target_compile_definitions(chat_server PRIVATE _POSIX_C_SOURCE=200809L)
target_compile_options(chat_server PRIVATE -Wall -Wextra -Wpedantic)
