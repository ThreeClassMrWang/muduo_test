//
// Created by wcj on 17-12-13.
//

#ifndef MUDUO_TEST_FILEUTIL_H
#define MUDUO_TEST_FILEUTIL_H

#include <muduo/base/Types.h>

class FileUtil {
public:
    static muduo::string readFile(const char* filename);
    static bool checkFileRead(const char* filename);
};

#endif //MUDUO_TEST_FILEUTIL_H
