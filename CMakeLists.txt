cmake_minimum_required(VERSION 3.15.0)

project(SimpleServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 生成compile_commands.json文件，方便vscode clangd插件提供语言服务
enable_language(ASM) # 使得add_executable支持汇编文件，避免找不到co_swap符号

# set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_BUILD_TYPE "Release")

# 为 Debug 和 Release 模式设置不同的编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Configuring Debug build")

    # 0g专门为调试优化设计的选项，提供比O0更好的调试体验
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Og")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address") # 内存相关问题检测
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Configuring Release build")

    # NDEBUG 选项使C的assert失效
    # todo 生成静态库
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()


function(dir_find_src dir_name output_var)
    # file(GLOB_RECURSE cpp_files "${dir_name}/*.cpp")
    file(GLOB_RECURSE cpp_files
    "${dir_name}/*.cpp"
    "${dir_name}/*.s"
    )
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

# 将src目录下的所有模块目录添加到include目录
dir_find_include(src)

# 将指定模块目录下的cpp文件保存到变量中
dir_find_src(src/base base_src)
dir_find_src(src/logger logger_src)
dir_find_src(src/timer/ timer_src)
dir_find_src(src/reactor reactor_src)
dir_find_src(src/co co_src)
dir_find_src(src/http http_src)
dir_find_src(src/tcp tcp_src)

link_libraries(pthread)

add_executable(test_logger test/logger/test.cpp ${logger_src})
add_executable(test_timer test/timer/test.cpp ${timer_src} ${reactor_src} ${base_src} ${logger_src})
add_executable(test_reactor test/reactor/test.cpp ${reactor_src} ${timer_src} ${base_src} ${logger_src})
add_executable(test_co test/co/test.cpp ${co_src} ${reactor_src} ${timer_src} ${base_src} ${logger_src})
add_executable(test_tcp test/tcp/test.cpp ${tcp_src} ${reactor_src} ${co_src} ${timer_src} ${base_src} ${logger_src})
add_executable(test_http test/http/test.cpp ${http_src} ${tcp_src} ${reactor_src} ${co_src} ${timer_src} ${base_src} ${logger_src})






