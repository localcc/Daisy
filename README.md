# Daisy

This is a **C++17** library designed for interfacing with DualSense controllers.

## Supported features

- [x] Accelerometer + gyroscope
- [x] Touchpad
- [x] Trigger feedback data
- [x] Battery level
- [x] Adaptive Triggers
    * [x] Feedback effect
    * [x] Weapon effect
    * [x] Vibration effect
    * [x] Multiple positions feedback effect
    * [x] Slope feedback effect
- [x] Rumble settings
- [x] Headphone/Speaker/Mic volume control
- [x] Player led control / brightness control
- [x] Lightbar color and animation control
- [x] USB/Bluetooth gamepad support
- [ ] Audio sample playback

## Example code

```c++
Result result = DaisyManager::Initialize();
if (result != Result::OK) {
    std::cout << "Failed to initialize Daisy, reason: " << result.code << std::endl;
    return -1;
}

while (1) {
    DaisyManager::Get()->Tick();
    for (auto controller : DaisyManager::Get()->AvailableControllers()) {
        ControllerInput controllerInput{};
        if (DaisyManager::Get()->GetControllerData(controller, &controllerInput) != Result::OK)
            continue;

        auto outputData = OutputBuilder{}
                              .SetLedColor({controllerInput.analog.leftStick.x, controllerInput.analog.leftStick.y, 0})
                              .SetLeftMotor(controllerInput.analog.l2)
                              .SetRightMotor(controllerInput.analog.r2)
                              .SetLeftTrigger(AdaptiveTriggerBuilder::Feedback(TriggerUtils::Zone(0.5f), TriggerUtils::Strength(0.8f)))
                              .SetRightTrigger(AdaptiveTriggerBuilder::Weapon(0.2_z, 0.6_z, 1.0_s))
                              .SetMicLed(MicLed::Pulsating)
                              .Build();
        DaisyManager::Get()->SetControllerData(controller, outputData);
    }
}
```

You can also find other examples in the [examples](./examples) directory

## Installation

To use this library in a CMake project, there are several ways:

### Fetch content

```cmake
FetchContent_Declare(
        Daisy
        GIT_REPOSITORY https://github.com/localcc/Daisy.git
        GIT_TAG main
)
FetchContent_MakeAvailable(Daisy)

target_link_libraries(YOUR_TARGET PUBLIC Daisy)
```

### Submoduling

Add the library as a submodule to your repo, then add the following to your `CMakeLists.txt`:

```cmake
add_subdirectory("Daisy")
target_link_libraries(YOUR_TARGET PUBLIC Daisy)
```

There are also other ways such as building a static library and linking to that, but that is left up to you to decide
on how you wanna use the library.

## Platform support

| Platform | status |
|----------|--------|
| Windows  | ✅      |
| macOS    | 🛠️    |
| Linux    | 🛠️    |

✅ - Supported, 🛠️ - Work in progress, ⚠️ - Currently not planned.

## Notes

The library tries to keep allocations to a minimum, and as such, the only time memory is allocated is when a controller
is connected/disconnected. It also makes no multithreaded guarantees, and it is only safe to use from a single-thread
if not guarded with a mutex.

Controller connection/disconnection will not get detected until the `Tick` method is called.

## Contributing

Commits should follow the [conventional commits](https://www.conventionalcommits.org/en/v1.0.0/) commit style.

Then just fork the repo and open a PR :3

## Trademark notice

"PlayStation", "PlayStation Family Mark", "PS5 logo", "PS5", "DualSense" and "DUALSHOCK" are registered trademarks or
trademarks of Sony Interactive Entertainment Inc. "SONY" is a registered trademark of Sony Corporation. The authors are
not affiliated in any kind with Sony Interactive Entertainment Inc.
