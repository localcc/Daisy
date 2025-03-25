#pragma once
#include <Daisy/BitFlags.hpp>
#include <Daisy/Vec.hpp>

#include <cstdint>
#include <stddef.h>

namespace ds::report {

const uint16_t VENDOR_ID = 0x054c;
const uint16_t PRODUCT_ID = 0x0CE6;

struct PointData {
    uint8_t id; // [ isNotActive | id(7) ]
    uint8_t x0;
    uint8_t x1; // [ x1(4) | y0(4) ]
    uint8_t y1;

    [[nodiscard]] bool IsActive() const { return !((id >> 7) & 1); }
    [[nodiscard]] uint8_t GetId() const { return id & 0x7f; }
    [[nodiscard]] uint16_t GetX() const { return x0 | static_cast<uint16_t>(x1 & 0xf) << 8; }
    [[nodiscard]] uint16_t GetY() const { return static_cast<uint16_t>(y1) << 4 | (x1 >> 4); }
};
static_assert(sizeof(PointData) == 4);

struct TouchData {
    PointData point1;
    PointData point2;
};
static_assert(sizeof(TouchData) == 8);

struct Buttons {
    uint8_t buttons0; // [ triangle | circle | cross | square | hatSwitch(4) ]
    uint8_t buttons1; // [ r3 | l3 | options | create | r2 | l2 | r1 | l1 ]
    uint8_t buttons2; // [ unk(5) | mute | touchpad | ps ]

    [[nodiscard]] uint8_t GetHatSwitch() const { return buttons0 & 0xf; }
};
static_assert(sizeof(Buttons) == 3);

struct BatteryData {
    uint8_t battery0; // [ unk(2) | isBatteryFull | unk | percentage(4) ]

    // Percentage returned should be multiplied by 10 to get the actual percentage
    [[nodiscard]] uint8_t GetPercentage() const { return battery0 & 0xf; }
    [[nodiscard]] uint8_t IsBatteryFull() const { return (battery0 >> 5) & 1; }
};
static_assert(sizeof(BatteryData) == 1);

enum class DeviceFlags : uint8_t {
    HeadphonesConnected = 1 << 0,
    MicConnected = 1 << 1,
    BatteryCharging = 1 << 3,
};
DS_BITFLAGS(DeviceFlags, uint8_t);

#pragma pack(push, 1)
struct InputReportData {
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t rz;
    uint8_t rx;
    uint8_t ry;
    uint8_t vendorDefined_ff00_20;
    Buttons buttons;
    uint8_t unk;
    uint32_t timestamp;
    uint16_t gyroPitch;
    uint16_t gyroYaw;
    uint16_t gyroRoll;
    uint16_t accelX;
    uint16_t accelY;
    uint16_t accelZ;
    uint32_t sensorTimestamp;
    uint8_t motionUnk;
    TouchData touchData;
    uint8_t unk1;
    uint8_t leftTriggerFeedback;
    uint8_t rightTriggerFeedback;
    uint32_t unk_timestamp;
    uint8_t unk2;
    uint32_t unk_timestamp1;
    BatteryData batteryData;
    DeviceFlags deviceFlags;
    uint8_t unk3[9];
};
#pragma pack(pop)
static_assert(sizeof(InputReportData) == 63);

/// Output change flags part 1
enum class ChangeFlags1 : uint8_t {
    None = 0x00,
    /// (0x01) set the main motors (also requires flag 0x02); setting this by itself will allow rumble to gracefully terminate and then re-enable audio haptics,
    /// whereas not setting it will kill the rumble instantly and re-enable audio haptics.
    /// (0x02) set the main motors (also requires flag 0x01; without bit 0x01 motors are allowed to time out without re-enabling audio haptics)
    EnableHaptics = 0x01 | 0x02,
    RightTriggerEffects = 0x04,
    LeftTriggerEffects = 0x08,
    AudioVolumeChange = 0x10,
    SpeakerToggle = 0x20,
    MicVolumeChange = 0x40,
};
DS_BITFLAGS(ChangeFlags1, uint8_t);

/// Output change flags part2
enum class ChangeFlags2 : uint8_t {
    None = 0x00,
    ToggleMicLed = 0x01,
    ToggleFullMute = 0x02,
    ToggleLedStrips = 0x04,
    Unk_TurnAllLedsOff = 0x08,
    TogglePlayerIndicator = 0x10,
    MotorPowerChange = 0x40,
};
DS_BITFLAGS(ChangeFlags2, uint8_t);

/// Audio flags
enum class AudioFlags : uint8_t {
    /// None
    None = 0x00,
    /// force use of internal controller mic (if neither 0x01 and 0x02 are set, an attached headset will take precedence)
    ForceInternalMic = 0x01,
    /// force use of mic attached to the controller (headset)
    ForceHeadsetMic = 0x02,
    /// pads left channel of external mic (~1/3rd of the volume? maybe the amount can be controlled?)
    PadLeftMicChannel = 0x04,
    /// pads left channel of internal mic(~1 / 3rd of the volume ? maybe the amount can be controlled ? )
    PadLeftMicChannel1 = 0x08,
    /// disable attached headphones (only if 0x20 to enable internal speakers is provided as well)
    DisableHeadphones = 0x10,
    /// enable audio on internal speaker (in addition to a connected headset; headset will use a stereo upmix of the left channel, internal speaker will play
    /// the right channel)
    EnableSpeaker = 0x20,
};
DS_BITFLAGS(AudioFlags, uint8_t);

/// Mic led mode
enum class MicLed : uint8_t { Off = 0, On = 1, Pulsating = 2 };

/// Audio mute flags
enum class AudioMute : uint8_t {
    None = 0x0,
    Mic = 0x10,
    Audio = 0x40,
};
DS_BITFLAGS(AudioMute, uint8_t);

/// Adaptive triggers mode
enum class TriggerMode : uint8_t { Off = 0x05, Feedback = 0x21, Weapon = 0x25, Vibration = 0x26 };

/// Adaptive trigger data
#pragma pack(push, 1)
struct TriggerData {
    TriggerMode mode;
    uint16_t activeZones;
    uint8_t forceZones[6];
    uint8_t frequency;
    uint8_t unk;
};
#pragma pack(pop)
static_assert(sizeof(TriggerData) == 11);

enum class ChangeFlags3 : uint8_t {
    Off = 0,
    /// Enabled led brightness change
    PlayerLedBrightness = 1 << 0,
    /// Enable uninterruptable blue LED pulse
    UnInterruptableLed = 1 << 1,
    /// Enable improved rumble emulation (requires firmwaer 2.24 or newer)
    ImprovedRumble = 1 << 2,
};
DS_BITFLAGS(ChangeFlags3, uint8_t);

enum class LightbarPulseOptions : uint8_t {
    Off = 0,
    /// Fade in blue color over 2 seconds
    FadeInBlue = 1 << 0,
    /// Fade out with switch back to configured LED color. @see ChangeFlags3::UnInterruptableLed is required for this to take effect
    FadeOutBlue = 1 << 1
};

enum class PlayerLedBrightness : uint8_t { High = 0, Medium = 0x01, Low = 0x02 };

enum class PlayerLedFlags : uint8_t {
    Off = 0,
    Center = 0b00100,
    Inner = 0b01010,
    Outer = 0b10001,
};
DS_BITFLAGS(PlayerLedFlags, uint8_t);

struct OutputReportData {
    ChangeFlags1 flags1;
    ChangeFlags2 flags2;
    uint8_t rightMotor;
    uint8_t leftMotor;
    uint8_t headphoneVolume;
    uint8_t speakerVolume;
    /// (not at all linear; 0-255, maxes out at 0x40, all values above are treated like 0x40; 0 is not fully muted, use audio mute flag instead!)
    uint8_t micVolume;
    AudioFlags audioFlags;
    MicLed micLed;
    AudioMute audioMute;
    TriggerData rightTrigger;
    TriggerData leftTrigger;
    uint8_t unk[4];
    uint8_t hapticsMuffle; // [ triggerEffects(4) | mainMotor(4) ] 12.5% increments
    uint8_t unk1;
    ChangeFlags3 flags3;
    uint8_t unk2[2];
    LightbarPulseOptions lightbarPulseOptions;
    PlayerLedBrightness playerLedBrightness;
    PlayerLedFlags playerLedFlags;
    Color<uint8_t> lightbarColor;
};

template <typename T>
struct HIDReport {
    uint8_t reportId;
    T data;
};

struct BluetoothInputReport {
    uint8_t reportId;
    uint8_t sequenceNumber;
    InputReportData data;
};

enum class BluetoothOutputMode : uint8_t {
    DS4Compat = 1 << 0,
    DS5 = 1 << 1,
};

#pragma pack(push, 1)
struct BluetoothOutputReport {
    uint8_t reportId;
    BluetoothOutputMode outputMode;
    OutputReportData data;
    uint8_t pad[0x19];
    uint32_t crc;
    uint8_t pad_1[0x1f0];
};
#pragma pack(pop)

struct HidReportProperties {
    uint16_t inputReportByteLength;
    uint16_t outputReportByteLength;
};

} // namespace ds::report
