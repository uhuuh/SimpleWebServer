cmake_minimum_required(VERSION 3.15.0)

project(SimpleServer)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 生成 compile_commands.json 文件，方便 vscode clangd 插件提供语言服务器服务

function(dir_find_src dir_name output_var)
    file(GLOB_RECURSE cpp_files "${dir_name}/*.cpp")
    message("in dir ${dir_name}")
    foreach(cpp_file ${cpp_files})
        message("-> found src file: ${cpp_file}")
    endforeach()
    message("")

    set(${output_var} ${cpp_files} PARENT_SCOPE)
endfunction()

function(dir_find_include base_dir)
    message("in dir ${base_dir}")
    file(GLOB_RECURSE subdirs LIST_DIRECTORIES true ${base_dir}/*)
    foreach(subdir ${subdirs})
        if(IS_DIRECTORY ${subdir})
            include_directories(${subdir})
            message("-> found header dir: ${subdir}")
        endif()
    endforeach()
    message("")
endfunction()

# 将src目录下的所有模块目录保存到变量中
dir_find_include(src)

# 将指定模块目录下的cpp文件保存到变量中
dir_find_src(src/reactor reactor_src)
dir_find_src(src/tcp tcp_src)
dir_find_src(src/base base_src)
dir_find_src(src/logger logger_src)
dir_find_src(src/http http_src)

link_libraries(pthread)

add_executable(test_logger test/logger/test.cpp ${logger_src})
add_executable(test_reactor test/reactor/test.cpp ${reactor_src} ${base_src} ${logger_src})
add_executable(test_tcp test/tcp/test.cpp ${tcp_src} ${reactor_src} ${base_src} ${logger_src})
add_executable(test_http test/http/test.cpp ${http_src})




