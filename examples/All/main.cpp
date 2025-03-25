/// An example showing every feature of the library being used

#include <Daisy/Daisy.hpp>

#include <iostream>

using namespace ds;

struct ControllerUserData {
    int32_t controllerIndex;
};

int main() {
    Result result = DaisyManager::Initialize();
    if (result != Result::OK) {
        std::cout << "Failed to initialize Daisy, reason: " << result.code << std::endl;
        return -1;
    }

    int32_t globalControllerIndex = 0;
    DaisyManager::Get()->OnControllerConnected([&](auto controllerHandle) {
        auto* userData = new ControllerUserData{globalControllerIndex++};
        DaisyManager::Get()->SetUserData(controllerHandle, userData);

        std::cout << "Controller connected" << std::endl;
    });

    DaisyManager::Get()->OnControllerDisconnected([&](auto controllerHandle, void* userData) {
        if (userData)
            delete static_cast<ControllerUserData*>(userData);

        std::cout << "Controller disconnected" << std::endl;
    });

    while (1) {
        DaisyManager::Get()->Tick();

        for (auto controller : DaisyManager::Get()->AvailableControllers()) {
            ControllerInput controllerInput{};
            if (DaisyManager::Get()->GetControllerData(controller, &controllerInput) != Result::OK)
                continue;

            int32_t controllerIndex = 0;
            void* userData = nullptr;
            if (DaisyManager::Get()->GetUserData(controller, &userData) == Result::OK) {
                controllerIndex = static_cast<ControllerUserData*>(userData)->controllerIndex;
            }

            std::cout << "[" << controllerIndex << "]"
                      << " Left Stick: " << (uint32_t)controllerInput.analog.leftStick.x << " " << (uint32_t)controllerInput.analog.rightStick.y << std::endl;

            auto outputData = OutputBuilder{}.SetLedColor({controllerInput.analog.l2, 255, controllerInput.analog.r2}).Build();
            DaisyManager::Get()->SetControllerData(controller, outputData);
        }
    }
}