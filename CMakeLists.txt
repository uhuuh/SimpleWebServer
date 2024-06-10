cmake_minimum_required(VERSION 3.15.0)

project(study_muduo)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


function(dir_find_src dir_name output_var)
    file(GLOB_RECURSE cpp_files "${dir_name}/*.cpp")
    message("in dir ${dir_name}")
    set(filtered_cpp_files "")
    foreach(cpp_file ${cpp_files})
        # 检查文件名是否包含 "test"
        string(FIND ${cpp_file} "test" test_index)
        if(${test_index} EQUAL -1)
            list(APPEND filtered_cpp_files ${cpp_file})
            message("-> found src file: ${cpp_file}")
        endif()
    endforeach()
    message("\n")

    set(${output_var} ${filtered_cpp_files} PARENT_SCOPE)
endfunction()

function(dir_find_include base_dir)
    message("in dir ${base_dir}")
    file(GLOB_RECURSE subdirs LIST_DIRECTORIES true ${base_dir}/*)
    foreach(subdir ${subdirs})
        if(IS_DIRECTORY ${subdir})
            include_directories(${subdir})
            message("-> found header file: ${subdir}")
        endif()
    endforeach()
    message("\n")
endfunction()

dir_find_include(src)

dir_find_src(src/reactor reactor_src)
dir_find_src(src/tcp tcp_src)
dir_find_src(src/base base_src)
dir_find_src(src/logger logger_src)

link_libraries(pthread)

add_executable(test_reactor test/reactor/test.cpp ${reactor_src} ${base_src})
add_executable(test_tcp test/tcp/test.cpp ${tcp_src} ${reactor_src} ${base_src})
add_executable(test_logger test/logger/test.cpp ${logger_src})




