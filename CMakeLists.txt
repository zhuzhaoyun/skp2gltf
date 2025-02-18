cmake_minimum_required(VERSION 3.14)
project(SKP2GLTF)

set(CMAKE_CXX_STANDARD 14)

set(BUILD_USE_64BITS on)
# set(BUILD_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/windows/)

# 添加 CMake 模块路径
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# 包含 SketchUp SDK 配置
include(sketchup-sdk)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/exportgltf
    ${PROJECT_SOURCE_DIR}/include/gltflib
    ${PROJECT_SOURCE_DIR}/include/skp2xml

    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/thirdparty/draco
    ${PROJECT_SOURCE_DIR}/thirdparty/pugixml
    ${PROJECT_SOURCE_DIR}/thirdparty/nlohmann
    ${PROJECT_SOURCE_DIR}/thirdparty/rapidjson
    ${PROJECT_SOURCE_DIR}/thirdparty/tinygltf
    ${PROJECT_SOURCE_DIR}/thirdparty/SketchUpAPI
    ${PROJECT_SOURCE_DIR}/thirdparty/rapidxml
    ${PROJECT_SOURCE_DIR}/thirdparty/tinyxml2
    ${PROJECT_SOURCE_DIR}/thirdparty/xml2json
)

# 添加子目录
add_subdirectory(src/gltflib)
add_subdirectory(src/exportgltf)
add_subdirectory(src/skp2xml)
add_subdirectory(src/tinyxml2)

# 添加可执行文件
add_executable(skp2gltf src/main.cpp)

# 根据编译器类型设置链接器选项
if(MSVC)
    # MSVC 编译器不需要这些链接选项
    add_compile_options(/utf-8)  # 设置 MSVC 编译器使用 UTF-8 编码
    target_link_options(skp2gltf PRIVATE)
else()
    # 对于 GCC/Clang 编译器保留原有选项
    target_link_options(skp2gltf PRIVATE 
        "-Wl,--start-group" 
        "-Wl,--end-group"
    )
endif()

target_link_libraries(skp2gltf PRIVATE
    gltflib
    exportgltflib
    skp2xml
    tinyxml2
    ${CMAKE_SOURCE_DIR}/lib/SketchUpAPI.lib
    ${CMAKE_SOURCE_DIR}/lib/sketchup.lib
    ${CMAKE_SOURCE_DIR}/lib/draco.lib
)

# 复制运行时依赖的 SKP_DLL 到输出目录
add_custom_command(TARGET skp2gltf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/lib/SketchUpAPI.dll"
        "${CMAKE_SOURCE_DIR}/lib/SketchUpCommonPreferences.dll"
        "$<TARGET_FILE_DIR:skp2gltf>"
)

