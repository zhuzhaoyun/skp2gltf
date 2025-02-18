add_library(tinyxml2 tinyxml2.cpp)

target_include_directories(tinyxml2 PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)