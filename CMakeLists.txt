cmake_minimum_required(VERSION 3.15)
project(3D_renderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML COMPONENTS Graphics Window System REQUIRED)

add_executable(3D_renderer
    src/main.cpp
    src/math.cpp
    src/obj_loader.cpp
)


target_include_directories(3D_renderer PRIVATE include)

target_link_libraries(3D_renderer
    PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
)
