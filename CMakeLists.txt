cmake_minimum_required(VERSION 3.21)

project(bilibilicardbrowser VERSION 1.1.1 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
add_subdirectory(3rdparty)

find_package(ZLIB REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(BROTLI REQUIRED IMPORTED_TARGET libbrotlicommon libbrotlidec)

# Enable AUTO_MOC AUTO_UIC AUTO_RCC
qt_standard_project_setup()
qt_add_executable(bilibilicardbrowser)

set_target_properties(bilibilicardbrowser PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WIN32_EXECUTABLE $<IF:$<CONFIG:Release>,ON,OFF>
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

execute_process(
    COMMAND git rev-parse --short --verify HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if (GIT_COMMIT_HASH STREQUAL "")
    execute_process(
        COMMAND cat commit
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if (GIT_COMMIT_HASH STREQUAL "")
        message(WARNING "Could not determine git commit hash")
        set(GIT_COMMIT_HASH "Unknown")
    endif ()
endif ()

target_compile_definitions(bilibilicardbrowser PRIVATE
    # see: https://cmake.org/cmake/help/latest/policy/CMP0043.html#policy:CMP0043
    GIT_COMMIT_HASH="${GIT_COMMIT_HASH}"
    APPLICATION_VERSION="${PROJECT_VERSION}"
)

target_sources(bilibilicardbrowser PRIVATE
    src/json_helper.hh
    src/bilibili_request_manager.hh
    src/compress_helper.hh
    src/my_decompose.hh
    src/asset_bag.hh
    src/collection_export_worker.hh
    src/main_window.hh
)

target_sources(bilibilicardbrowser PRIVATE
    src/main.cc
    src/bilibili_request_manager.cc
    src/compress_helper.cc
    src/my_decompose.cc
    src/asset_bag.cc
    src/collection_export_worker.cc
    src/main_window.cc
)

target_link_libraries(bilibilicardbrowser PRIVATE
    Qt6::Widgets
    Qt6::Network
    ZLIB::ZLIB
    PkgConfig::BROTLI
    nlohmann_json::nlohmann_json
)

install(TARGETS bilibilicardbrowser
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET bilibilicardbrowser
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_TOOL_OPTIONS
        "--no-system-d3d-compiler"
        "--exclude-plugins qpdf,qsvg"
        "--translations en,zh_CN"
)
install(SCRIPT ${deploy_script})
