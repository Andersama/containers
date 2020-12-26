# CMakeList.txt : CMake project for containers, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (containers "containers.cpp"  "plain_array.h" "nanobench.cpp" "nanobench.h" )
set_property(TARGET containers PROPERTY CXX_STANDARD 20)

# TODO: Add tests and install targets if needed.
