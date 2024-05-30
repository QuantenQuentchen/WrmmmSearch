# CMakeList.txt : CMake project for wrmmmSearch, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_TOOLCHAIN_FILE "D:/Proggo/IDE/vcpkg/scripts/buildsystems/vcpkg.cmake")

project ("wrmmmSearch")

file(GLOB SOURCES "src/*.cpp")

add_executable (wrmmmSearch ${SOURCES})

set_target_properties(wrmmmSearch PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

find_path(SQLITE_MODERN_CPP_INCLUDE_DIRS "sqlite_modern_cpp.h")
target_include_directories(wrmmmSearch PRIVATE ${SQLITE_MODERN_CPP_INCLUDE_DIRS})

find_package(unofficial-sqlite3 CONFIG REQUIRED)
target_link_libraries(wrmmmSearch PRIVATE unofficial::sqlite3::sqlite3)

#find_package(PkgConfig REQUIRED)

#pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

file(GLOB UI_FILES ${CMAKE_SOURCE_DIR}/ui/*)
add_custom_target(
        copy_ui_files ALL
        COMMENT "Copying UI files"
)
foreach(FILE ${UI_FILES})
    get_filename_component(FILE_NAME ${FILE} NAME)
    add_custom_command(
            TARGET copy_ui_files
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FILE}
            ${CMAKE_BINARY_DIR}/${FILE_NAME}
    )
endforeach()

add_dependencies(wrmmmSearch copy_ui_files)

find_package(PkgConfig REQUIRED)

if(PKG_CONFIG_FOUND)
    message(STATUS "pkg-config found at ${PKG_CONFIG_EXECUTABLE}")
    message(STATUS "pkg-config version: ${PKG_CONFIG_VERSION_STRING}")
else()
    message(FATAL_ERROR "pkg-config not found")
endif()

pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(GTKMM4 REQUIRED gtkmm-4.0)

include_directories(${GTK4_INCLUDE_DIRS} ${GTKMM4_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS} ${GTKMM4_LIBRARY_DIRS})
target_link_libraries(wrmmmSearch PRIVATE ${GTK4_LIBRARIES} ${GTKMM4_LIBRARIES})

add_compile_options(${GTK4_CFLAGS} ${GTKMM4_CFLAGS})

#string(STRIP ${GTK4_LDFLAGS} GTK4_LDFLAGS)
#string(STRIP ${GTKMM4_LDFLAGS} GTKMM4_LDFLAGS)
#target_link_libraries(wrmmmSearch ${GTK4_LDFLAGS} ${GTKMM4_LDFLAGS})


set_property(TARGET wrmmmSearch PROPERTY CXX_STANDARD 20)


# TODO: Add tests and install targets if needed.
