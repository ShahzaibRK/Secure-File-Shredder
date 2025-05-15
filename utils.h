#pragma once
#include <string>
#include <vector>
#include <windows.h>
#include <sstream> 
#include <iomanip> 

namespace utils {
    std::string generateRandomString(size_t length);
    size_t determineBufferSize();
    std::vector<unsigned char> generateRandomBuffer(size_t bufferSize);
    bool isAdmin();
    std::wstring stringToWString(const std::string& str);
    std::string formatSize(ULONGLONG size);
    void displayProgressBar(ULONGLONG current, ULONGLONG total, double elapsed, double eta);
    void logMessage(const std::string& message);
}