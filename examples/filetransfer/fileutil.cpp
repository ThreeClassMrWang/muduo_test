//
// Created by wcj on 17-12-13.
//

#include <fstream>
#include <exception>
#include <unistd.h>
#include "fileutil.h"

muduo::string FileUtil::readFile(const char* filename) {
    std::ifstream is{filename, std::ios::binary | std::ios::ate};
    if (!is)
        throw std::runtime_error("file open error");

    auto size = is.tellg();
    muduo::string file(size, '\0');
    is.seekg(0);
    if (!is.read(&file[0], size))
        throw std::runtime_error("read file error");
    if (!is.good())
        throw std::runtime_error("read file error");

    return std::move(file);
}

bool FileUtil::checkFileRead(const char *filename) {
    return (::access(filename, R_OK | F_OK) == 0);
}