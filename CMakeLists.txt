cmake_minimum_required(VERSION 3.12)
project(converter VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(converter
    src/main.cpp
    src/threadpool.cpp
    src/flac_to_mp3.cpp
    src/ncm_to_mp3.cpp
)

target_include_directories(converter PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(converter
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)
