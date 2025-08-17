#pragma once

#include <GLFW/glfw3.h>

namespace Kiaak {

class Timer {
public:
    Timer();
    
    // Get time between frames
    double getDeltaTime() const;
    
    // Get total time since engine start
    double getTotalTime() const;
    
    // Update timer (call this every frame)
    void update();
    
    // Fixed timestep functions
    bool shouldUpdateFixed();
    double getFixedDeltaTime() const;
    
private:
    double lastFrameTime;    // Time of last frame
    double deltaTime;        // Time between frames
    double totalTime;        // Time since start
    
    // For fixed timestep
    double accumulator;      // Tracks leftover time
    double fixedTimeStep;    // Fixed time step (e.g., 1/60 for 60 updates/sec)
};

} // namespace Kiaak
