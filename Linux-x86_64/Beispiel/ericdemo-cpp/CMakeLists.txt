cmake_minimum_required(VERSION 3.22)

project(ericdemo CXX)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "XL")
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(CMAKE_XCODE_GENERATE_SCHEME ON)

add_executable(ericdemo)

file(GLOB_RECURSE ERIC_DEMO_SRC "${PROJECT_SOURCE_DIR}/ericdemo/*.cpp")

file(GLOB_RECURSE ERIC_DEMO_HEADERS "${PROJECT_SOURCE_DIR}/ericdemo/*.h")

target_sources(ericdemo PRIVATE "${ERIC_DEMO_SRC}" "${ERIC_DEMO_HEADERS}")
target_include_directories(ericdemo PRIVATE "${PROJECT_SOURCE_DIR}/../../include")

target_compile_options(ericdemo
    PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,XL>:-qlanglvl=extended0x>
        $<$<COMPILE_LANG_AND_ID:CXX,XL>:-qchars=signed>
        $<$<COMPILE_LANG_AND_ID:CXX,XL>:-q64>
        $<$<COMPILE_LANG_AND_ID:CXX,XLClang>:-m64>
        )
target_link_options(ericdemo
    PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,XLClang>:-m64>
        $<$<COMPILE_LANG_AND_ID:CXX,XL>:-q64>
        $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-ldl>
        )


if(WIN32)
  set(ERIC_LIB_PATH "${PROJECT_SOURCE_DIR}/../../dll")
  target_link_libraries(ericdemo PRIVATE Shlwapi)
else()
  set(ERIC_LIB_PATH "${PROJECT_SOURCE_DIR}/../../lib")
endif()
cmake_path(NORMAL_PATH ERIC_LIB_PATH)

if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
# Setzt die LIBPATH-Eigenschaft in der ERiC-Demo-Anwendung
  set_target_properties(ericdemo PROPERTIES BUILD_RPATH "${ERIC_LIB_PATH};/usr/lib;/lib")
endif()


set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ericdemo)
set_target_properties(ericdemo PROPERTIES
  VS_DEBUGGER_COMMAND_ARGUMENTS "/x ${PROJECT_SOURCE_DIR}\\ESt_2020.xml /c ${PROJECT_SOURCE_DIR}\\test-softidnr-pse.pfx /d  ${ERIC_LIB_PATH}"
  VS_DEBUGGER_WORKING_DIRECTORY  "${PROJECT_SOURCE_DIR}"
)

set_target_properties(ericdemo PROPERTIES
  XCODE_SCHEME_ARGUMENTS "-x ${PROJECT_SOURCE_DIR}/ESt_2020.xml -c ${PROJECT_SOURCE_DIR}/test-softidnr-pse.pfx -d  ${ERIC_LIB_PATH}"
  XCODE_SCHEME_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)