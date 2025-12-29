cmake_minimum_required(VERSION 3.16)
project(Vectoria VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform detection
if(EMSCRIPTEN)
    set(PLATFORM_WEB TRUE)
    message(STATUS "Building for Web (Emscripten/WebAssembly)")
elseif(ANDROID)
    set(PLATFORM_ANDROID TRUE)
    message(STATUS "Building for Android")
elseif(IOS)
    set(PLATFORM_IOS TRUE)
    message(STATUS "Building for iOS")
elseif(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    message(STATUS "Building for Windows")
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
    message(STATUS "Building for macOS")
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
    message(STATUS "Building for Linux")
endif()

# Source files
set(VECTORIA_SOURCES
    src/main.cpp
    src/core/Game.cpp
    src/core/GameState.cpp
    src/core/InputManager.cpp
    src/core/ResourceManager.cpp
    src/physics/PhysicsWorld.cpp
    src/physics/GravityField.cpp
    src/physics/PhysicsObject.cpp
    src/physics/DeformableSurface.cpp
    src/graphics/Renderer.cpp
    src/graphics/ParticleSystem.cpp
    src/graphics/Camera.cpp
    src/ui/HUD.cpp
    src/ui/Menu.cpp
    src/audio/AudioManager.cpp
    src/level/Level.cpp
    src/level/LevelManager.cpp
    src/level/Objective.cpp
)

set(VECTORIA_HEADERS
    include/Vectoria/core/Game.h
    include/Vectoria/core/GameState.h
    include/Vectoria/core/InputManager.h
    include/Vectoria/core/ResourceManager.h
    include/Vectoria/physics/PhysicsWorld.h
    include/Vectoria/physics/GravityField.h
    include/Vectoria/physics/PhysicsObject.h
    include/Vectoria/physics/DeformableSurface.h
    include/Vectoria/graphics/Renderer.h
    include/Vectoria/graphics/ParticleSystem.h
    include/Vectoria/graphics/Camera.h
    include/Vectoria/ui/HUD.h
    include/Vectoria/ui/Menu.h
    include/Vectoria/audio/AudioManager.h
    include/Vectoria/level/Level.h
    include/Vectoria/level/LevelManager.h
    include/Vectoria/level/Objective.h
    include/Vectoria/Types.h
    include/Vectoria/Constants.h
)

# Create executable
add_executable(${PROJECT_NAME} ${VECTORIA_SOURCES} ${VECTORIA_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Emscripten/Web build configuration
if(PLATFORM_WEB)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VECTORIA_WEB)
    
    # Emscripten compiler flags
    set(EMSCRIPTEN_FLAGS
        "-s USE_SDL=2"
        "-s USE_SDL_MIXER=2"
        "-s USE_SDL_IMAGE=2"
        "-s SDL2_IMAGE_FORMATS=[\"png\"]"
        "-s ALLOW_MEMORY_GROWTH=1"
        "-s INITIAL_MEMORY=67108864"
        "-s MAXIMUM_MEMORY=536870912"
        "-s WASM=1"
        "-s ASYNCIFY=1"
        "-s FORCE_FILESYSTEM=1"
        "-s EXPORTED_RUNTIME_METHODS=[\"ccall\",\"cwrap\"]"
        "-s EXPORTED_FUNCTIONS=[\"_main\"]"
        "--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets"
    )
    
    # Join flags for compile and link
    string(REPLACE ";" " " EMSCRIPTEN_FLAGS_STR "${EMSCRIPTEN_FLAGS}")
    
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".html"
        COMPILE_FLAGS "${EMSCRIPTEN_FLAGS_STR}"
        LINK_FLAGS "${EMSCRIPTEN_FLAGS_STR} -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html"
    )
    
    # Box2D for Emscripten
    add_subdirectory(extern/box2d)
    target_link_libraries(${PROJECT_NAME} PRIVATE box2d)

else()
    # Native build configuration
    find_package(SDL2 REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    find_package(OpenGL REQUIRED)

    # Box2D as subdirectory or find package
    option(USE_SYSTEM_BOX2D "Use system-installed Box2D" OFF)
    if(USE_SYSTEM_BOX2D)
        find_package(box2d REQUIRED)
    else()
        add_subdirectory(extern/box2d)
    endif()

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${SDL2_INCLUDE_DIRS}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL2::SDL2
        SDL2::SDL2main
        SDL2_mixer::SDL2_mixer
        OpenGL::GL
        box2d
    )

    # Platform-specific settings
    if(PLATFORM_WINDOWS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE VECTORIA_WINDOWS)
        set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    elseif(PLATFORM_MACOS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE VECTORIA_MACOS)
        set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
    elseif(PLATFORM_ANDROID)
        target_compile_definitions(${PROJECT_NAME} PRIVATE VECTORIA_ANDROID)
    elseif(PLATFORM_IOS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE VECTORIA_IOS)
    elseif(PLATFORM_LINUX)
        target_compile_definitions(${PROJECT_NAME} PRIVATE VECTORIA_LINUX)
    endif()

    # Copy assets to build directory
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Install rules
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(DIRECTORY assets DESTINATION share/vectoria)
