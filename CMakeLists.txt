cmake_minimum_required(VERSION 3.15)

project("Daisy" VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

set(DAISY_INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
set(DAISY_LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
set(DAISY_BINARY_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}")
set(DAISY_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/cmake/Daisy" CACHE INTERNAL "")

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

add_library(Daisy
        "src/Daisy.cpp"
        "src/ControllerOutput.cpp"
        "src/Assert.cpp"
        "src/Crc32.cpp"
        "src/windows/RAIIHandle.cpp"
        "src/windows/WindowsManager.cpp")

target_compile_features(Daisy PUBLIC cxx_std_17)
target_include_directories(Daisy PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${DAISY_INCLUDE_INSTALL_DIR}>)
target_link_libraries(Daisy PUBLIC hid setupapi cfgmgr32)

set(MSVC_COMPILER_OPTIONS /W4 /WX)
set(CLANG_COMPILER_OPTIONS -Wall -Wextra -Werror)
set(GCC_COMPILER_OPTIONS -Wall -Wextra -Werror)
target_compile_options(Daisy PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:${MSVC_COMPILER_OPTIONS}>
        $<$<CXX_COMPILER_ID:Clang>:${CLANG_COMPILER_OPTIONS}>
        $<$<CXX_COMPILER_ID:GNU>:${GCC_COMPILER_OPTIONS}>)

option(DAISY_BUILD_EXAMPLES "Build Examples" OFF)
if (DAISY_BUILD_EXAMPLES)
    add_subdirectory("examples")
endif ()

if (DAISY_INSTALL)
    include(CMakePackageConfigHelpers)

    install(
            DIRECTORY "${PROJECT_SOURCE_DIR}/include/Daisy"
            DESTINATION "${DAISY_INCLUDE_INSTALL_DIR}"
    )
    install(
            TARGETS Daisy
            EXPORT DaisyTargets
            INCLUDES DESTINATION "${DAISY_INCLUDE_INSTALL_DIR}"
            LIBRARY DESTINATION "${DAISY_LIBRARY_INSTALL_DIR}"
            ARCHIVE DESTINATION "${DAISY_LIBRARY_INSTALL_DIR}"
            RUNTIME DESTINATION "${DAISY_BINARY_INSTALL_DIR}"
    )
    export(
            TARGETS Daisy
            NAMESPACE ds::
            FILE "${DAISY_CONFIG_INSTALL_DIR}/DaisyTargets.cmake"
    )

    write_basic_package_version_file(
            "DaisyVersion.cmake"
            COMPATIBILITY AnyNewerVersion
    )
    install(
            FILES "${CMAKE_CURRENT_BINARY_DIR}/DaisyVersion.cmake" "cmake/DaisyConfig.cmake"
            DESTINATION "${DAISY_CONFIG_INSTALL_DIR}"
    )
    install(
            EXPORT DaisyTargets
            NAMESPACE ds::
            DESTINATION "${DAISY_CONFIG_INSTALL_DIR}"
    )
endif ()