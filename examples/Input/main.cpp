/// An example showing how to read controller input

#include <Daisy/Daisy.hpp>

#include <iostream>

using namespace ds;

int main() {
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

            std::cout << "Left stick: " << (uint32_t)controllerInput.analog.leftStick.x << " " << (uint32_t)controllerInput.analog.leftStick.y << std::endl;
            std::cout << "Cross: " << HasAnyFlag(controllerInput.buttons, PressedButtons::Cross)
                      << " Circle: " << HasAnyFlag(controllerInput.buttons, PressedButtons::Circle)
                      << " Square: " << HasAnyFlag(controllerInput.buttons, PressedButtons::Square)
                      << " Triangle: " << HasAnyFlag(controllerInput.buttons, PressedButtons::Triangle) << std::endl;
        }
    }
}