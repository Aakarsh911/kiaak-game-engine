#pragma once

#include "Core/Window.hpp"
#include <memory>

namespace Kiaak {

class Engine {
public:
    Engine();
    ~Engine();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    bool isRunning;
    std::unique_ptr<Window> window;
};

} // namespace Kiaak
