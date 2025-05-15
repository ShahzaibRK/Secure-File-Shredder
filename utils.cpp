#include "utils.h"
#include <random>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <fstream>

namespace utils {
    std::string generateRandomString(size_t length) {
        const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::string result;
        result.reserve(length);
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, sizeof(charset) - 2);
        for (size_t i = 0; i < length; i++) {
            result += charset[dist(rng)];
        }
        return result;
    }

    size_t determineBufferSize() {
        return 64 * 1024 * 1024;
    }

    std::vector<unsigned char> generateRandomBuffer(size_t bufferSize) {
        std::vector<unsigned char> buffer(bufferSize);
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<unsigned int> dist(0, 255);
        std::generate(buffer.begin(), buffer.end(), [&]() { return static_cast<unsigned char>(dist(rng)); });
        return buffer;
    }

    bool isAdmin() {
        BOOL isElevated = FALSE;
        HANDLE hToken = NULL;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            TOKEN_ELEVATION elevation;
            DWORD size = sizeof(TOKEN_ELEVATION);
            if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &size)) {
                isElevated = elevation.TokenIsElevated;
            }
            CloseHandle(hToken);
        }
        return isElevated;
    }

    std::wstring stringToWString(const std::string& str) {
        return std::wstring(str.begin(), str.end());
    }

    std::string formatSize(ULONGLONG size) {
        const char* units[] = { "B", "KB", "MB", "GB", "TB" };
        int unit = 0;
        double formattedSize = static_cast<double>(size);

        while (formattedSize >= 1024.0 && unit < 4) {
            formattedSize /= 1024.0;
            unit++;
        }

        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << formattedSize << " " << units[unit];
        return stream.str();
    }

    void displayProgressBar(ULONGLONG current, ULONGLONG total, double elapsed, double eta) {
        const int barWidth = 50;
        double progress = static_cast<double>(current) / total;
        int pos = static_cast<int>(barWidth * progress);

        std::cout << "[";

        for (int i = 0; i < barWidth; ++i) {
            if (i < pos)
                std::cout << "=";
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << " ";
        }

        std::cout << "] " << std::fixed << std::setprecision(1) << (progress * 100) << "% ";

        // Format elapsed time
        if (elapsed >= 3600) {
            int hours = static_cast<int>(elapsed / 3600);
            int minutes = static_cast<int>((elapsed - hours * 3600) / 60);
            double seconds = elapsed - hours * 3600 - minutes * 60;
            std::cout << "Elapsed: " << hours << "h " << minutes << "m " << std::setprecision(0) << seconds << "s ";
        }
        else if (elapsed >= 60) {
            int minutes = static_cast<int>(elapsed / 60);
            double seconds = elapsed - minutes * 60;
            std::cout << "Elapsed: " << minutes << "m " << std::setprecision(0) << seconds << "s ";
        }
        else {
            std::cout << "Elapsed: " << elapsed << "s ";
        }

        // Format ETA
        if (eta >= 3600) {
            int hours = static_cast<int>(eta / 3600);
            int minutes = static_cast<int>((eta - hours * 3600) / 60);
            double seconds = eta - hours * 3600 - minutes * 60;
            std::cout << "ETA: " << hours << "h " << minutes << "m " << std::setprecision(0) << seconds << "s\r";
        }
        else if (eta >= 60) {
            int minutes = static_cast<int>(eta / 60);
            double seconds = eta - minutes * 60;
            std::cout << "ETA: " << minutes << "m " << std::setprecision(0) << seconds << "s\r";
        }
        else {
            std::cout << "ETA: " << eta << "s\r";
        }

        std::cout.flush();
    }

    void logMessage(const std::string& message) {
        std::ofstream logFile("shredder.log", std::ios::app); // Open log file in append mode
        if (logFile.is_open()) {
            logFile << message << std::endl;
            logFile.close();
        }
    }
}