#include "menu.h"
#include "utils.h"
#include "file_shredder.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <limits>

namespace menu {
    void displayMenu() {
        std::cout << "=========================\n";
        std::cout << " Secure File Shredder\n";
        std::cout << "=========================\n";
        std::cout << "1. Securely delete a file\n";
        std::cout << "2. Securely shred a folder\n";
        std::cout << "3. Securely delete a partition\n";
        std::cout << "4. Exit\n";
        std::cout << "=========================\n";
    }

    void run() {
        if (!utils::isAdmin()) {
            std::cerr << "This program requires administrative privileges. Please run as administrator.\n";
            return;
        }

        while (true) {
            displayMenu();
            int choice;
            std::cout << "Enter your choice: ";
            std::cin >> choice;

            std::string customPatternStr;
            std::vector<unsigned char> customPattern;

            if (choice >= 1 && choice <= 3) {
                std::cout << "Enter a custom pattern for overwriting, or leave empty for random data: ";
                std::cin.ignore(1000000, '\n');
                std::getline(std::cin, customPatternStr);
                if (!customPatternStr.empty()) {
                    customPattern.assign(customPatternStr.begin(), customPatternStr.end());
                }
            }

            switch (choice) {
            case 1: {
                std::cout << "Enter the full path of the file to shred: ";
                std::string filepath;
                std::getline(std::cin, filepath);

                if (!std::filesystem::exists(filepath)) {
                    std::cerr << "Error: File not found. Please check the path and try again.\n";
                    continue;
                }

                std::cout << "Enter the number of overwrite passes (recommended 3 or more): ";
                size_t passes;
                std::cin >> passes;
                if (passes < 1) {
                    std::cerr << "Error: Passes must be at least 1.\n";
                    continue;
                }

                std::cout << "WARNING: This will permanently delete the file and it cannot be recovered.\n";
                std::cout << "Are you sure you want to continue? (y/n): ";
                char confirm;
                std::cin >> confirm;
                if (std::tolower(confirm) == 'y') {
                    file_shredder::securelyDelete(filepath, passes, customPattern);
                }
                else {
                    std::cout << "Operation canceled.\n";
                }
                break;
            }
            case 2: {
                std::cout << "Enter the full path of the folder to shred: ";
                std::string folderPath;
                std::getline(std::cin, folderPath);

                if (!std::filesystem::exists(folderPath)) {
                    std::cerr << "Error: Folder not found. Please check the path and try again.\n";
                    continue;
                }

                std::cout << "Enter the number of overwrite passes (recommended 3 or more): ";
                size_t passes;
                std::cin >> passes;
                if (passes < 1) {
                    std::cerr << "Error: Passes must be at least 1.\n";
                    continue;
                }

                std::cout << "WARNING: This will permanently delete the folder and its contents, and it cannot be recovered.\n";
                std::cout << "Are you sure you want to continue? (y/n): ";
                char confirm;
                std::cin >> confirm;
                if (std::tolower(confirm) == 'y') {
                    file_shredder::shredFolder(folderPath, passes, customPattern);
                }
                else {
                    std::cout << "Operation canceled.\n";
                }
                break;
            }
            case 3: {
                std::cout << "Enter the partition path (e.g., \\\\.\\F:): ";
                std::string partitionPath;
                std::getline(std::cin, partitionPath);

                if (partitionPath.empty()) {
                    std::cerr << "Error: No partition path provided. Please enter a valid partition path.\n";
                    continue;
                }

                std::cout << "Enter the number of overwrite passes (recommended 3 or more): ";
                size_t passes;
                std::cin >> passes;
                if (passes < 1) {
                    std::cerr << "Error: Passes must be at least 1.\n";
                    continue;
                }

                std::cout << "WARNING: This will permanently delete the partition and its contents, and it cannot be recovered.\n";
                std::cout << "Are you sure you want to continue? (y/n): ";
                char confirm;
                std::cin >> confirm;
                if (std::tolower(confirm) == 'y') {
                    file_shredder::shredPartition(partitionPath, passes);
                }
                else {
                    std::cout << "Operation canceled.\n";
                }
                break;
            }
            case 4:
                std::cout << "Exiting the Secure File Shredder...\n";
                return;
            default:
                std::cout << "Invalid choice, please try again.\n";
            }
        }
    }
}
