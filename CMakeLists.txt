cmake_minimum_required(VERSION 3.21)

project(bilibilicardbrowser VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZLIB REQUIRED IMPORTED_TARGET zlib)
pkg_check_modules(BROTLI REQUIRED IMPORTED_TARGET libbrotlicommon libbrotlidec)

include(FetchContent)
FetchContent_Declare(
    json
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    URL      https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa
)
FetchContent_MakeAvailable(json)

# Enable AUTO_MOC AUTO_UIC AUTO_RCC
qt_standard_project_setup()
qt_add_executable(bilibilicardbrowser)

set_target_properties(bilibilicardbrowser PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WIN32_EXECUTABLE $<IF:$<CONFIG:Release>,ON,OFF>
    DEBUG_POSTFIX d
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
execute_process(
    COMMAND date --iso-8601=seconds
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE COMPILE_TIME
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if (GIT_COMMIT_HASH STREQUAL "")
    message(WARNING "Could not determine git commit hash")
    set(GIT_COMMIT_HASH "Unknown")
endif ()

if (COMPILE_TIME STREQUAL "")
    set(COMPILE_TIME "Unknown")
endif ()

target_compile_definitions(bilibilicardbrowser PRIVATE
    # see: https://cmake.org/cmake/help/latest/policy/CMP0043.html#policy:CMP0043
    $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
    GIT_COMMIT_HASH="${GIT_COMMIT_HASH}"
    COMPILE_TIME="${COMPILE_TIME}"
    APPLICATION_VERSION="${PROJECT_VERSION}"
)

target_compile_options(bilibilicardbrowser PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_sources(bilibilicardbrowser PRIVATE
    bilibili_request_manager.hh
    compress_helper.hh
    my_decompose.hh
    asset_bag.hh
    main_window.hh
)

target_sources(bilibilicardbrowser PRIVATE
    main.cc
    bilibili_request_manager.cc
    compress_helper.cc
    my_decompose.cc
    asset_bag.cc
    main_window.cc
)

target_link_libraries(bilibilicardbrowser PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    PkgConfig::ZLIB
    PkgConfig::BROTLI
    nlohmann_json::nlohmann_json
)

install(TARGETS bilibilicardbrowser
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET bilibilicardbrowser
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_TOOL_OPTIONS
        "--exclude-plugins qpdf,qsvg"
        "--translations en,zh_CN"
)
install(SCRIPT ${deploy_script})