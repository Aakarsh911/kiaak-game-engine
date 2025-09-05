# Kiaak Game Engine

**Kiaak** is a lightweight, modern 2D game engine written in C++17 designed for rapid prototyping and game development. It features an integrated visual editor, component-based architecture, and Lua scripting support, making it ideal for indie developers and educational purposes.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)

## 🎮 What is Kiaak Engine?

Kiaak is a complete 2D game development solution that provides:
- **Visual Scene Editor** - Drag-and-drop interface for building game scenes
- **Component System** - Flexible entity-component architecture
- **Real-time Rendering** - OpenGL-based 2D graphics pipeline
- **Physics Simulation** - Built-in 2D physics with collision detection
- **Scripting Support** - Lua integration for gameplay logic
- **Asset Management** - Project-based workflow with organized asset handling
- **Cross-platform** - Runs on Windows, macOS, and Linux

## ✨ Key Features

### 🎨 Visual Editor
- **Scene Hierarchy** - Tree view of all game objects in your scene
- **Inspector Panel** - Edit component properties in real-time
- **Asset Browser** - Manage textures, scripts, and other resources
- **Play/Edit Modes** - Test your game without leaving the editor
- **Camera Controls** - Pan, zoom, and navigate your scenes with ease
- **Gizmo Tools** - Visual transform handles for moving objects

### 🧩 Component System
- **Transform Component** - Position, rotation, and scale
- **Sprite Renderer** - 2D texture rendering with transparency support
- **Camera Component** - Orthographic cameras with zoom control
- **Rigidbody2D** - Physics simulation for dynamic objects
- **Collider2D** - Collision detection and response
- **Script Component** - Lua scripting for custom behavior
- **Tilemap System** - Efficient tile-based level creation
- **Animator** - Animation system for sprites and objects

### 🎯 Core Engine Systems
- **Scene Management** - Multiple scenes with seamless switching
- **Project System** - Organized folder structure (assets/, scenes/)
- **Serialization** - Save/load scenes and project data
- **Input Handling** - Keyboard and mouse input processing
- **Timer System** - Fixed timestep and variable timestep updates
- **2D Physics** - Collision detection and rigid body dynamics
- **Shader System** - Custom GLSL shader support

### 💻 Scripting & Extensibility
- **Lua Integration** - Write gameplay logic in Lua
- **Hot Reloading** - Modify scripts without restarting
- **Component Scripting** - Attach custom behaviors to game objects
- **Engine API Access** - Full access to engine systems from scripts

## 🛠️ Technical Architecture

### Core Systems
```
Kiaak Engine
├── Core/
│   ├── Engine.cpp           # Main engine loop and initialization
│   ├── Scene.cpp            # Scene management and object containers
│   ├── GameObject.cpp       # Entity system with components
│   ├── Transform.cpp        # Position, rotation, scale handling
│   ├── Camera.cpp           # Orthographic camera system
│   ├── Timer.cpp            # Frame timing and delta time
│   └── Project.cpp          # Project and asset path management
├── Graphics/
│   ├── Renderer.cpp         # OpenGL rendering pipeline
│   ├── Shader.cpp           # GLSL shader loading and management
│   ├── Texture.cpp          # Texture loading and binding
│   ├── SpriteRenderer.cpp   # 2D sprite rendering component
│   └── VertexArray.cpp      # OpenGL vertex array abstraction
├── Editor/
│   ├── EditorCore.cpp       # Editor initialization and management
│   └── EditorUI.cpp         # ImGui-based user interface
└── Physics/
    ├── Physics2D.cpp        # 2D physics world simulation
    ├── Rigidbody2D.cpp      # Dynamic physics bodies
    └── Collider2D.cpp       # Collision shapes and detection
```

### Dependencies
- **GLFW** - Cross-platform windowing and input
- **OpenGL** - Hardware-accelerated 2D/3D graphics
- **ImGui** - Immediate mode GUI for the editor
- **GLM** - Mathematics library for graphics
- **Lua 5.4** - Embedded scripting language
- **Sol2** - Modern C++ to Lua binding
- **GLAD** - OpenGL function loading

## 🚀 Getting Started

### Prerequisites
- **CMake** 3.10 or higher
- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **OpenGL** 3.3+ support
- **Git** for cloning dependencies

### Building on Linux/macOS
```bash
# Clone the repository
git clone https://github.com/Aakarsh911/kiaak-game-engine.git
cd kiaak-game-engine

# Install dependencies (Ubuntu/Debian)
sudo apt-get install cmake build-essential libglfw3-dev libgl1-mesa-dev

# Install dependencies (macOS with Homebrew)
brew install cmake glfw glm

# Build the engine
mkdir build && cd build
cmake ..
make -j4

# Run the engine
./KiaakEngine
```

### Building on Windows
```cmd
# Clone the repository
git clone https://github.com/Aakarsh911/kiaak-game-engine.git
cd kiaak-game-engine

# Install dependencies using vcpkg (recommended)
vcpkg install glfw3 glm

# Build with CMake
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release

# Run the engine
Release\KiaakEngine.exe
```

## 📖 Usage Guide

### Creating Your First Project

1. **Launch the Engine**: Run the KiaakEngine executable
2. **Create Project Structure**: The engine automatically creates:
   - `assets/` - Store textures, sounds, and other resources
   - `scenes/` - Scene files (.scene format)
3. **Scene Editor**: Use the visual editor to build your game world

### Basic Workflow

```cpp
// Example: Creating a game object with components
auto* gameObject = scene->CreateGameObject("Player");

// Add transform component (automatically added)
auto* transform = gameObject->GetTransform();
transform->SetPosition(0.0f, 0.0f, 0.0f);

// Add sprite renderer
auto* sprite = gameObject->AddComponent<SpriteRenderer>();
sprite->SetTexture("assets/player.png");

// Add physics
auto* rigidbody = gameObject->AddComponent<Rigidbody2D>();
rigidbody->SetMass(1.0f);

auto* collider = gameObject->AddComponent<Collider2D>();
collider->SetSize(1.0f, 1.0f);
```

### Lua Scripting Example

```lua
-- player_controller.lua
local Player = {}

function Player:Start()
    self.speed = 5.0
    self.transform = self.gameObject:GetTransform()
end

function Player:Update(deltaTime)
    local input = Engine.GetInput()
    local x, y = 0, 0
    
    if input:IsKeyPressed("A") then x = x - 1 end
    if input:IsKeyPressed("D") then x = x + 1 end
    if input:IsKeyPressed("W") then y = y + 1 end
    if input:IsKeyPressed("S") then y = y - 1 end
    
    local currentPos = self.transform:GetPosition()
    self.transform:SetPosition(
        currentPos.x + x * self.speed * deltaTime,
        currentPos.y + y * self.speed * deltaTime,
        currentPos.z
    )
end

return Player
```

## 🎮 Editor Controls

### Navigation
- **Middle Mouse + Drag**: Pan camera
- **Mouse Wheel**: Zoom in/out
- **Space**: Reset camera to origin

### Object Manipulation
- **Left Click**: Select objects
- **G + Drag**: Move selected object
- **R + Drag**: Rotate selected object
- **S + Drag**: Scale selected object

### Editor Modes
- **Tab**: Toggle between Edit and Play mode
- **Ctrl+S**: Save current scene
- **Ctrl+N**: Create new scene

## 📁 Project Structure

When you create a project, Kiaak organizes files as follows:

```
MyGame/
├── assets/              # Game resources
│   ├── textures/       # Image files (.png, .jpg)
│   ├── scripts/        # Lua script files (.lua)
│   └── shaders/        # GLSL shader files (.vert, .frag)
├── scenes/             # Scene files
│   ├── MainScene.scene # Scene data in text format
│   └── Level1.scene    # Additional game scenes
└── last_project.txt    # Project path persistence
```

## 🔧 Extending the Engine

### Creating Custom Components

```cpp
// Custom component example
class HealthComponent : public Kiaak::Core::Component {
public:
    HealthComponent(int maxHealth) : maxHP(maxHealth), currentHP(maxHealth) {}
    
    void TakeDamage(int damage) {
        currentHP = std::max(0, currentHP - damage);
        if (currentHP <= 0) {
            // Handle death
        }
    }
    
    bool IsAlive() const { return currentHP > 0; }
    
private:
    int maxHP;
    int currentHP;
};
```

### Adding Custom Shaders

```glsl
// custom_shader.vert
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 transform;
uniform float time;

out vec2 vUV;

void main() {
    vec2 pos = aPos;
    pos.y += sin(pos.x * 10.0 + time) * 0.1;  // Wave effect
    
    vUV = aUV;
    gl_Position = transform * vec4(pos, 0.0, 1.0);
}
```

## 🤝 Contributing

We welcome contributions! Please follow these guidelines:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### Development Setup
```bash
# Clone your fork
git clone https://github.com/yourusername/kiaak-game-engine.git
cd kiaak-game-engine

# Add upstream remote
git remote add upstream https://github.com/Aakarsh911/kiaak-game-engine.git

# Create development build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j4
```

## 🐛 Known Issues & Limitations

- **3D Support**: Currently only supports 2D rendering
- **Audio System**: Audio playback not yet implemented
- **Networking**: No multiplayer or networking features
- **Mobile Platforms**: Desktop platforms only (Windows, macOS, Linux)
- **Asset Pipeline**: Limited asset import/export capabilities

## 📚 Examples & Tutorials

### Simple Platformer Game
Check out the `examples/` directory for a complete platformer game showcasing:
- Player movement with physics
- Collision detection
- Scene transitions
- Lua scripting integration

### Space Shooter
Demonstrates:
- Sprite animation
- Projectile physics
- Enemy AI scripting
- Particle effects

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Dear ImGui** for the excellent immediate mode GUI
- **GLFW** for cross-platform windowing
- **GLM** for mathematics operations
- **Lua** and **Sol2** for scripting integration
- The open-source community for inspiration and support

## 📞 Support & Contact

- **Issues**: [GitHub Issues](https://github.com/Aakarsh911/kiaak-game-engine/issues)
- **Author**: Aakarsh Kaushal
- **Documentation**: [Wiki](https://github.com/Aakarsh911/kiaak-game-engine/wiki) (coming soon)

---

**Happy Game Development! 🎮**