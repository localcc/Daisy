/// An example showing how to register callbacks

#include <Daisy/Daisy.hpp>

#include <iostream>

using namespace ds;

int main() {
    Result result = DaisyManager::Initialize();
    if (result != Result::OK) {
        std::cout << "Failed to initialize Daisy, reason: " << result.code << std::endl;
        return -1;
    }

    DaisyManager::Get()->OnControllerConnected([](auto controllerHandle) { std::cout << "A controller got connected" << std::endl; });
    DaisyManager::Get()->OnControllerDisconnected([](auto controllerHandle, void* userData) { std::cout << "A controller got disconnected" << std::endl; });

    while (1) {
        DaisyManager::Get()->Tick();
    }
}