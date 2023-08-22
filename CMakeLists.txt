cmake_minimum_required (VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

add_definitions("-DUNICODE" "-D_UNICODE")

project(VulkanRepo)

aux_source_directory(. SOURCE_ALL)
aux_source_directory(./Core SOURCE_ALL)
aux_source_directory(./Runtime SOURCE_ALL)
aux_source_directory(./Runtime/Graphics SOURCE_ALL)
aux_source_directory(./Runtime/Renderer SOURCE_ALL)
aux_source_directory(./Utils SOURCE_ALL)
aux_source_directory(./VK SOURCE_ALL)

add_executable(VulkanRepo ${SOURCE_ALL})

include_directories(./)
include_directories(./Third/Include)
include_directories(./Third/Include/assimp/include)

# _debug的库太大，仓库里面没有传，需要的话自己编
# 目前缺少zlibstaticd.lib assimp-vc143-mtd.lib
# 放到Third/Lib下面即可
find_library(glfw_lib NAMES glfw3.lib PATHS ./Third/Lib)
find_library(vulkan_lib NAMES vulkan-1.lib PATHS ./Third/Lib)
find_library(assimp_lib_debug NAMES assimp-vc143-mtd.lib PATHS ./Third/Lib)
find_library(zlib_lib_debug NAMES zlibstaticd.lib PATHS ./Third/Lib)
find_library(assimp_lib_release NAMES assimp-vc143-mt.lib PATHS ./Third/Lib)
find_library(zlib_lib_release NAMES zlibstatic.lib PATHS ./Third/Lib)

target_link_libraries(${PROJECT_NAME} ${glfw_lib})
target_link_libraries(${PROJECT_NAME} ${vulkan_lib})
target_link_libraries(${PROJECT_NAME} debug ${assimp_lib_debug} optimized ${assimp_lib_release})
target_link_libraries(${PROJECT_NAME} debug ${zlib_lib_debug} optimized ${zlib_lib_release})
