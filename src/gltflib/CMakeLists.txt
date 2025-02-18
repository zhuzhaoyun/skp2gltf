add_library(gltflib 
    ${CMAKE_CURRENT_SOURCE_DIR}/creategltfcommon.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/parsegltfcommon.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/parsemeshdata.cpp
    ${PROJECT_SOURCE_DIR}/include/gltflib/gltfdraco.h
    ${PROJECT_SOURCE_DIR}/include/gltflib/parsemeshdata.h
)

target_include_directories(gltflib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)