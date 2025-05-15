#include "file_shredder.h"
#include "utils.h"
#include "volume_utils.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

namespace fs = std::filesystem;
using namespace std::chrono;

namespace file_shredder {
    void overwriteFile(const string& filepath, size_t passes, const std::vector<unsigned char>& customPattern) {
        if (!fs::exists(filepath)) {
            throw runtime_error("File does not exist: " + filepath);
        }
        utils::logMessage("Starting to overwrite file: " + filepath + " with " + std::to_string(passes) + " passes.");

        auto filesize = fs::file_size(filepath);
        size_t buffersize = utils::determineBufferSize();
        vector<unsigned char> buffer = utils::generateRandomBuffer(buffersize);

        ofstream file(filepath, ios::binary | ios::in);
        if (!file.is_open()) {
            throw runtime_error("Failed to open file for overwriting: " + filepath);
        }

        cout << "Using a buffer size of " << (buffersize / (1024 * 1024)) << " MB.\n";
        cout << "Overwriting file with random patterns (" << passes << " passes)...\n";

        for (size_t pass = 1; pass <= passes; ++pass) {
            cout << "Pass " << pass << "/" << passes << " in progress...\n";
            utils::logMessage("Pass " + std::to_string(pass) + " of " + std::to_string(passes) + " started.");
            auto start = high_resolution_clock::now();
            ULONGLONG totalBytesWritten = 0;

            // Check and fill the buffer accordingly
            if (!customPattern.empty()) {
                // Fill the buffer with the custom pattern, repeat the pattern if it's shorter than the buffer size
                size_t patternIndex = 0;
                for (size_t i = 0; i < buffer.size(); ++i) {
                    buffer[i] = customPattern[patternIndex];
                    patternIndex = (patternIndex + 1) % customPattern.size(); // Cycle through the custom pattern
                }
            }
            else {
                // Use the existing logic to fill the buffer
                if (pass % 2 == 0) {
                    buffer = utils::generateRandomBuffer(buffersize); // Generate a random buffer
                }
                else {
                    fill(buffer.begin(), buffer.end(), static_cast<unsigned char>(0xFF)); // Fill with 0xFF
                }
            }

            // Writing the buffer to the file
            for (size_t offset = 0; offset < filesize; offset += buffersize) {
                size_t writesize = min(buffersize, filesize - offset);
                file.write(reinterpret_cast<const char*>(buffer.data()), writesize);
                totalBytesWritten += writesize;
                auto now = high_resolution_clock::now();
                double elapsed = duration<double>(now - start).count();
                double eta = (totalBytesWritten > 0) ? (elapsed / totalBytesWritten) * (filesize - totalBytesWritten) : 0.0;

                utils::displayProgressBar(totalBytesWritten, filesize, elapsed, eta);
            }
            file.flush();
            cout << "\nPass " << pass << " completed.\n";
            utils::logMessage("Pass " + std::to_string(pass) + " of " + std::to_string(passes) + " completed.");
        }

        cout << "Final pass: overwriting with zeros...\n";
        utils::logMessage("Final pass: Overwriting " + filepath + " with zeros started.");
        auto start = high_resolution_clock::now();
        ULONGLONG totalBytesWritten = 0;

        for (size_t offset = 0; offset < filesize; offset += buffersize) {
            size_t writesize = min(buffersize, filesize - offset);
            fill(buffer.begin(), buffer.begin() + writesize, 0);
            file.write(reinterpret_cast<const char*>(buffer.data()), writesize);

            totalBytesWritten += writesize;
            auto now = high_resolution_clock::now();
            double elapsed = duration<double>(now - start).count();
            double eta = (totalBytesWritten > 0) ? (elapsed / totalBytesWritten) * (filesize - totalBytesWritten) : 0.0;

            utils::displayProgressBar(totalBytesWritten, filesize, elapsed, eta);
        }
        file.flush();
        cout << "\nFinal pass completed.\n";
        utils::logMessage("Final pass of overwriting " + filepath + " completed.");
        file.close();
        cout << "File successfully overwritten.\n";
        utils::logMessage("File overwrite completed successfully for: " + filepath);
    }

    void securelyDelete(const std::string& filepath, size_t passes, const std::vector<unsigned char>& customPattern) {
        try {
            std::cout << "Preparing to securely delete: " << filepath << std::endl;
            utils::logMessage("Preparing to securely delete file: " + filepath);
            overwriteFile(filepath, passes, customPattern);

            std::string newPath = filepath + "." + utils::generateRandomString(10);
            fs::rename(filepath, newPath);
            utils::logMessage("File renamed for secure deletion: " + filepath + " to " + newPath);

            std::ofstream file(newPath, std::ios::binary | std::ios::trunc);
            file.close();

            std::cout << "Deleting the file...\n";
            if (fs::remove(newPath)) {
                std::cout << "File securely deleted: " << filepath << std::endl;
                utils::logMessage("File securely deleted: " + filepath);
            }
            else {
                throw std::runtime_error("Failed to delete file: " + newPath);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            utils::logMessage("Failed to securely delete file: " + filepath + "; Error: " + e.what());
        }
    }

    void shredFolder(const std::string& folderPath, size_t passes, const std::vector<unsigned char>& customPattern) {
        if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
            std::cerr << "Error: Folder does not exist or is not a directory: " << folderPath << std::endl;
            return;
        }

        utils::logMessage("Starting to shred folder: " + folderPath);

        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (fs::is_directory(entry.path())) {
                shredFolder(entry.path().string(), passes, customPattern);
            }
            else if (fs::is_regular_file(entry.path())) {
                securelyDelete(entry.path().string(), passes, customPattern);
            }
        }

        try {
            fs::remove(folderPath);
            std::cout << "Folder securely shredded: " << folderPath << std::endl;
            utils::logMessage("Folder shredding completed successfully: " + folderPath);
        }
        catch (const std::exception& e) {
            std::cerr << "Error removing folder: " << folderPath << " - " << e.what() << std::endl;
            utils::logMessage("Folder shredding failed: " + folderPath + " with error: " + e.what());
        }
    }

    bool shredPartition(const std::string& partitionPath, size_t passes, const std::vector<unsigned char>& customPattern) {
        if (!utils::isAdmin()) {
            std::cerr << "Error: Administrative privileges required." << std::endl;
            return false;
        }

        std::wstring widePartitionPath = utils::stringToWString(partitionPath);

        // For GetVolumeInformation, we need a path with trailing backslash
        std::wstring volumeInfoPath = widePartitionPath;
        if (volumeInfoPath.back() != L'\\') {
            volumeInfoPath += L'\\';
        }

        // Check if partition is formatted by attempting to get volume information
        wchar_t volumeName[MAX_PATH + 1] = { 0 };
        wchar_t fileSystemName[MAX_PATH + 1] = { 0 };
        DWORD serialNumber = 0;
        DWORD maxComponentLen = 0;
        DWORD fileSystemFlags = 0;

        if (!GetVolumeInformationW(
            volumeInfoPath.c_str(),
            volumeName,
            MAX_PATH + 1,
            &serialNumber,
            &maxComponentLen,
            &fileSystemFlags,
            fileSystemName,
            MAX_PATH + 1
        )) {
            std::cerr << "Error: Cannot shred unformatted partition: " << partitionPath << std::endl
                << "Partition must be formatted with a valid file system." << std::endl;
            return false;
        }

        if (wcslen(fileSystemName) == 0) {
            std::cerr << "Error: No valid file system found on partition: " << partitionPath << std::endl;
            return false;
        }

        // For CreateFile, we need the original path format without trailing backslash
        std::cout << "Attempting to lock and dismount volume..." << std::endl;
        utils::logMessage("Starting to shred partition: " + partitionPath);
        HANDLE hVolume = volume_utils::lockVolume(widePartitionPath);

        if (hVolume == INVALID_HANDLE_VALUE) {
            std::cerr << "Error: Unable to lock volume. Error code: " << GetLastError() << std::endl
                << "Make sure the volume is not in use and you have administrative privileges." << std::endl;
            return false;
        }

        if (!volume_utils::dismountVolume(hVolume)) {
            std::cerr << "Error: Failed to dismount volume. Error code: " << GetLastError() << std::endl;
            CloseHandle(hVolume);
            return false;
        }

        ULONGLONG volumeSize = volume_utils::getVolumeSize(hVolume);
        if (volumeSize == 0) {
            std::cerr << "Error: Unable to determine volume size. Error code: " << GetLastError() << std::endl;
            CloseHandle(hVolume);
            return false;
        }

        const size_t bufferSize = utils::determineBufferSize();
        std::vector<char> buffer(bufferSize);
        std::cout << "Shredding partition: " << partitionPath << std::endl;

        cout << "Using a buffer size of " << (bufferSize / (1024 * 1024)) << " MB.\n";

        std::cout << "Volume size: " << utils::formatSize(volumeSize) << std::endl;

        try {
            // Perform the specified number of passes with random data
            for (int pass = 1; pass <= passes; ++pass) {
                std::cout << "Pass " << pass << "/" << passes << " in progress..." << std::endl;

                if (!customPattern.empty()) {
                    // Use the custom pattern to fill the buffer, repeating it if necessary
                    size_t patternIndex = 0;
                    for (size_t i = 0; i < buffer.size(); ++i) {
                        buffer[i] = customPattern[patternIndex];
                        patternIndex = (patternIndex + 1) % customPattern.size();  // Wrap around the custom pattern
                    }
                }
                else {
                    // Fill the buffer with random data
                    for (size_t i = 0; i < buffer.size(); ++i) {
                        buffer[i] = static_cast<char>(std::rand() % 256);
                    }
                }

                LARGE_INTEGER offset = { 0 };
                SetFilePointerEx(hVolume, offset, nullptr, FILE_BEGIN);
                DWORD bytesWritten = 0;
                ULONGLONG totalBytesWritten = 0;

                auto start = std::chrono::high_resolution_clock::now();
                auto lastUpdate = start;

                while (totalBytesWritten < volumeSize) {
                    size_t writeSize = static_cast<size_t>(
                        std::min<ULONGLONG>(buffer.size(), volumeSize - totalBytesWritten));

                    if (!WriteFile(hVolume, buffer.data(), static_cast<DWORD>(writeSize), &bytesWritten, nullptr)) {
                        DWORD error = GetLastError();
                        throw std::runtime_error("Write failed at offset " + std::to_string(offset.QuadPart) +
                            " (Error Code: " + std::to_string(error) + ")");
                    }

                    if (bytesWritten == 0) {
                        break;
                    }

                    totalBytesWritten += bytesWritten;
                    offset.QuadPart += bytesWritten;

                    // Update progress bar after 10 MB or 1 second
                    auto now = std::chrono::high_resolution_clock::now();
                    double elapsed = std::chrono::duration<double>(now - start).count();
                    double timeSinceLastUpdate = std::chrono::duration<double>(now - lastUpdate).count();

                    if (totalBytesWritten % (10 * 1024 * 1024) == 0 || timeSinceLastUpdate >= 1.0) {
                        double eta = (totalBytesWritten > 0)
                            ? (elapsed / totalBytesWritten) * (volumeSize - totalBytesWritten)
                            : 0.0;

                        std::cout << "\r";
                        utils::displayProgressBar(totalBytesWritten, volumeSize, elapsed, eta);
                        lastUpdate = now;
                    }
                }

                std::cout << "\r";
                utils::displayProgressBar(volumeSize, volumeSize,
                    std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count(),
                    0.0);
                std::cout << std::endl;

                if (!FlushFileBuffers(hVolume)) {
                    throw std::runtime_error("Unable to flush data to disk (Error Code: " +
                        std::to_string(GetLastError()) + ")");
                }

                std::cout << "Pass " << pass << " completed." << std::endl;
                utils::logMessage("Pass " + std::to_string(pass) + " of " + std::to_string(passes) + " completed on partition: " + partitionPath);
            }

            // Final overwrite with zeroes
            std::cout << "Final pass: Overwriting with zeros..." << std::endl;
            std::fill(buffer.begin(), buffer.end(), 0);

            LARGE_INTEGER offset = { 0 };
            SetFilePointerEx(hVolume, offset, nullptr, FILE_BEGIN);
            DWORD bytesWritten = 0;
            ULONGLONG totalBytesWritten = 0;

            auto start = std::chrono::high_resolution_clock::now();
            auto lastUpdate = start;

            while (totalBytesWritten < volumeSize) {
                size_t writeSize = static_cast<size_t>(
                    std::min<ULONGLONG>(buffer.size(), volumeSize - totalBytesWritten));
                if (!WriteFile(hVolume, buffer.data(), static_cast<DWORD>(writeSize), &bytesWritten, nullptr)) {
                    DWORD error = GetLastError();
                    throw std::runtime_error("Write failed at offset " + std::to_string(offset.QuadPart) +
                        " (Error Code: " + std::to_string(error) + ")");
                }

                if (bytesWritten == 0) {
                    break;
                }

                totalBytesWritten += bytesWritten;
                offset.QuadPart += bytesWritten;

                // Update progress bar after 10 MB or 1 second
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - start).count();
                double timeSinceLastUpdate = std::chrono::duration<double>(now - lastUpdate).count();

                if (totalBytesWritten % (10 * 1024 * 1024) == 0 || timeSinceLastUpdate >= 1.0) {
                    double eta = (totalBytesWritten > 0)
                        ? (elapsed / totalBytesWritten) * (volumeSize - totalBytesWritten)
                        : 0.0;

                    std::cout << "\r";
                    utils::displayProgressBar(totalBytesWritten, volumeSize, elapsed, eta);
                    lastUpdate = now;
                }
            }

            std::cout << "\r";
            utils::displayProgressBar(volumeSize, volumeSize,
                std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count(),
                0.0);
            std::cout << std::endl;

            if (!FlushFileBuffers(hVolume)) {
                throw std::runtime_error("Unable to flush data to disk (Error Code: " +
                    std::to_string(GetLastError()) + ")");
            }

            std::cout << "Final pass completed." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            utils::logMessage("Partition shredding failed: " + partitionPath + " with error: " + e.what());
            CloseHandle(hVolume);
            return false;
        }

        std::cout << "Partition " << partitionPath << " shredded successfully." << std::endl;
        utils::logMessage("Partition shredding completed successfully: " + partitionPath);
        CloseHandle(hVolume);
        return true;
    }
}
