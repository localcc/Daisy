#pragma once
#include <Daisy/Report.hpp>
#include <Daisy/Vec.hpp>

#include <array>

namespace ds {

/// Trigger utilities
class TriggerUtils {
public:
    /// @brief Returns zone index from a floating point value
    /// @param val zone float in range 0..=1
    /// @returns zone index for use with @see AdaptiveTriggerBuilder
    static uint8_t Zone(float val);

    /// @briefs Returns strength from a floating point value
    /// @params val strength float in range 0..=1
    /// @retuns strength for use with @see AdaptiveTriggerBuilder
    static uint8_t Strength(float val);
};

namespace literals {
inline uint8_t operator""_z(long double val) { return TriggerUtils::Zone(static_cast<float>(val)); }
inline uint8_t operator""_s(long double val) { return TriggerUtils::Strength(static_cast<float>(val)); }
} // namespace literals

using AudioFlags = ds::report::AudioFlags;
using MicLed = ds::report::MicLed;
using PlayerLedFlags = ds::report::PlayerLedFlags;
using PlayerLedBrightness = ds::report::PlayerLedBrightness;

class OutputBuilder {
public:
    OutputBuilder() = default;

    /// Set right motor rumble
    OutputBuilder& SetRightMotor(uint8_t value);
    /// Set left motor rumble
    OutputBuilder& SetLeftMotor(uint8_t value);

    /// Set left adaptive trigger
    OutputBuilder& SetLeftTrigger(ds::report::TriggerData triggerData);
    /// Set right adaptive trigger
    OutputBuilder& SetRightTrigger(ds::report::TriggerData triggerData);

    /// Set connected headphones volume
    OutputBuilder& SetHeadphoneVolume(uint8_t volume);
    /// Set built-in speaker volume
    OutputBuilder& SetSpeakerVolume(uint8_t volume);
    /// Set mic volume (either built-in or headphones one)
    OutputBuilder& SetMicVolume(uint8_t volume);

    /// Set audio flags
    OutputBuilder& SetAudioFlags(AudioFlags flags);
    /// Set mic led mode
    OutputBuilder& SetMicLed(MicLed mode);

    /// Mute mic (either built-in or headphones one)
    OutputBuilder& MuteMic(bool mute);
    /// Mute audio (either built-in speaker or headphones)
    OutputBuilder& MuteAudio(bool mute);

    /// Set touchpad led brightness
    OutputBuilder& SetLedBrightness(PlayerLedBrightness brightness);
    /// Set player number
    OutputBuilder& SetPlayerLed(PlayerLedFlags flags);

    /// Set touchpad led color
    OutputBuilder& SetLedColor(Color<uint8_t> color);

    /// Finish the builder
    [[nodiscard]] ds::report::OutputReportData Build() const { return data; }

private:
    ds::report::OutputReportData data{};
};

// Factories credit: https://gist.github.com/Nielk1/6d54cc2c00d2201ccb8c2720ad7538db
class AdaptiveTriggerBuilder {
public:
    /// Resets all effects
    ///
    /// @returns Trigger data for the effect
    static ds::report::TriggerData Off();

    /// Trigger will resist movement beyond the start position
    /// Trigger feedback data will report 0 before the effect, and 1 while the effect is triggered
    ///
    /// @param startZone starting zone of the trigger effect, must be in the range 0..=9
    /// @param resistanceForce force of trigger resistance, must be in the range 0..=8
    /// @returns Trigger data for the effect
    /// @note @see TriggerUtils
    static ds::report::TriggerData Feedback(uint8_t startZone, uint8_t resistanceForce);

    /// Trigger will resist movement beyond the start position until the end position
    /// Trigger feedback data will report 0 before the effect, 1 while the effect is triggered, 2 after the end position
    ///
    /// @param startZone starting zone of the trigger effect, must be in range 0..=9
    /// @param endZone ending zone of the trigger effect, must be in range (startingZone+1)..=9
    /// @param resistanceForce force of trigger resistance, must be in the range 0..=8
    /// @returns Trigger data for the effect
    /// @note @see TriggerUtils
    static ds::report::TriggerData Weapon(uint8_t startZone, uint8_t endZone, uint8_t resistanceForce);

    /// Trigger will vibrate with the input amplitude and frequency beyond the start position
    /// Trigger feedback data will report 0 before the effect, and 1 while the effect is triggered
    ///
    /// @param startZone starting zone of the trigger effect, must be in range 0..=9
    /// @param amplitude strength of the automatic cycling action, must be in range 0..=8
    /// @param frequency frequency of the automatic cycling action in hertz
    /// @returns Trigger data for the effect
    /// @note @see TriggerUtils
    static ds::report::TriggerData Vibration(uint8_t startZone, uint8_t amplitude, uint8_t frequency);

    /// Trigger will resist movement at varying strengths in 10 regions
    ///
    /// @param strengths array of 10 resistance values for zones 0 through 9, each strength must be in the range 0..=8
    /// @returns Trigger data for the effect
    /// @note @see TriggerUtils
    static ds::report::TriggerData MultiplePositionFeedback(std::array<uint8_t, 10> strengths);

    /// Trigger will resist movement at a linear range of strengths
    ///
    /// @param startZone starting zone of the trigger effect, must be in range 0..=9
    /// @param endZone ending zone of the trigger effect, must be in range (startingZone+1)..=9
    /// @param startStrength resistance of the trigger at the start, must be in range 0..=8
    /// @param endStrength resistance of the trigger at the end, must be in range 0..=8
    /// @returns Trigger data for the effect
    /// @note @see TriggerUtils
    static ds::report::TriggerData SlopeFeedback(uint8_t startZone, uint8_t endZone, uint8_t startStrength, uint8_t endStrength);
};

} // namespace ds