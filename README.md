# Vectoria - Gravity Painter

A physics-based puzzle game where you manipulate gravity vectors to guide objects through dynamic environments.

## Features

- **Real-time Gravity Painting** - Swipe to create temporary gravity fields
- **Multi-body Physics** - Multiple objects interact with emergent behavior
- **Energy Flow System** - Objects transfer energy on collision
- **Deformable Surfaces** - Dynamic surfaces that respond to impact
- **50+ Procedural Levels** - Escalating difficulty with star ratings
- **Cross-platform** - Windows, macOS, Linux, Android, iOS, and Web

## Building

### Prerequisites

- CMake 3.16+
- C++17 compiler
- SDL2, SDL2_mixer
- Box2D (included as submodule)

### Desktop (Windows/macOS/Linux)

```bash
# Clone with submodules
git clone --recursive https://github.com/yaredgit/Vectoria.git
cd Vectoria

# Create build directory
mkdir build && cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Run
./Vectoria
```

### Web (Emscripten/WebAssembly)

```bash
# Install Emscripten SDK first
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
cd ..

# Build for web
cd Vectoria
./web/build-web.sh  # Linux/macOS
# OR
web\build-web.bat   # Windows

# Output in web/dist/
# Serve locally:
cd web/dist
python3 -m http.server 8080
# Open http://localhost:8080
```

### Android

```bash
# Requires Android NDK
mkdir build-android && cd build-android
cmake .. -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
         -DANDROID_ABI=arm64-v8a \
         -DANDROID_PLATFORM=android-21
make
```

## How to Play

1. **Swipe** anywhere on screen to create a gravity field
2. Objects are pulled in the direction you swipe
3. Guide objects to the **green goal zone**
4. Minimize strokes and time for higher scores
5. Create **chain reactions** for bonus points

## Controls

| Platform | Action |
|----------|--------|
| Touch | Swipe to create gravity |
| Mouse | Click and drag |
| Keyboard | ESC to pause |

## Game Mechanics

### Gravity Strokes
- Swipe to paint gravity vectors
- Strokes fade over time (2 seconds)
- Maximum 5 active strokes

### Energy System
- Objects carry energy (visualized as glow)
- Energy transfers on collision
- Higher energy = more points at goal

### Scoring
- Base points for reaching goal
- Time bonus for speed
- Efficiency bonus for fewer strokes
- Combo multiplier for chain reactions

## Architecture

```
Vectoria/
├── src/
│   ├── core/         - Game loop, states, input
│   ├── physics/      - Box2D integration, gravity fields
│   ├── graphics/     - SDL2 rendering, particles
│   ├── ui/           - HUD, menus
│   ├── audio/        - Sound effects, music
│   └── level/        - Level loading, objectives
├── include/          - Header files
├── assets/           - Sprites, sounds, levels
└── web/              - Emscripten build files
```

## Dependencies

- [SDL2](https://www.libsdl.org/) - Cross-platform media layer
- [SDL2_mixer](https://www.libsdl.org/projects/SDL_mixer/) - Audio
- [Box2D](https://box2d.org/) - 2D physics engine

## License

MIT License - See LICENSE file

## Credits

Developed as a cross-platform mobile game demonstration.
