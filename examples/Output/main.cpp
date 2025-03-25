/// An example showing how to send various things to the controller

#include <Daisy/Daisy.hpp>

#include <iostream>

using namespace ds;
using namespace ds::literals;

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
}