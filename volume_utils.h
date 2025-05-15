#pragma once
#include <windows.h>
#include <string>

namespace volume_utils {
    DWORD getSectorSize(HANDLE hVolume);
    HANDLE lockVolume(const std::wstring& volumePath);
    bool dismountVolume(HANDLE hVolume);
    ULONGLONG getVolumeSize(HANDLE hVolume);
}