cmake_minimum_required(VERSION 3.5)
project(uxp1a)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 11)

add_executable(uxp1a main.cpp tuple/Tuple.cpp tuple/Tuple.h tuple/TupleElementType.h tuple/TupleElement.cpp tuple/TupleElement.h protocol/control_data.cpp protocol/control_data.h linda/linda.h architecture/Proces.cpp architecture/Proces.h thread/Thread.cpp thread/Thread.h thread/SynchronizedQueue.h thread/monitor.h linda/linda.cpp parser/Element.cpp parser/Parser.cpp parser/Element.h parser/Parser.h parser/Requirement.h parser/check_fun.h)

target_link_libraries(uxp1a Threads::Threads)