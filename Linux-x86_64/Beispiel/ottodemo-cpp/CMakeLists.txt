cmake_minimum_required(VERSION 3.22)

project(ottodemo CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_XCODE_GENERATE_SCHEME ON)

add_executable(ottodemo)

file(GLOB_RECURSE OTTO_DEMO_SRC "${PROJECT_SOURCE_DIR}/ottodemo/*.cpp")

file(GLOB_RECURSE OTTO_DEMO_HEADERS "${PROJECT_SOURCE_DIR}/ottodemo/*.h")


target_sources(ottodemo PRIVATE "${OTTO_DEMO_SRC}" "${OTTO_DEMO_HEADERS}")
target_include_directories(ottodemo PRIVATE "${PROJECT_SOURCE_DIR}/../../include")

target_compile_options(ottodemo PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,XLClang>:-m64>)
target_link_options(ottodemo
    PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,XLClang>:-m64>
        $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-ldl>
        )

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ottodemo)

if(WIN32)
  set(OTTO_LIB_PATH "${PROJECT_SOURCE_DIR}/../../dll")
else()
  set(OTTO_LIB_PATH "${PROJECT_SOURCE_DIR}/../../lib")
endif()
cmake_path(NORMAL_PATH OTTO_LIB_PATH)

if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
# Setzt die LIBPATH-Eigenschaft in der Otto-Demo-Anwendung
  set_target_properties(ottodemo PROPERTIES BUILD_RPATH "${OTTO_LIB_PATH};/usr/lib;/lib")
endif()

set_target_properties(ottodemo PROPERTIES
  VS_DEBUGGER_COMMAND_ARGUMENTS "/c ${PROJECT_SOURCE_DIR}\\test-softidnr-pse.pfx /d   ${OTTO_LIB_PATH}"
  VS_DEBUGGER_WORKING_DIRECTORY  "${PROJECT_SOURCE_DIR}"
)

set_target_properties(ottodemo PROPERTIES
  XCODE_SCHEME_ARGUMENTS "-c ${PROJECT_SOURCE_DIR}/test-softidnr-pse.pfx -d   ${OTTO_LIB_PATH}"
  XCODE_SCHEME_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)