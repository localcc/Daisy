#include <Daisy/ControllerOutput.hpp>

#include <cmath>
#include <cstring>

using namespace ds::report;

namespace ds {

OutputBuilder& OutputBuilder::SetRightMotor(const uint8_t value) {
    data.flags1 |= ChangeFlags1::EnableHaptics;
    data.flags2 |= ChangeFlags2::MotorPowerChange;
    data.rightMotor = value;
    return *this;
}
OutputBuilder& OutputBuilder::SetLeftMotor(const uint8_t value) {
    data.flags1 |= ChangeFlags1::EnableHaptics;
    data.flags2 |= ChangeFlags2::MotorPowerChange;
    data.leftMotor = value;
    return *this;
}

OutputBuilder& OutputBuilder::SetLeftTrigger(ds::report::TriggerData triggerData) {
    data.flags1 |= ChangeFlags1::LeftTriggerEffects;
    data.leftTrigger = triggerData;
    return *this;
}
OutputBuilder& OutputBuilder::SetRightTrigger(ds::report::TriggerData triggerData) {
    data.flags1 |= ChangeFlags1::RightTriggerEffects;
    data.rightTrigger = triggerData;
    return *this;
}

OutputBuilder& OutputBuilder::SetHeadphoneVolume(const uint8_t volume) {
    data.flags1 |= ChangeFlags1::AudioVolumeChange;
    data.headphoneVolume = volume;
    return *this;
}
OutputBuilder& OutputBuilder::SetSpeakerVolume(const uint8_t volume) {
    data.flags1 |= ChangeFlags1::AudioVolumeChange;
    data.speakerVolume = volume;
    return *this;
}
OutputBuilder& OutputBuilder::SetMicVolume(const uint8_t volume) {
    data.flags1 |= ChangeFlags1::MicVolumeChange;
    data.micVolume = volume;
    return *this;
}
OutputBuilder& OutputBuilder::SetAudioFlags(const AudioFlags flags) {
    data.flags1 |= ChangeFlags1::AudioVolumeChange | ChangeFlags1::MicVolumeChange | ChangeFlags1::SpeakerToggle;
    data.audioFlags = flags;
    return *this;
}

OutputBuilder& OutputBuilder::SetMicLed(const MicLed mode) {
    data.flags2 |= ChangeFlags2::ToggleMicLed;
    data.micLed = mode;
    return *this;
}
OutputBuilder& OutputBuilder::MuteMic(const bool mute) {
    data.flags1 |= ChangeFlags1::MicVolumeChange;
    data.flags2 |= ChangeFlags2::ToggleFullMute;
    if (mute) {
        data.audioMute |= AudioMute::Mic;
    } else {
        data.audioMute &= ~AudioMute::Mic;
    }
    return *this;
}
OutputBuilder& OutputBuilder::MuteAudio(const bool mute) {
    data.flags1 |= ChangeFlags1::AudioVolumeChange;
    data.flags2 |= ChangeFlags2::ToggleFullMute;
    if (mute) {
        data.audioMute |= AudioMute::Audio;
    } else {
        data.audioMute = ~AudioMute::Mic;
    }
    return *this;
}

OutputBuilder& OutputBuilder::SetLedBrightness(const PlayerLedBrightness brightness) {
    data.flags2 |= ChangeFlags2::ToggleLedStrips;
    data.playerLedBrightness = brightness;
    return *this;
}
OutputBuilder& OutputBuilder::SetPlayerLed(const PlayerLedFlags flags) {
    data.flags2 |= ChangeFlags2::TogglePlayerIndicator;
    data.playerLedFlags = flags;
    return *this;
}

OutputBuilder& OutputBuilder::SetLedColor(const Color<uint8_t> color) {
    data.flags2 |= ChangeFlags2::ToggleLedStrips | ChangeFlags2::TogglePlayerIndicator;
    data.lightbarColor = color;
    return *this;
}

uint8_t TriggerUtils::Zone(float val) { return static_cast<uint8_t>(std::round(val * 9.0)); }
uint8_t TriggerUtils::Strength(float val) { return static_cast<uint8_t>(std::round(val * 8.0)); }

ds::report::TriggerData AdaptiveTriggerBuilder::Off() {
    TriggerData triggerData{};
    triggerData.mode = TriggerMode::Off;
    return triggerData;
}

ds::report::TriggerData AdaptiveTriggerBuilder::Feedback(uint8_t startZone, uint8_t resistanceForce) {
    if (resistanceForce > 0) {
        // todo: add big endian support
        TriggerData data{};
        data.mode = TriggerMode::Feedback;

        uint64_t forceZones = 0;
        for (uint8_t i = startZone; i < 10; i++) {
            forceZones |= static_cast<uint64_t>(resistanceForce) << (3u * static_cast<uint64_t>(i));
            data.activeZones |= 1u << static_cast<uint16_t>(i);
        }

        data.forceZones[0] = forceZones & 0xff;
        data.forceZones[1] = (forceZones >> 8) & 0xff;
        data.forceZones[2] = (forceZones >> 16) & 0xff;
        data.forceZones[3] = (forceZones >> 24) & 0xff;
        data.forceZones[4] = (forceZones >> 32) & 0xff;
        data.forceZones[5] = (forceZones >> 48) & 0xff;
        return data;
    }
    return Off();
}

ds::report::TriggerData AdaptiveTriggerBuilder::Weapon(uint8_t startZone, uint8_t endZone, uint8_t resistanceForce) {
    if (resistanceForce > 0) {
        TriggerData data{};
        data.mode = TriggerMode::Weapon;
        data.activeZones = (1 << static_cast<uint16_t>(startZone)) | (1 << static_cast<uint16_t>(endZone));
        data.forceZones[0] = resistanceForce - 1;
        return data;
    }
    return Off();
}

ds::report::TriggerData AdaptiveTriggerBuilder::Vibration(uint8_t startZone, uint8_t amplitude, uint8_t frequency) {
    if (amplitude > 0 && frequency > 0) {
        TriggerData data{};
        data.mode = TriggerMode::Vibration;

        auto strength = static_cast<uint64_t>((amplitude - 1) & 0x07);
        uint64_t amplitudeZones = 0;
        for (uint8_t i = startZone; i < 10; i++) {
            amplitudeZones |= strength << (3u * static_cast<uint64_t>(i));
            data.activeZones |= 1 << static_cast<uint16_t>(i);
        }

        data.forceZones[0] = amplitudeZones & 0xff;
        data.forceZones[1] = (amplitudeZones >> 8) & 0xff;
        data.forceZones[2] = (amplitudeZones >> 16) & 0xff;
        data.forceZones[3] = (amplitudeZones >> 24) & 0xff;
        data.forceZones[4] = (amplitudeZones >> 32) & 0xff;
        data.forceZones[5] = (amplitudeZones >> 48) & 0xff;
        data.frequency = frequency;
        return data;
    }
    return Off();
}

ds::report::TriggerData AdaptiveTriggerBuilder::MultiplePositionFeedback(std::array<uint8_t, 10> strengths) {
    TriggerData data{};
    data.mode = TriggerMode::Feedback;

    uint64_t forceZones = 0;
    for (uint8_t i = 0; i < 10; i++) {
        if (strengths[i] > 0) {
            uint8_t force = (strengths[i] - 1) & 0x07;
            forceZones |= static_cast<uint64_t>(force) << (3u * static_cast<uint64_t>(i));
            data.activeZones |= 1u << static_cast<uint16_t>(i);
        }
    }

    data.forceZones[0] = forceZones & 0xff;
    data.forceZones[1] = (forceZones >> 8) & 0xff;
    data.forceZones[2] = (forceZones >> 16) & 0xff;
    data.forceZones[3] = (forceZones >> 24) & 0xff;
    data.forceZones[4] = (forceZones >> 32) & 0xff;
    data.forceZones[5] = (forceZones >> 48) & 0xff;
    return data;
}

} // namespace ds