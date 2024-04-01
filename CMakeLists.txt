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
# add_executable(test_loop test/test_loop.cpp ${files} ${files_lib})
# add_executable(test_server test/test_server.cpp ${files} ${files_lib})
add_executable(test_client test/test_client.cpp ${files} ${files_lib})

get_property(include_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(include_dir ${include_dirs})
    message("[debug] Include directory: ${include_dir}")
endforeach()

add_definitions(-DELPP_THREAD_SAFE) # easyloggingpp需要, 多线程添加支持


