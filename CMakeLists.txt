cmake_minimum_required(VERSION 2.8)
project(OpenCVProject CXX)
find_package(OpenCV REQUIRED)

add_subdirectory(lib/docopt.cpp)
file(GLOB library_headers lib/*/*.h)
file(GLOB library_source lib/*/docopt.cpp)

file(GLOB project_headers src/*.hpp)
file(GLOB project_source  src/*.cpp)
add_executable(composer ${project_source} ${project_headers} ${library_source} ${library_headers})

target_compile_features(composer PRIVATE cxx_range_for)
target_compile_features(composer PRIVATE cxx_auto_type)

target_link_libraries(composer ${OpenCV_LIBS})
