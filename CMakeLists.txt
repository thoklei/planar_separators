cmake_minimum_required(VERSION 3.19)
project(benchmark CXX)

set(CMAKE_CXX_STANDARD 17)

set(OGDF_DIR "/Users/thomas/Uni/Masterarbeit/OGDF/cmake-build-release")
find_package(OGDF REQUIRED PATHS "/Users/thomas/Uni/Masterarbeit/OGDF/cmake-build-release" NO_DEFAULT_PATH)

add_executable(main src/main.cpp)
target_include_directories(main PRIVATE ${OGDF_INCLUDE_DIRS})
target_link_libraries(main OGDF)

add_executable(tri src/triangulation.cpp)
target_include_directories(tri PRIVATE ${OGDF_INCLUDE_DIRS})
target_link_libraries(tri OGDF)

add_executable(instance_gen src/instance_generator.cpp)
target_include_directories(instance_gen PRIVATE ${OGDF_INCLUDE_DIRS})
target_link_libraries(instance_gen OGDF)