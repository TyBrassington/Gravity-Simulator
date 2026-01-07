cmake_minimum_required(VERSION 3.15)
project(gravity_simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML COMPONENTS Graphics Window System REQUIRED)

add_executable(gravity_simulator
    src/main.cpp
    src/math.cpp
)


target_include_directories(gravity_simulator PRIVATE include)

target_link_libraries(gravity_simulator
    PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
)
