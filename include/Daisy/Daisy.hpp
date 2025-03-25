#pragma once
#include <Daisy/ControllerInput.hpp>
#include <Daisy/ControllerOutput.hpp>
#include <Daisy/Handle.hpp>
#include <Daisy/Result.hpp>
#include <Daisy/windows/WindowsManager.hpp>

#include <cstdint>
#include <optional>
#include <vector>

namespace ds {

using PlatformManager = WindowsManager;
using ControllerHandle = PlatformManager::ControllerHandle;

/// @brief Controller manager
///
/// The manager should be first initialized by making a call to @see DaisyManager::Initialize
///
/// Then you should periodically call @see DaisyManager::Tick for device connection/disconnection
/// events to get processed.
class DaisyManager {
public:
    using ControllerConnected = std::function<void(ControllerHandle handle)>;
    /// The user data parameter will be the userData you've set or nullptr
    using ControllerDisconnected = std::function<void(ControllerHandle handle, void* userData)>;

public:
    /// @brief Initializes the Daisy manager
    ///
    /// This function must be called before anything else is attempted with the manager
    static Result Initialize();
    /// @brief Shuts down the Daisy manager
    ///
    /// After calling this function, no further calls to methods of the manager should be made.
    /// The manager can be re-initialized again with the @see DaisyManager::Initialize function
    static void Shutdown();

    /// @brief Get an instance of the manager
    static DaisyManager* Get() { return SInstance; }

public:
    /// @bief Ticks the manager
    ///
    /// This must be called every so-often for the device disconnect/connect events to take effect
    void Tick();

    /// @brief Get available controllers
    [[nodiscard]] const std::vector<ControllerHandle>& AvailableControllers() const;

    /// @brief Get controller data for a controller at the specified index
    /// @param controller controller handle
    /// @param out input data to be set on successfull fetch
    /// @return result code
    ///
    /// If the controller doesn't exist, returns default instance of the struct
    Result GetControllerData(ControllerHandle controller, ControllerInput* out);

    /// @brief Set controller data for a controller at the specified index
    /// @param controller controller handle
    /// @param data output report data, can be built with @see ds::OutputBuilder
    /// @return result code
    ///
    /// If the controller doesn't exist or setting the data fails, returns false
    Result SetControllerData(ControllerHandle controller, const ds::report::OutputReportData& data);

    /// @brief Get custom user data for the controller
    /// @param controller controller handle
    /// @param outUserData user data output
    Result GetUserData(ControllerHandle controller, void** outUserData);

    /// @brief Set custom user data for the controller
    /// @param controller controller handle
    /// @param userData pointer to set as user data
    Result SetUserData(ControllerHandle controller, void* userData);

    /// @brief Sets the callback that gets invoked when a controller gets connected
    /// @param callback callback to call
    void OnControllerConnected(ControllerConnected callback) { connectedCallback = callback; }
    /// @brief Clears the callback that gets invoked when a controller gets connected
    void ClearControllerConnected() { connectedCallback = {}; }

    /// @brief Sets the callback that gets invoked when a controller get disconnected
    /// @param callback callback to call
    ///
    /// At the point when the callback is invoked, it is still valid to call any function with this controller handle
    /// until your function returns.
    void OnControllerDisconnected(ControllerDisconnected callback) { disconnectedCallback = callback; }
    /// @briefs Clears the callback that gets invoked when a controller gets disconnected
    void ClearControllerDisconnected() { disconnectedCallback = {}; }

private:
    void SendInitialReport(ControllerHandle controller);

    void OnControllerConnected(ControllerHandle controller);
    void OnControllerDisconnected(ControllerHandle controller);

private:
    static DaisyManager* SInstance;

private:
    PlatformManager platform;
    std::optional<ControllerConnected> connectedCallback;
    std::optional<ControllerDisconnected> disconnectedCallback;
};

} // namespace ds