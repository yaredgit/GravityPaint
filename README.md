# Gravity Paint

A physics-based puzzle game where you manipulate gravity to guide objects to their goals.

## Features

- **Intuitive Gravity Control**: Draw strokes to create gravity fields that influence object movement
- **Campaign Mode**: 50 handcrafted levels with increasing difficulty
- **Endless Mode**: Infinite procedurally generated challenges
- **Difficulty Settings**: Easy, Medium, and Hard modes
- **Lives System**: 3 lives per run with Game Over on failure
- **Progress Saving**: Your progress is automatically saved
- **Beautiful Graphics**: Gradient backgrounds, particle effects, and smooth animations

## Platforms

- Windows (Desktop)
- macOS (Desktop)
- Linux (Desktop)
- iOS (Mobile)
- Android (Mobile)
- Web (Browser via WebAssembly)

## Building

### Prerequisites

- CMake 3.16+
- C++17 compatible compiler
- SDL2, SDL2_mixer, SDL2_ttf

### Windows (MSYS2/MinGW)

```bash
# Install dependencies via MSYS2
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-SDL2_ttf

# Build
mkdir build && cd build
cmake -G "Ninja" ..
cmake --build .
```

### Linux

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### macOS

```bash
# Install dependencies via Homebrew
brew install sdl2 sdl2_mixer sdl2_ttf

# Build
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

### Web (Emscripten)

```bash
# Install and activate Emscripten
source /path/to/emsdk/emsdk_env.sh

# Build
mkdir build-web && cd build-web
emcmake cmake ..
emmake make
```

## How to Play

1. Objects spawn at the top of the screen
2. Draw gravity strokes by clicking/touching and dragging
3. Guide all objects into the glowing goal zone
4. Complete levels within the time limit and stroke count
5. Earn stars based on your performance

## Controls

- **Mouse/Touch**: Draw gravity strokes
- **ESC**: Pause game
- **Click**: Navigate menus

## License

MIT License - See [LICENSE](LICENSE) for details.

## Legal

- [Privacy Policy](legal/PRIVACY_POLICY.md)
- [Terms of Service](legal/TERMS_OF_SERVICE.md)
- [Disclaimer](legal/DISCLAIMER.md)

## Credits

Developed with SDL2 and Box2D.

---

© 2024 Gravity Paint. All rights reserved.
