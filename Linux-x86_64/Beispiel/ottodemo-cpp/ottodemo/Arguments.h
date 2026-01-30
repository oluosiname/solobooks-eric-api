#pragma once

#include "System.h"
#include <string>

struct Arguments {
    std::string appPath;
    std::string demoDirPath;
    std::string ottoDirPath;
    std::string logDirPath;
    std::string inFilePath;
    std::string objectId;
    std::string outFilePath;
    std::string certPath;
    std::string certPin;
    std::string herstellerId;

    bool        showHelp;
    bool        sendData;
    bool        fetchData;

    bool        parseOk;

    Arguments();

    void print() const;
    void help() const;

    const char* getPin() const;
    const char* getCertPath() const;
};

const Arguments parseArguments(int argc, char *argv[]);
