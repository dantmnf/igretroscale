cmake_minimum_required(VERSION 3.20)
project(igretroscale CXX)

add_executable(igretroscale main.cpp cApiWrapper.cpp)

# Intel, why are you exporting API functions in consumer code?
target_compile_definitions(igretroscale PRIVATE CTL_APIEXPORT=)
