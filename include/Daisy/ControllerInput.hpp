#pragma once
#include <Daisy/BitFlags.hpp>
#include <Daisy/Vec.hpp>

#include <cstdint>

namespace ds {

/// Analog controller data
struct AnalogData {
    /// Left stick, range 0..=255
    Vec2<uint8_t> leftStick;
    /// Right stick range 0..=255
    Vec2<uint8_t> rightStick;
    /// L2 range 0..=255
    uint8_t l2;
    /// R2 range 0..=255
    uint8_t r2;
};

/// A bitflags enum representing all the currently pressed buttons
enum class PressedButtons : uint16_t {
    Square = 1 << 0,
    Cross = 1 << 1,
    Circle = 1 << 2,
    Triangle = 1 << 3,
    L1 = 1 << 4,
    R1 = 1 << 5,
    L2 = 1 << 6,
    R2 = 1 << 7,
    Create = 1 << 8,
    Options = 1 << 9,
    L3 = 1 << 10,
    R3 = 1 << 11,
    PS = 1 << 12,
    Touchpad = 1 << 13,
    Mute = 1 << 14,
};
DS_BITFLAGS(PressedButtons, uint16_t);

/// A bitflags enum representing the hat-switch state
enum class HatSwitch : uint8_t { None = 0, Up = 1 << 0, Right = 1 << 1, Down = 1 << 2, Left = 1 << 3 };
DS_BITFLAGS(HatSwitch, uint8_t);

/// A touchpad point
struct TouchPoint {
    /// Is the touch point currently being touched
    bool isTouching;
    /// Press id, always increments each press
    uint8_t id;
    /// Touch position
    Vec2<uint16_t> pos;
};

/// Touchpad touch data
struct TouchData {
    TouchPoint point1;
    TouchPoint point2;
};

/// Gamepad battery data
struct BatteryData {
    /// Whether the gamepad is fully charged
    bool isFullyCharged;
    /// Battery level, range 0..=100, increments of 10
    uint8_t batteryLevel;
};

/// Device flags
enum class DeviceFlags : uint8_t {
    HeadphonesConnected = 1 << 0,
    MicConnected = 1 << 1,
    BatteryCharging = 1 << 2,
};
DS_BITFLAGS(DeviceFlags, uint8_t);

/// Trigger feedback data
struct TriggerFeedback {
    uint8_t l2;
    uint8_t r2;
};

/// Controller data
struct ControllerInput {
    /// Analog data (sticks)
    AnalogData analog;
    /// Pressed buttons
    PressedButtons buttons;
    /// Hat switch pressed buttons
    HatSwitch hatSwitch;
    /// Gyroscope rotation
    Rot<uint16_t> gyro;
    /// Accelerometer values
    Vec3<uint16_t> accel;
    /// Touchpad data
    TouchData touchData;
    /// Trigger feedback data
    TriggerFeedback feedback;
    /// Battery data
    BatteryData batteryData;
    /// Device flags
    DeviceFlags flags;
};

} // namespace ds