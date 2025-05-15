#pragma once
#include <string>
#include <cstddef>
#include <chrono>
#include <vector> 

// file_shredder.h
namespace file_shredder {
    void overwriteFile(const std::string& filepath, size_t passes, const std::vector<unsigned char>& customPattern = {});
    void securelyDelete(const std::string& filepath, size_t passes, const std::vector<unsigned char>& customPattern = {});
    void shredFolder(const std::string& folderPath, size_t passes, const std::vector<unsigned char>& customPattern = {});
    bool shredPartition(const std::string& partitionPath, size_t passes, const std::vector<unsigned char>& customPattern = {});
}
