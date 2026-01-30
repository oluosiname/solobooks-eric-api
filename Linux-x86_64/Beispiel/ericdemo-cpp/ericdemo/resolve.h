#ifndef ERIC_RESOLVE_H
#define ERIC_RESOLVE_H

#include <iostream>
#include "system.h"

// Funktionen zum dynamischen Laden und Entladen von Bibliotheken sowie zum
// Aufloesen von Funktionsymbolen in solchen Bibliotheken.
namespace Resolve
{

    typedef ERIC_RESOLVE_LIB_PTR Library;

    template<typename charT>
    Library inline library(const charT* library_name)
    {
        ERIC_RESOLVE_RESET_ERROR;
        const Library library = ERIC_RESOLVE_LOAD_LIBRARY(library_name);
        return library;
    }


    template<class T>
    T function(Library library, const char* function_name)
    {
        ERIC_RESOLVE_RESET_ERROR;
        return (T)(ERIC_RESOLVE_GET_SYMBOL_FUNCTION (library, function_name));
    }

    bool inline free_library(Library library) {
        ERIC_RESOLVE_RESET_ERROR;
        if (!ERIC_RESOLVE_CLOSE_LIBRARY(library))
        {
            std::cerr << "-> ericapi wurde nicht entladen" << std::endl;
            return false;
        }
        return true;
    }

} // Ende des namespace

#endif

