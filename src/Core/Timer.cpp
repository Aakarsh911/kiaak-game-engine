#include "Core/Timer.hpp"

namespace Kiaak {

Timer::Timer() 
    : lastFrameTime(0.0)
    , deltaTime(0.0)
    , totalTime(0.0)
    , accumulator(0.0)
    , fixedTimeStep(1.0 / 60.0)  // 60 updates per second
{
    lastFrameTime = glfwGetTime();  // Initialize with current time
}

double Timer::getDeltaTime() const {
    return deltaTime;
}

double Timer::getTotalTime() const {
    return totalTime;
}

void Timer::update() {
    double currentTime = glfwGetTime();
    deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;
    
    totalTime += deltaTime;
    accumulator += deltaTime;
}

bool Timer::shouldUpdateFixed() {
    if (accumulator >= fixedTimeStep) {
        accumulator -= fixedTimeStep;
        return true;
    }
    return false;
}

double Timer::getFixedDeltaTime() const {
    return fixedTimeStep;
}

} // namespace Kiaak
