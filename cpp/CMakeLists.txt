cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME "BiLiBiLiDanMuCrawling")
set(PROJECT_VERSION "1.0.0")

# 定义工程名称, 并指定语言为C++
project(${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    DESCRIPTION "B站历史弹幕爬虫"
    HOMEPAGE_URL "https://github.com/HengXin666/BiLiBiLi_DanMu_Crawling"
    LANGUAGES CXX
)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译器标志非常重要
# if you use gcc/g++
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
    if (CMAKE_CXX_COMPILER_VERSION MATCHES "12.*")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-maybe-uninitialized")
    endif()
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#
# @todo 因为安装方式不对(?) 我还直接在 cmake 中加了 add_compile_definitions("YLT_ENABLE_SSL")
#
set(YLT_ENABLE_SSL ON)
add_subdirectory(libs/yalantinglibs)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS 
    src/*.cpp
)

include_directories(include)

include_directories(libs/yalantinglibs/include)
include_directories(libs/yalantinglibs/include/ylt/thirdparty)
include_directories(libs/yalantinglibs/include/ylt/standalone)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} yalantinglibs::yalantinglibs)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)