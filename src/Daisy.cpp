#include <Daisy/Crc32.hpp>
#include <Daisy/Daisy.hpp>

#include <array>

namespace ds {

#define MAX_REPORTS_PER_FRAME 10

DaisyManager* DaisyManager::SInstance = nullptr;

struct ControllerCache {
    ControllerInput cachedInputState{};
    void* userData = nullptr;
};

Result DaisyManager::Initialize() {
    if (SInstance) {
        return Result::OK;
    }
    SInstance = new DaisyManager();

    Result res = PlatformManager::Create([](auto handle) { Get()->OnControllerConnected(handle); },
                                         [](auto handle) { Get()->OnControllerDisconnected(handle); }, &SInstance->platform);
    if (res != Result::OK) {
        return res;
    }

    return Result::OK;
}

void DaisyManager::Shutdown() {
    if (SInstance) {
        delete SInstance;
        SInstance = nullptr;
    }
}

void DaisyManager::Tick() { platform.Tick(); }

const std::vector<ControllerHandle>& DaisyManager::AvailableControllers() const { return platform.GetConnectedControllers(); }

ControllerInput FromInputReport(const report::InputReportData report) {
    // todo: big endian support
    ControllerInput input{};
    input.analog.leftStick = {report.x, report.y};
    input.analog.rightStick = {report.z, report.rz};
    input.analog.l2 = report.rx;
    input.analog.r2 = report.ry;

    const auto actions = static_cast<PressedButtons>(report.buttons.buttons0 >> 4 & 0xf);
    const auto triggers = static_cast<PressedButtons>((report.buttons.buttons1 & 0xf) << 4);
    const auto front = static_cast<PressedButtons>(static_cast<uint16_t>(report.buttons.buttons1 & 0xf0) << 4);
    const auto front2 = static_cast<PressedButtons>(static_cast<uint16_t>(report.buttons.buttons1 & 0x7) << 8);
    input.buttons |= actions | triggers | front | front2;

    HatSwitch hatSwitchFlags[] = {HatSwitch::Up,   HatSwitch::Up | HatSwitch::Right,  HatSwitch::Right, HatSwitch::Right | HatSwitch::Down,
                                  HatSwitch::Down, HatSwitch::Down | HatSwitch::Left, HatSwitch::Left,  HatSwitch::Left | HatSwitch::Up};
    if (report.buttons.GetHatSwitch() <= 7) {
        input.hatSwitch = hatSwitchFlags[report.buttons.GetHatSwitch()];
    }

    input.gyro = {report.gyroPitch, report.gyroYaw, report.gyroRoll};
    input.accel = {report.accelX, report.accelY, report.accelZ};

    const auto& rpoint1 = report.touchData.point1;
    const auto& rpoint2 = report.touchData.point2;
    input.touchData.point1 = {rpoint1.IsActive(), rpoint1.GetId(), {rpoint1.GetX(), rpoint1.GetY()}};
    input.touchData.point2 = {rpoint2.IsActive(), rpoint2.GetId(), {rpoint2.GetX(), rpoint2.GetY()}};

    input.feedback.l2 = report.leftTriggerFeedback;
    input.feedback.r2 = report.rightTriggerFeedback;

    input.batteryData.batteryLevel = report.batteryData.GetPercentage() * 10;
    input.batteryData.isFullyCharged = report.batteryData.IsBatteryFull();

    SetFlags(input.flags, DeviceFlags::HeadphonesConnected, HasAnyFlag(report.deviceFlags, report::DeviceFlags::HeadphonesConnected));
    SetFlags(input.flags, DeviceFlags::MicConnected, HasAnyFlag(report.deviceFlags, report::DeviceFlags::MicConnected));
    SetFlags(input.flags, DeviceFlags::BatteryCharging, HasAnyFlag(report.deviceFlags, report::DeviceFlags::BatteryCharging));

    return input;
}

constexpr uint16_t USBInputReportSize = 64;
constexpr uint16_t BluetoothInputReportSize = 78;

Result DaisyManager::GetControllerData(ControllerHandle controller, ControllerInput* out) {
    if (!out)
        return Result::INVALID_PARAMETER;

    report::HidReportProperties reportProperties{};
    Result res = platform.GetHidProperties(controller, &reportProperties);
    if (res != Result::OK)
        return res;

    std::array<uint8_t, USBInputReportSize> usbReport{};
    std::array<uint8_t, BluetoothInputReportSize> bluetoothReport{};
    void* reportData = nullptr;
    // todo: maybe make handling of this a virtual instead? evaluate performance impact
    if (reportProperties.inputReportByteLength == USBInputReportSize) {
        reportData = usbReport.data();
    } else if (reportProperties.inputReportByteLength == BluetoothInputReportSize) {
        reportData = bluetoothReport.data();
    } else {
        return Result::UNKNOWN_INPUT_REPORT;
    }

    auto* hidReport = static_cast<report::HIDReport<uint8_t>*>(reportData);
    size_t readSize = 0;
    for (int i = 0; i < MAX_REPORTS_PER_FRAME; i++) {
        res = platform.GetReport(controller, reportData, reportProperties.inputReportByteLength, &readSize);
        if (res != Result::OK)
            return res;
        if (hidReport->reportId == 1 || hidReport->reportId == 49) // for some reason you can get either id on bluetooth, report size still applies :/
            break;
    }

    void* userData;
    Result cacheResult = platform.GetUserData(controller, &userData);
    if (hidReport->reportId != 1 && hidReport->reportId != 49) {
        if (cacheResult != Result::OK)
            return cacheResult;
        *out = static_cast<ControllerCache*>(userData)->cachedInputState;
        return Result::OK;
    }

    report::InputReportData inputReport{};
    if (reportProperties.inputReportByteLength == USBInputReportSize)
        inputReport = static_cast<report::HIDReport<report::InputReportData>*>(reportData)->data;
    else if (reportProperties.inputReportByteLength == BluetoothInputReportSize)
        inputReport = static_cast<report::BluetoothInputReport*>(reportData)->data;
    // no else clause here because higher clauses guard from other cases

    ControllerInput input = FromInputReport(inputReport);
    if (cacheResult == Result::OK) {
        static_cast<ControllerCache*>(userData)->cachedInputState = input;
    }
    *out = input;

    return Result::OK;
}

void CalculateCrc(report::BluetoothOutputReport& outputReport) {
    // todo: big endian support
    uint32_t crc = Crc32(0xeada2d49, reinterpret_cast<uint8_t*>(&outputReport), offsetof(report::BluetoothOutputReport, crc), false);
    outputReport.crc = crc;
}

Result DaisyManager::SetControllerData(ControllerHandle controller, const ds::report::OutputReportData& data) {
    report::HidReportProperties reportProperties{};
    Result res = platform.GetHidProperties(controller, &reportProperties);
    if (res != Result::OK)
        return res;

    report::HIDReport<report::OutputReportData> usbReport{};
    report::BluetoothOutputReport bluetoothReport{};

    void* reportPtr = nullptr;
    size_t reportSize = 0;
    if (reportProperties.inputReportByteLength == USBInputReportSize) {
        usbReport.reportId = 2;
        usbReport.data = data;

        reportPtr = &usbReport;
        reportSize = sizeof(usbReport);
    } else if (reportProperties.inputReportByteLength == BluetoothInputReportSize) {
        bluetoothReport.reportId = 49; // seems to be identical with input report id?
        bluetoothReport.outputMode = report::BluetoothOutputMode::DS5;
        bluetoothReport.data = data;
        CalculateCrc(bluetoothReport);

        reportPtr = &bluetoothReport;
        reportSize = sizeof(bluetoothReport);
    }

    return platform.SendReport(controller, reportPtr, reportSize);
}

Result DaisyManager::GetUserData(ControllerHandle controller, void** outUserData) {
    if (!outUserData)
        return Result::INVALID_PARAMETER;

    void* controllerCache = nullptr;
    Result res = platform.GetUserData(controller, &controllerCache);
    if (res != Result::OK)
        return res;

    *outUserData = static_cast<ControllerCache*>(controllerCache)->userData;
    return Result::OK;
}

Result DaisyManager::SetUserData(ControllerHandle controller, void* userData) {
    void* controllerCache = nullptr;
    Result res = platform.GetUserData(controller, &controllerCache);
    if (res != Result::OK)
        return res;

    static_cast<ControllerCache*>(controllerCache)->userData = userData;
    return Result::OK;
}

void DaisyManager::SendInitialReport(ControllerHandle controller) {
    ds::report::OutputReportData initialReport{};
    initialReport.flags2 = report::ChangeFlags2::ToggleMicLed | report::ChangeFlags2::ToggleLedStrips | report::ChangeFlags2::ToggleMicLed;
    initialReport.flags3 = report::ChangeFlags3::UnInterruptableLed;
    initialReport.lightbarPulseOptions = report::LightbarPulseOptions::FadeOutBlue;
    SetControllerData(controller, initialReport);
}

void DaisyManager::OnControllerConnected(ControllerHandle controller) {
    platform.SetUserData(controller, new ControllerCache{});
    SendInitialReport(controller);

    if (connectedCallback) {
        (*connectedCallback)(controller);
    }
}

void DaisyManager::OnControllerDisconnected(ControllerHandle controller) {
    void* userData = nullptr;
    void* setUserData = nullptr;
    if (platform.GetUserData(controller, &userData) == Result::OK) {
        auto* cache = static_cast<ControllerCache*>(userData);
        setUserData = cache->userData;
        delete cache;
    }

    if (disconnectedCallback) {
        (*disconnectedCallback)(controller, setUserData);
    }
}

} // namespace ds
