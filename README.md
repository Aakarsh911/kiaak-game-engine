# Kiaak Game Engine

**Kiaak** is a lightweight, modern 2D game engine written in C++17 designed for rapid prototyping and game development. It features an integrated visual editor, component-based architecture, and Lua scripting support, making it ideal for indie developers and educational purposes.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)

## 🎮 What is Kiaak Engine?

Kiaak is a complete 2D game development solution that provides everything needed to create games from concept to completion. The engine combines a powerful visual editor with a flexible component-based architecture, enabling developers to build games through both visual interfaces and code.

**Core Philosophy**: Kiaak prioritizes simplicity and rapid iteration while maintaining the power and flexibility needed for serious game development. The engine automatically manages complex systems like rendering, physics, and resource management, allowing developers to focus on creating engaging gameplay.

**Key Capabilities**:
- **Visual Scene Editor** - Drag-and-drop interface for building game scenes with real-time preview
- **Component System** - Modular entity-component architecture for flexible game object design
- **Real-time Rendering** - Hardware-accelerated OpenGL-based 2D graphics pipeline
- **Physics Simulation** - Integrated 2D physics with collision detection and response
- **Scripting Support** - Lua integration for gameplay logic and rapid prototyping
- **Asset Management** - Project-based workflow with organized asset handling and hot-reloading
- **Cross-platform** - Runs natively on Windows, macOS, and Linux

## ✨ Key Features

### 🎨 Visual Editor
The built-in editor provides a complete development environment without requiring external tools:

- **Scene Hierarchy** - Tree view of all game objects with parent-child relationships
- **Inspector Panel** - Real-time component property editing with immediate visual feedback
- **Asset Browser** - Centralized management of textures, scripts, and other resources
- **Play/Edit Modes** - Test your game instantly without leaving the editor
- **Camera Controls** - Intuitive pan, zoom, and navigation with mouse and keyboard
- **Gizmo Tools** - Visual transform handles for precise object positioning and manipulation
- **Project Manager** - Create and switch between multiple game projects seamlessly

### 🧩 Component System
Kiaak uses a flexible entity-component architecture that allows for modular game object design:

- **Transform Component** - 3D position, rotation, and scale with hierarchical transformations
- **Sprite Renderer** - 2D texture rendering with transparency, tinting, and layer sorting
- **Camera Component** - Orthographic cameras with configurable zoom, viewport, and culling
- **Rigidbody2D** - Full physics simulation with mass, velocity, and force application
- **Collider2D** - Collision detection with box, circle, and polygon collision shapes
- **Script Component** - Lua scripting integration for custom behaviors and game logic
- **Tilemap System** - Efficient grid-based level creation with collision layer support
- **Animator** - Frame-based animation system with customizable sequences and timing

### 🎯 Core Engine Systems
The engine provides robust foundational systems that handle complex game development tasks:

- **Scene Management** - Multiple scenes with seamless loading, switching, and persistence
- **Project System** - Organized folder structure with automatic asset discovery and management
- **Serialization** - Human-readable text format for scenes and project data
- **Input Handling** - Cross-platform keyboard and mouse input with customizable key bindings
- **Timer System** - Precise frame timing with both fixed timestep and variable timestep updates
- **2D Physics** - Collision detection, rigid body dynamics, and constraint solving
- **Shader System** - Custom GLSL shader support with uniform parameter binding
- **Resource Management** - Automatic texture loading, caching, and memory management

### 💻 Scripting & Extensibility
Kiaak provides powerful scripting capabilities that make gameplay programming accessible:

- **Lua Integration** - Full-featured Lua 5.4 support with modern C++ bindings via Sol2
- **Hot Reloading** - Modify and reload scripts without restarting the editor or game
- **Component Scripting** - Attach custom behaviors to any game object with lifecycle callbacks
- **Engine API Access** - Complete access to engine systems including physics, input, and rendering
- **Built-in Functions** - Pre-defined Lua functions for common game operations like logging and physics queries
- **Script Editor** - Integrated syntax-highlighted script editor with immediate testing capabilities

## 🛠️ Technical Architecture

### Engine Architecture
Kiaak is built with a modular architecture that separates concerns and enables easy maintenance and extension:

```
Kiaak Engine
├── Core/                    # Fundamental engine systems
│   ├── Engine.cpp          # Main engine loop, initialization, and coordination
│   ├── Scene.cpp           # Scene management and game object containers
│   ├── GameObject.cpp      # Entity system with component management
│   ├── Transform.cpp       # Hierarchical position, rotation, and scale
│   ├── Camera.cpp          # Orthographic camera system with viewport control
│   ├── Timer.cpp           # Frame timing, delta time, and fixed timestep
│   ├── SceneManager.cpp    # Multi-scene handling and transitions
│   └── Project.cpp         # Project structure and asset path management
├── Graphics/               # Rendering and visual systems
│   ├── Renderer.cpp        # OpenGL rendering pipeline and draw calls
│   ├── Shader.cpp          # GLSL shader compilation and management
│   ├── Texture.cpp         # Image loading, caching, and GPU upload
│   ├── SpriteRenderer.cpp  # 2D sprite rendering component
│   ├── VertexArray.cpp     # OpenGL vertex array object abstraction
│   └── VertexBuffer.cpp    # GPU buffer management
├── Editor/                 # Visual editor and tools
│   ├── EditorCore.cpp      # Editor initialization and system coordination
│   └── EditorUI.cpp        # ImGui-based user interface and panels
├── Physics/                # 2D physics simulation
│   ├── Physics2D.cpp       # Physics world simulation and collision detection
│   ├── Rigidbody2D.cpp     # Dynamic physics bodies with forces and constraints
│   └── Collider2D.cpp      # Collision shapes and contact generation
└── Scripting/              # Lua integration and script management
    ├── ScriptComponent.cpp # Script attachment and lifecycle management
    └── LuaBindings.cpp     # Engine API exposure to Lua scripts
```

### Runtime Systems
The engine operates on a multi-threaded architecture with distinct phases:

1. **Initialization Phase** - System startup, OpenGL context creation, and resource loading
2. **Game Loop** - Fixed timestep physics updates and variable timestep rendering
3. **Input Processing** - Event handling and input state management
4. **Physics Simulation** - Collision detection, constraint solving, and integration
5. **Script Execution** - Lua script callbacks and engine API interactions
6. **Rendering Pipeline** - GPU command generation and frame presentation
7. **Editor Updates** - UI rendering and editor-specific functionality

### Dependencies and Technologies
Kiaak is built on proven, stable libraries that provide cross-platform compatibility:

- **GLFW 3.3+** - Cross-platform windowing, input handling, and OpenGL context management
- **OpenGL 3.3+** - Hardware-accelerated graphics rendering with modern shader support
- **ImGui** - Immediate mode GUI library for the visual editor interface
- **GLM** - OpenGL Mathematics library for vector and matrix operations
- **Lua 5.4** - Fast, lightweight scripting language with excellent C++ integration
- **Sol2** - Modern C++ to Lua binding library with type safety and performance
- **GLAD** - OpenGL function loading library for cross-platform compatibility

## 📖 Usage Guide

### Creating Your First Project

Kiaak follows a project-based workflow that automatically organizes your game assets and scenes:

1. **Launch the Engine**: Run the KiaakEngine executable to open the editor
2. **Automatic Project Structure**: The engine creates a structured project layout:
   - `assets/` - Store all game resources (textures, scripts, shaders)
   - `scenes/` - Scene files in human-readable text format
   - `last_project.txt` - Automatically saves your current project path
3. **Scene Editor**: Use the visual editor to build your game world with drag-and-drop simplicity

### Basic Workflow

The typical development workflow in Kiaak involves creating game objects, adding components, and writing scripts:

```cpp
// Example: Creating a player character with multiple components
auto* player = scene->CreateGameObject("Player");

// Transform is automatically added to every game object
auto* transform = player->GetTransform();
transform->SetPosition(0.0f, 0.0f, 0.0f);
transform->SetScale(2.0f, 2.0f, 1.0f);

// Add visual representation
auto* spriteRenderer = player->AddComponent<SpriteRenderer>();
spriteRenderer->SetTexture("assets/textures/player_sprite.png");
spriteRenderer->SetColor(1.0f, 1.0f, 1.0f, 1.0f);

// Add physics simulation
auto* rigidbody = player->AddComponent<Rigidbody2D>();
rigidbody->SetMass(1.0f);
rigidbody->SetGravityScale(1.0f);

// Add collision detection
auto* collider = player->AddComponent<Collider2D>();
collider->SetSize(1.0f, 1.5f);  // Width and height
collider->SetIsTrigger(false);  // Physical collision

// Add custom behavior through scripting
auto* script = player->AddComponent<ScriptComponent>();
script->SetScriptPath("assets/scripts/player_controller.lua");
```

### Lua Scripting Example

Kiaak's Lua integration provides full access to engine systems with simple, readable syntax:

```lua
-- player_controller.lua - Example player movement script
local Player = {}

function Player:Start()
    -- Initialize player properties
    self.speed = 250.0  -- pixels per second
    self.jumpForce = 500.0
    self.grounded = false
    
    -- Get component references for efficiency
    self.transform = self.gameObject:GetTransform()
    self.rigidbody = self.gameObject:GetComponent("Rigidbody2D")
    
    -- Log initialization
    log("Player controller initialized")
end

function Player:Update(deltaTime)
    -- Handle horizontal movement
    local horizontalInput = 0
    if IsKeyPressed("A") or IsKeyPressed("Left") then
        horizontalInput = horizontalInput - 1
    end
    if IsKeyPressed("D") or IsKeyPressed("Right") then
        horizontalInput = horizontalInput + 1
    end
    
    -- Apply movement force
    if horizontalInput ~= 0 then
        local force = horizontalInput * self.speed * deltaTime
        self.rigidbody:AddForce(force, 0)
    end
    
    -- Handle jumping
    if (IsKeyPressed("W") or IsKeyPressed("Space")) and self.grounded then
        self.rigidbody:AddForce(0, self.jumpForce)
        self.grounded = false
    end
    
    -- Check for ground collision (simplified)
    local contacts = GetPhysicsContacts()
    for _, contact in ipairs(contacts) do
        if contact.aID == self.gameObject:GetID() or contact.bID == self.gameObject:GetID() then
            -- Simple ground detection based on contact normal
            if contact.normal[2] > 0.7 then
                self.grounded = true
            end
        end
    end
end

function Player:OnCollisionEnter(other)
    log("Player collided with: " .. other:GetName())
end

return Player
```

### Advanced Features

#### Tilemap System
Create efficient 2D levels using Kiaak's built-in tilemap system:

```cpp
// Create a tilemap for level geometry
auto* levelMap = scene->CreateGameObject("LevelMap");
auto* tilemap = levelMap->AddComponent<Tilemap>();
tilemap->SetTileSize(32, 32);  // 32x32 pixel tiles
tilemap->LoadTileTexture("assets/textures/tileset.png");

// Paint tiles at specific grid positions
tilemap->SetTile(0, 0, 1);  // Set tile at grid position (0,0) to tile index 1
tilemap->SetTile(1, 0, 2);  // Different tile type
tilemap->SetCollisionAtTile(0, 0, true);  // Make this tile solid
```

#### Animation System
Animate sprites using frame-based sequences:

```cpp
// Add animation capability to a game object
auto* animator = gameObject->AddComponent<Animator>();
animator->LoadAnimationSheet("assets/textures/character_walk.png", 4, 2);  // 4x2 grid
animator->CreateClip("walk", {0, 1, 2, 3}, 8.0f);  // 8 FPS animation
animator->CreateClip("idle", {4, 5}, 2.0f);        // Slower idle animation
animator->PlayClip("walk");
```

## 🎮 Editor Controls

### Camera Navigation
Kiaak provides intuitive camera controls for scene navigation:

- **Middle Mouse + Drag**: Pan the camera around the scene
- **Mouse Wheel**: Zoom in and out (maintains cursor position)
- **Space**: Reset camera to origin position and default zoom
- **F**: Focus camera on selected object

### Object Manipulation
Select and manipulate objects directly in the scene view:

- **Left Click**: Select individual objects (highlights in hierarchy)
- **Ctrl + Left Click**: Multi-select objects
- **G + Drag**: Grab and move selected objects
- **R + Drag**: Rotate selected objects around their center
- **S + Drag**: Scale selected objects uniformly
- **Delete**: Remove selected objects from the scene

### Editor Modes and Shortcuts
Switch between different editor states and workflows:

- **Tab**: Toggle between Edit Mode (scene editing) and Play Mode (game testing)
- **Ctrl + S**: Save current scene to disk
- **Ctrl + O**: Open existing scene file
- **Ctrl + N**: Create new empty scene
- **Ctrl + D**: Duplicate selected objects
- **Ctrl + Z**: Undo last action (limited support)

## 📁 Project Structure

Kiaak automatically organizes your game files in a logical, source-control-friendly structure:

```
MyGameProject/
├── assets/                    # All game resources
│   ├── textures/             # Image files (.png, .jpg, .tga)
│   │   ├── characters/       # Organized by category
│   │   ├── environments/     # Environment artwork
│   │   └── ui/              # User interface graphics
│   ├── scripts/             # Lua script files (.lua)
│   │   ├── components/      # Reusable component scripts
│   │   ├── systems/         # Game system scripts
│   │   └── utilities/       # Helper functions and libraries
│   ├── shaders/             # Custom GLSL shaders
│   │   ├── vertex/          # Vertex shaders (.vert)
│   │   └── fragment/        # Fragment shaders (.frag)
│   └── audio/               # Sound files (future feature)
├── scenes/                   # Scene definition files
│   ├── MainMenu.scene       # Main menu scene
│   ├── Level01.scene        # Game level scenes
│   ├── Level02.scene        # Additional levels
│   └── GameOver.scene       # End game scene
└── last_project.txt          # Automatically maintained project state
```

### Scene File Format
Kiaak uses human-readable text files for scene data, making version control and debugging easier:

```
SCENE MainLevel
  ACTIVE_CAMERA MainCamera
  PHYSICS2D gravity 0.0 -9.81

  GAMEOBJECT Player
    TRANSFORM pos 0.0 0.0 0.0 rot 0.0 0.0 0.0 scale 1.0 1.0 1.0
    SPRITE texture assets/textures/player.png
    RIGIDBODY2D mass 1.0 gravityScale 1.0
    COLLIDER2D size 1.0 1.5
    SCRIPT path assets/scripts/player_controller.lua

  GAMEOBJECT Ground
    TRANSFORM pos 0.0 -5.0 0.0 rot 0.0 0.0 0.0 scale 10.0 1.0 1.0
    SPRITE texture assets/textures/ground.png
    COLLIDER2D size 10.0 1.0 static true
```

## 🔧 Extending the Engine

### Creating Custom Components

Kiaak's component system is designed for easy extension. Here's how to create a custom component:

```cpp
// HealthComponent.hpp - Custom health management component
#pragma once
#include "Core/Component.hpp"

namespace Kiaak::Core {
    class HealthComponent : public Component {
    public:
        HealthComponent(int maxHealth = 100);
        
        // Component lifecycle
        void Start() override;
        void Update(double deltaTime) override;
        
        // Health management
        void TakeDamage(int damage);
        void Heal(int amount);
        bool IsAlive() const { return currentHP > 0; }
        float GetHealthPercentage() const { return (float)currentHP / maxHP; }
        
        // Serialization support
        void Serialize(std::ostream& out) const override;
        void Deserialize(std::istream& in) override;
        
    private:
        int maxHP;
        int currentHP;
        bool invulnerable;
        float invulnerabilityTimer;
        
        // Event callbacks
        void OnDeath();
        void OnDamageTaken(int damage);
    };
}
```

### Custom Shader Development

Create custom visual effects using GLSL shaders:

```glsl
// wave_effect.vert - Vertex shader for wave animation
#version 330 core
layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;
uniform float uTime;
uniform float uWaveAmplitude;
uniform float uWaveFrequency;

out vec2 vTexCoord;

void main() {
    vec2 position = aPosition;
    
    // Apply wave effect to vertex position
    position.y += sin(position.x * uWaveFrequency + uTime) * uWaveAmplitude;
    
    vTexCoord = aTexCoord;
    gl_Position = uProjection * uView * uModel * vec4(position, 0.0, 1.0);
}
```

```glsl
// wave_effect.frag - Fragment shader for wave animation
#version 330 core
in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform float uTime;
uniform vec3 uTintColor;
uniform float uAlpha;

out vec4 fragColor;

void main() {
    vec4 textureColor = texture(uTexture, vTexCoord);
    
    // Apply color tinting and transparency
    vec3 finalColor = textureColor.rgb * uTintColor;
    float finalAlpha = textureColor.a * uAlpha;
    
    // Add subtle color animation
    finalColor += sin(uTime * 2.0) * 0.1;
    
    fragColor = vec4(finalColor, finalAlpha);
}
```

### Engine Lua API Reference

Kiaak exposes a comprehensive API to Lua scripts for game development:

```lua
-- Core Engine Functions
log(message)                           -- Print to console
GetDeltaTime()                        -- Get frame delta time
GetTime()                            -- Get total elapsed time

-- Input System
IsKeyPressed(key)                     -- Check if key is currently pressed
IsKeyReleased(key)                    -- Check if key was just released
GetMousePosition()                    -- Get mouse coordinates
IsMouseButtonPressed(button)          -- Check mouse button state

-- Physics System
GetPhysicsContacts()                  -- Get all current collision contacts
SetGravity(x, y)                     -- Set world gravity

-- Game Object Management
CreateGameObject(name)                -- Create new game object
FindGameObject(name)                  -- Find existing game object
DestroyGameObject(gameObject)         -- Remove game object

-- Transform Operations
transform:SetPosition(x, y, z)        -- Set world position
transform:GetPosition()               -- Get current position
transform:SetRotation(x, y, z)        -- Set rotation in degrees
transform:SetScale(x, y, z)          -- Set scale factors

-- Component Access
gameObject:GetComponent(typeName)     -- Get component by type name
gameObject:AddComponent(typeName)     -- Add new component
gameObject:RemoveComponent(typeName)  -- Remove component
```

## 🎯 Engine Capabilities and Limitations

### What Kiaak Excels At
- **Rapid 2D Game Prototyping** - Get gameplay ideas running quickly with minimal setup
- **Educational Game Development** - Clear architecture and Lua scripting make learning accessible
- **Indie Game Development** - Full feature set without overwhelming complexity
- **Visual Novel and Adventure Games** - Strong scene management and scripting support
- **Platformer and Action Games** - Robust physics and input handling
- **Puzzle and Strategy Games** - Flexible component system and turn-based logic support

### Current Limitations
- **3D Rendering** - Currently focused exclusively on 2D graphics and gameplay
- **Audio System** - Sound effects and music playback not yet implemented
- **Networking** - No built-in multiplayer or network communication features
- **Mobile Platforms** - Desktop platforms only (Windows, macOS, Linux)
- **Advanced Physics** - Basic 2D physics without complex constraints or soft bodies
- **Asset Pipeline** - Limited import/export capabilities for complex asset formats

### Performance Characteristics
- **Target Platform** - Modern desktop computers with dedicated graphics cards
- **Rendering Performance** - Handles thousands of sprites at 60+ FPS on mid-range hardware
- **Memory Usage** - Lightweight footprint, typically under 100MB for most projects
- **Startup Time** - Fast cold start, typically under 2 seconds to editor
- **Build Size** - Compiled games are small, usually under 50MB including engine
