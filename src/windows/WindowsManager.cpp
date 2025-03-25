#include <Daisy/Report.hpp>
#include <Daisy/windows/WindowsManager.hpp>

// clang-format off
#include <Windows.h>
#include <hidsdi.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
// clang-format on

#include <algorithm>

namespace ds {

GUID HID_GUID;

void OnDeviceAdded(WindowsManager* self) { self->wantsEnumeration.Store(true, std::memory_order_release); }
void OnDeviceRemoved(WindowsManager* self) { self->wantsEnumeration.Store(true, std::memory_order_release); }

static DWORD CALLBACK DeviceNotificationCallback(HCMNOTIFICATION notification, PVOID context, CM_NOTIFY_ACTION action, PCM_NOTIFY_EVENT_DATA eventData,
                                                 DWORD eventDataSize) {
    DS_UNUSED(notification);
    DS_UNUSED(eventData);
    DS_UNUSED(eventDataSize);

    auto manager = static_cast<WindowsManager*>(context);
    if (action == CM_NOTIFY_ACTION_DEVICEINTERFACEARRIVAL) {
        OnDeviceAdded(manager);
    } else if (action == CM_NOTIFY_ACTION_DEVICEINTERFACEREMOVAL) {
        OnDeviceRemoved(manager);
    }
    return ERROR_SUCCESS;
}

static DWORD CALLBACK SpecificDeviceNotificationCallback(HCMNOTIFICATION notification, PVOID context, CM_NOTIFY_ACTION action, PCM_NOTIFY_EVENT_DATA eventData,
                                                         DWORD eventDataSize) {
    DS_UNUSED(notification);
    DS_UNUSED(eventData);
    DS_UNUSED(eventDataSize);

    auto manager = static_cast<WindowsManager*>(context);
    switch (action) {
    case CM_NOTIFY_ACTION_DEVICEQUERYREMOVE:
    case CM_NOTIFY_ACTION_DEVICEREMOVEPENDING:
    case CM_NOTIFY_ACTION_DEVICEREMOVECOMPLETE:
        OnDeviceRemoved(manager); // todo: pass the handle into this callback so we don't needlessly re-enumerate the whole list
        break;
    case CM_NOTIFY_ACTION_DEVICEQUERYREMOVEFAILED:
        OnDeviceAdded(manager);
        break;
    default:
        break;
    }
    return ERROR_SUCCESS;
}

Result WindowsManager::Create(std::function<void(ControllerHandle)> onConnected, std::function<void(ControllerHandle)> onDisconnect,
                              WindowsManager* outManager) {
    if (!outManager) {
        return Result::INVALID_PARAMETER;
    }

    HidD_GetHidGuid(&HID_GUID);

    CM_NOTIFY_FILTER notifyFilter{};
    notifyFilter.cbSize = sizeof(notifyFilter);
    notifyFilter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEINTERFACE;
    notifyFilter.u.DeviceInterface.ClassGuid = HID_GUID;

    WindowsManager manager{};
    manager.onConnected = std::move(onConnected);
    manager.onDisconnect = std::move(onDisconnect);
    *outManager = std::move(manager);

    auto res = CM_Register_Notification(&notifyFilter, outManager, DeviceNotificationCallback,
                                        reinterpret_cast<HCMNOTIFICATION*>(&outManager->notificationHandle.handle));
    if (res != CR_SUCCESS) {
        return Result(Result::NOTIFICATION_REGISTER, res);
    }

    return Result::OK;
}

void WindowsManager::Tick() {
    bool expected = true;
    if (this->wantsEnumeration.CompareExchangeStrong(expected, true, std::memory_order_acquire)) {
        EnumerateDevices();
        this->wantsEnumeration.Store(false, std::memory_order_release);
    }
}

Result WindowsManager::EnumerateDevices() {
    DevInfoHandle deviceList = SetupDiGetClassDevsW(&HID_GUID, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (deviceList.handle == INVALID_HANDLE_VALUE) {
        return Result(Result::DEVICE_ENUMERATION, GetLastError());
    }

    DWORD memberIndex = 0;
    SP_DEVICE_INTERFACE_DATA interfaceData{};
    interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    std::vector<ControllerHandle> keptControllers{};
    while (SetupDiEnumDeviceInterfaces(deviceList, nullptr, &HID_GUID, memberIndex, &interfaceData)) {
        memberIndex++;

        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetailW(deviceList, &interfaceData, nullptr, 0, &requiredSize, nullptr);

        std::vector<uint8_t> backingData(requiredSize);
        auto* interfaceDetail = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(backingData.data());
        interfaceDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA_W);

        BOOL res = SetupDiGetDeviceInterfaceDetailW(deviceList, &interfaceData, interfaceDetail, requiredSize, nullptr, nullptr);
        if (!res) {
            continue;
        }

        std::wstring devicePath(interfaceDetail->DevicePath);

        WinHandle deviceHandle =
            CreateFileW(devicePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        if (deviceHandle.handle == INVALID_HANDLE_VALUE) {
            continue;
        }

        HIDD_ATTRIBUTES hidAttributes;
        if (!HidD_GetAttributes(deviceHandle, &hidAttributes)) {
            continue;
        }

        if (hidAttributes.VendorID == report::VENDOR_ID && hidAttributes.ProductID == report::PRODUCT_ID) {
            PHIDP_PREPARSED_DATA preparsedData{};
            if (!HidD_GetPreparsedData(deviceHandle, &preparsedData))
                continue;

            auto _ = ds::raii([preparsedData]() { HidD_FreePreparsedData(preparsedData); });

            HIDP_CAPS caps{};
            if (HidP_GetCaps(preparsedData, &caps) != HIDP_STATUS_SUCCESS)
                continue;

            // skipping if the controller has already been added
            bool alreadyConnected = false;
            for (auto [handle, controller] : this->controllers) {
                if (controller.devicePath == devicePath) {
                    keptControllers.push_back(handle);
                    alreadyConnected = true;
                    break;
                }
            }
            if (alreadyConnected)
                continue;

            WinHandle readEventHandle = CreateEventW(nullptr, true, false, L"Daisy_Read");
            if (!readEventHandle)
                continue;

            WindowsControllerData controllerData{};
            controllerData.devicePath = std::move(devicePath);
            controllerData.hidHandle = std::move(deviceHandle);
            controllerData.properties = {caps.InputReportByteLength, caps.OutputReportByteLength};
            controllerData.readEventHandle = std::move(readEventHandle);

            CM_NOTIFY_FILTER filter{};
            filter.cbSize = sizeof(filter);
            filter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEHANDLE;
            filter.u.DeviceHandle.hTarget = controllerData.hidHandle.handle;

            CM_Register_Notification(&filter, this, SpecificDeviceNotificationCallback, reinterpret_cast<HCMNOTIFICATION*>(&controllerData.deviceNotification));

            auto handle = OnControllerConnected(std::move(controllerData));
            keptControllers.push_back(handle);
        }
    }

    std::vector<ControllerHandle> removedControllers{};
    for (auto [handle, _] : this->controllers) {
        if (std::find(keptControllers.begin(), keptControllers.end(), handle) == keptControllers.end()) {
            removedControllers.push_back(handle);
        }
    }
    for (auto removed : removedControllers) {
        OnControllerDisconnect(removed);
    }

    return Result::OK;
}

const std::vector<WindowsManager::ControllerHandle>& WindowsManager::GetConnectedControllers() const { return connectedControllers; }

Result WindowsManager::GetReport(ControllerHandle controller, void* reportData, size_t reportSize, size_t* readSize) {
    if (!readSize)
        return Result::INVALID_PARAMETER;
    if (!this->controllers.Contains(controller))
        return Result::CONTROLLER_NOT_FOUND;
    auto& controllerData = this->controllers[controller];

    OVERLAPPED overlapped{};
    overlapped.hEvent = controllerData.readEventHandle;
    overlapped.Offset = overlapped.OffsetHigh = 0;

    DWORD numberOfBytesRead = 0;
    DWORD lastError = 0;
    ResetEvent(controllerData.readEventHandle);
    if (!ReadFile(controllerData.hidHandle, reportData, static_cast<DWORD>(reportSize), &numberOfBytesRead, &overlapped)) {
        lastError = GetLastError();
        if (lastError != ERROR_IO_PENDING) {
            if (lastError == ERROR_DEVICE_NOT_CONNECTED) {
                HidD_FlushQueue(controllerData.hidHandle);
                OnDeviceRemoved(this);
            }
            return Result(Result::USB_COMMUNICATION, lastError);
        }
    }
    if (!GetOverlappedResultEx(controllerData.hidHandle, &overlapped, &numberOfBytesRead, 2, false)) {
        lastError = GetLastError();
        if (lastError == WAIT_TIMEOUT) {
            HidD_FlushQueue(controllerData.hidHandle); // need to flush queue because there might be a case
                                                       // where the bluetooth device was disconnected but windows might not detect it
                                                       // and we will keep timing out on a disconnected controller
            return Result::TIMEOUT;
        }
        return Result(Result::USB_COMMUNICATION, lastError);
    }
    *readSize = numberOfBytesRead;

    return Result::OK;
}

Result WindowsManager::SendReport(ControllerHandle controller, const void* reportData, size_t reportSize) {
    if (!this->controllers.Contains(controller))
        return Result::CONTROLLER_NOT_FOUND;
    auto& controllerData = this->controllers[controller];

    DWORD numberOfBytesWritten = 0;
    if (!WriteFile(controllerData.hidHandle.handle, reportData, static_cast<DWORD>(reportSize), &numberOfBytesWritten, nullptr)) {
        DWORD lastError = GetLastError();
        if (lastError == ERROR_DEVICE_NOT_CONNECTED)
            OnDeviceRemoved(this);
        return Result(Result::USB_COMMUNICATION, lastError);
    }

    return Result::OK;
}

Result WindowsManager::GetHidProperties(ControllerHandle controller, report::HidReportProperties* outProperties) {
    if (!this->controllers.Contains(controller))
        return Result::CONTROLLER_NOT_FOUND;
    *outProperties = this->controllers[controller].properties;
    return Result::OK;
}

Result WindowsManager::GetUserData(ControllerHandle controller, void** outUserData) {
    if (!outUserData)
        return Result::INVALID_PARAMETER;
    if (!this->controllers.Contains(controller))
        return Result::CONTROLLER_NOT_FOUND;
    *outUserData = this->controllers[controller].userData;
    return Result::OK;
}

Result WindowsManager::SetUserData(ControllerHandle controller, void* userData) {
    if (!this->controllers.Contains(controller))
        return Result::CONTROLLER_NOT_FOUND;
    this->controllers[controller].userData = userData;
    return Result::OK;
}

WindowsManager::ControllerHandle WindowsManager::OnControllerConnected(WindowsControllerData controllerData) {
    auto handle = this->controllers.Add(std::move(controllerData));
    this->connectedControllers.push_back(handle);
    onConnected(handle);
    return handle;
}
void WindowsManager::OnControllerDisconnect(ControllerHandle controller) {
    onDisconnect(controller);
    this->connectedControllers.erase(std::remove(this->connectedControllers.begin(), this->connectedControllers.end(), controller),
                                     this->connectedControllers.end());
    this->controllers.Remove(controller);
}

} // namespace ds