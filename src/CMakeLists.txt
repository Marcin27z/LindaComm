cmake_minimum_required(VERSION 3.5)
project(uxp1a)

set(CMAKE_CXX_STANDARD 11)

add_executable(uxp1a main.cpp tuple/Tuple.cpp tuple/Tuple.h tuple/TupleElementType.h tuple/TupleElement.cpp tuple/TupleElement.h)