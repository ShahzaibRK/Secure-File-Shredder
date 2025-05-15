#include "volume_utils.h"
#include <winioctl.h>

namespace volume_utils {
    DWORD getSectorSize(HANDLE hVolume) {
        DISK_GEOMETRY diskGeometry;
        DWORD bytesReturned;
        if (DeviceIoControl(hVolume, IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0, &diskGeometry, sizeof(diskGeometry), &bytesReturned, nullptr)) {
            return diskGeometry.BytesPerSector;
        }
        return 4096;
    }

    HANDLE lockVolume(const std::wstring& volumePath) {
        HANDLE hVolume = CreateFileW(
            volumePath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
            nullptr
        );

        if (hVolume == INVALID_HANDLE_VALUE) {
            return INVALID_HANDLE_VALUE;
        }

        DWORD bytesReturned;
        if (!DeviceIoControl(hVolume, FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0, &bytesReturned, nullptr)) {
            CloseHandle(hVolume);
            return INVALID_HANDLE_VALUE;
        }

        return hVolume;
    }

    bool dismountVolume(HANDLE hVolume) {
        DWORD bytesReturned;
        return DeviceIoControl(hVolume, FSCTL_DISMOUNT_VOLUME, nullptr, 0, nullptr, 0, &bytesReturned, nullptr);
    }

    ULONGLONG getVolumeSize(HANDLE hVolume) {
        PARTITION_INFORMATION_EX partInfo;
        DWORD bytesReturned;
        if (DeviceIoControl(hVolume, IOCTL_DISK_GET_PARTITION_INFO_EX, nullptr, 0, &partInfo, sizeof(partInfo), &bytesReturned, nullptr)) {
            return partInfo.PartitionLength.QuadPart;
        }
        return 0;
    }
}