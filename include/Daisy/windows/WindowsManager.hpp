#pragma once
#include <Daisy/Atomic.hpp>
#include <Daisy/Handle.hpp>
#include <Daisy/Report.hpp>
#include <Daisy/Result.hpp>
#include <Daisy/windows/RAIIHandle.hpp>
#include <Daisy/windows/WindowsFwd.hpp>

#include <functional>
#include <string>
#include <vector>

namespace ds {

struct WindowsControllerData {
    std::wstring devicePath;
    WinHandle hidHandle;
    NotificationHandle deviceNotification;
    WinHandle readEventHandle;
    report::HidReportProperties properties;
    void* userData;
};

class WindowsManager {
public:
    using ControllerHandle = Handle<WindowsControllerData>;

public:
    /// @brief Ticks the manager
    ///
    /// This must be called every so-often for the device disconnect/connect events to take effect
    void Tick();

    /// @brief Enumerate connected devices
    Result EnumerateDevices();

    /// @brief Gets handles for connected controllers
    [[nodiscard]] const std::vector<ControllerHandle>& GetConnectedControllers() const;

    /// @brief Reads a report for a controller
    /// @param controller controller handle
    /// @param reportData report data to be filled
    /// @param reportSize report data size
    /// @param readSize actual read size from the device
    /// @returns result code
    Result GetReport(ControllerHandle controller, void* reportData, size_t reportSize, size_t* readSize);

    /// @brief Sends a report to the controller
    /// @param controller controller handle
    /// @param reportData report data to send
    /// @param reportSize report data size
    /// @returns result code
    Result SendReport(ControllerHandle controller, const void* reportData, size_t reportSize);

    /// @brief Gets hid report properties of a controller
    /// @param controller controller handle
    /// @param outProperties properties to set
    /// @returns result code
    Result GetHidProperties(ControllerHandle controller, report::HidReportProperties* outProperties);

    /// @brief Gets user data for the controller
    /// @param controller controller handle
    /// @param outUserData user data output
    Result GetUserData(ControllerHandle controller, void** outUserData);

    /// @brief Sets user data for the controller
    /// @param controller controller handle
    /// @param userData pointer to set as user data
    Result SetUserData(ControllerHandle controller, void* userData);

private:
    static Result Create(std::function<void(ControllerHandle)> onConnected, std::function<void(ControllerHandle)> onDisconnect, WindowsManager* outManager);

private:
    ControllerHandle OnControllerConnected(WindowsControllerData controllerData);
    void OnControllerDisconnect(ControllerHandle controller);

private:
    HandleVec<WindowsControllerData> controllers{};
    std::vector<ControllerHandle> connectedControllers{}; // storing in a separate vector, to prevent allocation on query
    std::function<void(ControllerHandle)> onConnected;
    std::function<void(ControllerHandle)> onDisconnect;
    NotificationHandle notificationHandle{};
    AtomicBool wantsEnumeration = true;

private:
    friend class DaisyManager;
    friend void OnDeviceAdded(WindowsManager* self);
    friend void OnDeviceRemoved(WindowsManager* self);
};

} // namespace ds