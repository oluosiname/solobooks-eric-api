#pragma once

// Plattformspezifische Makros und Konstanten

#ifdef _WIN32

#   define DYLIB_PREFIX ""
#   define DYLIB_SUFFIX ".dll"
#   define DYLIB_DIR "dll"
#   define PATH_SEPARATOR '\\'
#   define DYLIB_HANDLE HINSTANCE
#   define OPEN_LIBRARY(libPath) ::LoadLibraryExA(libPath, nullptr, 0)
#   define CLOSE_LIBRARY(handle) (::FreeLibrary(handle) == TRUE)
#   define GET_FUNCTION_ADDR(x, y) ::GetProcAddress(x, y)
#   define GETCWD(x, y) _getcwd(x, y)
#   define RESET_ERROR() ::SetLastError(0)
#   define GETLASTERROR() ::GetLastError()

#   define WIN32_LEAN_AND_MEAN
#   include <direct.h>
#   include <windows.h>

#else
#   ifdef __APPLE__
#       include "TargetConditionals.h"
#       ifdef TARGET_OS_MAC
#           define DYLIB_PREFIX "lib"
#           define DYLIB_SUFFIX ".dylib"
#       else
#           error Plattform nicht unterstuetzt
#       endif
#   else
#      define DYLIB_PREFIX "lib"
#      define DYLIB_SUFFIX ".so"
#   endif
#   define DYLIB_DIR "lib"
#   define PATH_SEPARATOR '/'
#   define DYLIB_HANDLE void *
#   define OPEN_LIBRARY(handle) ::dlopen(handle, RTLD_NOW|RTLD_GLOBAL)
#   define CLOSE_LIBRARY(handle) (::dlclose(handle) == 0)
#   define GET_FUNCTION_ADDR(x, y) ::dlsym(x, y)
#   define GETCWD(x, y) getcwd(x, y)
#   define RESET_ERROR() ::dlerror()
#   define GETLASTERROR() errno

#   include <dlfcn.h>
#   include <errno.h>
#   include <unistd.h>

#endif
