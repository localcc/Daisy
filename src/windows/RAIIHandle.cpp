#include <Daisy/windows/RAIIHandle.hpp>

// clang-format off
#include <Windows.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
// clang-format on

namespace ds {

void CloseWin::operator()(wt::HANDLE& handle) const {
    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        handle = INVALID_HANDLE_VALUE;
    }
}

void CloseNotification::operator()(wt::HCMNOTIFICATION& handle) const {
    if (handle != INVALID_HANDLE_VALUE) {
        CM_Unregister_Notification(static_cast<HCMNOTIFICATION>(handle));
        handle = INVALID_HANDLE_VALUE;
    }
}

void CloseDevInfo::operator()(wt::HDEVINFO& handle) const {
    if (handle != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(handle);
        handle = INVALID_HANDLE_VALUE;
    }
}

} // namespace ds