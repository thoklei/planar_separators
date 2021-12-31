cmake_minimum_required(VERSION 3.13)
project(benchmark CXX)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
    set(CMAKE_CXX_STANDARD 17)

    # OGDF
    set(OGDF_DIR "/Users/thomas/Uni/Masterarbeit/OGDF/cmake-build-release")
    find_package(OGDF REQUIRED PATHS "/Users/thomas/Uni/Masterarbeit/OGDF/cmake-build-release" NO_DEFAULT_PATH)

    # CGAL
    find_package(CGAL REQUIRED PATHS CGAL_DIR)
    set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE true)

    # TINY_XML
    set(tinyxml2_DIR "/Users/thomas/Uni/Masterarbeit/tinyxml2/build")
    find_package(tinyxml2 REQUIRED PATHS "/Users/thomas/Uni/Masterarbeit/tinyxml2/cmake")

else()
    set(CMAKE_CXX_COMPILER_ID "GCC")
    set(CMAKE_CXX_COMPILER_VERSION "8.3.0")

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # OGDF
    set(OGDF_DIR "/home/tklein/master_thesis/OGDF/")
    find_package(OGDF REQUIRED PATHS "/home/tklein/master_thesis/OGDF/")
endif()

add_executable(main src/main.cpp src/property_recorder.cpp src/tinyxml2.cpp src/utils.cpp)
target_include_directories(main PUBLIC include)
target_include_directories(main PRIVATE ${OGDF_INCLUDE_DIRS})
target_link_libraries(main OGDF)
target_compile_options(main PUBLIC -Wall)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_features(main PRIVATE cxx_std_17)
    target_link_libraries(main stdc++fs)
endif()

add_executable(instance_gen src/instance_generator.cpp src/utils.cpp)
target_include_directories(instance_gen PUBLIC include)
target_include_directories(instance_gen PRIVATE ${CGAL_INCLUDE_DIRS} ${OGDF_INCLUDE_DIRS})
target_link_libraries(instance_gen CGAL OGDF)

add_executable(workbench src/workbench.cpp)
target_include_directories(workbench PUBLIC include)
target_include_directories(workbench PRIVATE ${OGDF_INCLUDE_DIRS})
target_link_libraries(workbench OGDF)

add_executable(properties src/record_properties.cpp src/property_recorder.cpp src/tinyxml2.cpp src/utils.cpp)
target_include_directories(properties PUBLIC include)
target_include_directories(properties PRIVATE ${OGDF_INCLUDE_DIRS})
target_include_directories(properties PRIVATE ${tinyxml2_DIR})
target_link_libraries(properties OGDF)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_features(properties PRIVATE cxx_std_17)
    target_link_libraries(properties stdc++fs)
endif()