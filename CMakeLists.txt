cmake_minimum_required(VERSION 3.23)
project(EchouSocket)

set(CMAKE_CXX_STANDARD 20)

include_directories(/usr/local/include/uWebSockets
        /home/shakya/CLionProjects/ThirdParty_lib/uWebSockets/uSockets/src)

link_directories(/home/shakya/CLionProjects/ThirdParty_lib/uWebSockets/uSockets)

add_executable(EchouSocket main.cpp)

target_link_libraries(EchouSocket uSockets pthread z)

