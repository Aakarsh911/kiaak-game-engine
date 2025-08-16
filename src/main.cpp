#include "Engine.hpp"

int main() {
    Kiaak::Engine engine;
    
    if (engine.Initialize()) {
        engine.Run();
    }
    
    return 0;
}
