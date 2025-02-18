if (NOT TARGET SketchUpAPI)
    set(SKETCHUP_SDK_DIR "${CMAKE_CURRENT_BINARY_DIR}/sketchup_sdk")

    find_program(UNZIP_EXECUTABLE NAMES unzip)
    if (NOT UNZIP_EXECUTABLE)
        set(UNZIP_EXECUTABLE "${BUILD_BIN_DIR}/unzip")
    endif()

    if(NOT EXISTS "${SKETCHUP_SDK_DIR}/headers")
        make_directory(${SKETCHUP_SDK_DIR})
        execute_process(
                COMMAND "${UNZIP_EXECUTABLE}" -o "${CMAKE_CURRENT_SOURCE_DIR}/sdk/SDK_WIN_x64_2021-0-339.zip"
                WORKING_DIRECTORY ${SKETCHUP_SDK_DIR}
        )
    endif()

    add_library(SketchUpCommonPreferences SHARED IMPORTED)
    set_target_properties(
            SketchUpCommonPreferences
            PROPERTIES
            IMPORTED_LOCATION ${SKETCHUP_SDK_DIR}/binaries/sketchup/x64/SketchUpCommonPreferences.dll
    )

    function(add_skp_library lib_name dll_file lib_file)
        add_library(${lib_name} SHARED IMPORTED)
        target_include_directories(
                ${lib_name}
                INTERFACE
                ${SKETCHUP_SDK_DIR}/headers
        )
        set_target_properties(
                ${lib_name}
                PROPERTIES
                IMPORTED_IMPLIB
                ${lib_file}
                IMPORTED_LOCATION
                ${dll_file}
        )
    endfunction()

    add_skp_library(LayOutAPI ${SKETCHUP_SDK_DIR}/binaries/layout/x64/LayOutAPI.dll ${SKETCHUP_SDK_DIR}/binaries/layout/x64/LayOutAPI.lib)
    add_skp_library(SketchUpViewerAPI ${SKETCHUP_SDK_DIR}/binaries/layout/x64/SketchUpViewerAPI.dll ${SKETCHUP_SDK_DIR}/binaries/layout/x64/SketchUpViewerAPI.lib)
    add_skp_library(SketchUpAPI ${SKETCHUP_SDK_DIR}/binaries/sketchup/x64/SketchUpAPI.dll ${SKETCHUP_SDK_DIR}/binaries/sketchup/x64/SketchUpAPI.lib)

    install(DIRECTORY
            ${SKETCHUP_SDK_DIR}/binaries/sketchup/x64/
            ${SKETCHUP_SDK_DIR}/binaries/layout/x64/
            DESTINATION "bin"
            FILES_MATCHING PATTERN "*.dll")
endif()
