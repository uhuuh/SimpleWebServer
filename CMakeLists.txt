cmake_minimum_required(VERSION 3.15.0)

project(study_muduo)
set(CMAKE_CXX_STANDARD 20)

include_directories(./include)
link_libraries(pthread)

include_directories(./lib/easyloggingpp/include)
file(GLOB files_lib "lib/**/**/*.cpp")
message("[debug] files_lib: ${files_lib}")

file(GLOB files "src/**/*.cpp")
message("[deubg] files: ${files}")
add_executable(test_Eventloop test/test_Eventloop.cpp ${files} ${files_lib})
add_executable(test_ThreadEventloop test/test_ThreadEventloop.cpp ${files} ${files_lib})
add_executable(test_TcpServer test/test_TcpServer.cpp ${files} ${files_lib})
add_executable(test_TcpClient test/test_TcpClient.cpp ${files} ${files_lib})
add_executable(test_logger test/test_logger.cpp ${files} ${files_lib})

get_property(include_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(include_dir ${include_dirs})
    message("[debug] Include directory: ${include_dir}")
endforeach()

add_definitions(-DELPP_THREAD_SAFE) # easyloggingpp需要, 多线程添加支持


