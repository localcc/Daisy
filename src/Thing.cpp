#include <iostream>

#include <Daisy/Daisy.hpp>
#include <Daisy/Report.hpp>

#include <Windows.h>
#include <hidsdi.h>

#include <SetupAPI.h>

#include <cfgmgr32.h>

#include <iostream>
/*
#define CHECK_HANDLE(x)                                                                                                                                        \
    if ((x) == INVALID_HANDLE_VALUE)                                                                                                                           \
        __debugbreak();

DWORD CALLBACK NotifyCallback(HCMNOTIFICATION notify, PVOID context, CM_NOTIFY_ACTION action, PCM_NOTIFY_EVENT_DATA eventData, DWORD eventDataSize) {
    if (action == CM_NOTIFY_ACTION_DEVICEINTERFACEARRIVAL) {
        std::cout << "Added" << std::endl;
    } else if (action == CM_NOTIFY_ACTION_DEVICEINTERFACEREMOVAL) {
        std::cout << "Removed" << std::endl;
    }
    return ERROR_SUCCESS;
}

int main() {
    ds::Result res = ds::DualSenseManager::Initialize();
    if (res != ds::Result::OK) {
        __debugbreak();
    }

    bool first = true;
    while (1) {
        for (auto controller : ds::DualSenseManager::Get()->AvailableControllers()) {

            auto& c = ds::DualSenseManager::Get()->platform.controllers[controller];
            HidD_FlushQueue(c.hidHandle);
            // ULONG inputBuffers;
            // HidD_GetNumInputBuffers(c.hidHandle, &inputBuffers);
*/

// ds::ControllerInput input{};
// auto res = ds::DualSenseManager::Get()->GetControllerData(controller, &input);
// if (res != ds::Result::OK) {
//     // __debugbreak();
// }

// std::cout << (uint32_t)input.analog.l2 << " " << (uint32_t)input.analog.rightStick.y << std::endl;
/*
            auto data = ds::OutputBuilder()
                            .SetLeftMotor(12)
                            .SetRightMotor(128)
                            .SetLedColor({255, 0, 126})
                            .SetMicLed(ds::MicLed::On)
                            .SetPlayerLed(ds::PlayerLedFlags::Outer | ds::PlayerLedFlags::Center)
                            .Build();
            res = ds::DualSenseManager::Get()->SetControllerData(controller, data);
            if (res != ds::Result::OK) {
                // __debugbreak();
            }
        }

        ds::DualSenseManager::Get()->Tick();
    }

        GUID hidGuid;
        HidD_GetHidGuid(&hidGuid);

        CM_NOTIFY_FILTER notifyFilter{};
        notifyFilter.cbSize = sizeof(notifyFilter);
        notifyFilter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEINTERFACE;
        notifyFilter.u.DeviceInterface.ClassGuid = hidGuid;

        HCMNOTIFICATION notificationHandle;
        auto res = CM_Register_Notification(&notifyFilter, nullptr, NotifyCallback, &notificationHandle);
        if (res != CR_SUCCESS) {
            __debugbreak();
        }

        HDEVINFO classDevs = SetupDiGetClassDevsW(&hidGuid, nullptr, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        CHECK_HANDLE(classDevs);

        DWORD memberIndex = 0;
        SP_DEVICE_INTERFACE_DATA interfaceData{};
        interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        while (SetupDiEnumDeviceInterfaces(classDevs, nullptr, &hidGuid, memberIndex, &interfaceData)) {
            memberIndex++;

            DWORD requiredSize = 0;
            SetupDiGetDeviceInterfaceDetailW(classDevs, &interfaceData, nullptr, 0, &requiredSize, nullptr);

            std::vector<uint8_t> detail(requiredSize);
            SP_DEVICE_INTERFACE_DETAIL_DATA_W* casted = (SP_DEVICE_INTERFACE_DETAIL_DATA_W*)detail.data();
            casted->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

            if (!SetupDiGetDeviceInterfaceDetailW(classDevs, &interfaceData, (SP_DEVICE_INTERFACE_DETAIL_DATA_W*)detail.data(), requiredSize, nullptr, nullptr))
       {
                __debugbreak();
            }

            std::wcout << casted->DevicePath << std::endl;

            HANDLE deviceHandle =
                CreateFileW(casted->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
            if (deviceHandle == INVALID_HANDLE_VALUE) {
                continue;
            }

            HIDD_ATTRIBUTES hidAttributes;
            if (!HidD_GetAttributes(deviceHandle, &hidAttributes)) {
                continue;
            }

            if (hidAttributes.VendorID == 0x054C && hidAttributes.ProductID == 0x0CE6) {
                std::cout << "gamipad" << std::endl;
            }
        }

        while (1) {
        }*/

int main() {}