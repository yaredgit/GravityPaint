cmake_minimum_required(VERSION 3.16)
project(GravityPaint VERSION 1.0.0 LANGUAGES CXX)

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
set(GRAVITYPAINT_SOURCES
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

set(GRAVITYPAINT_HEADERS
    include/GravityPaint/core/Game.h
    include/GravityPaint/core/GameState.h
    include/GravityPaint/core/InputManager.h
    include/GravityPaint/core/ResourceManager.h
    include/GravityPaint/physics/PhysicsWorld.h
    include/GravityPaint/physics/GravityField.h
    include/GravityPaint/physics/PhysicsObject.h
    include/GravityPaint/physics/DeformableSurface.h
    include/GravityPaint/graphics/Renderer.h
    include/GravityPaint/graphics/ParticleSystem.h
    include/GravityPaint/graphics/Camera.h
    include/GravityPaint/ui/HUD.h
    include/GravityPaint/ui/Menu.h
    include/GravityPaint/audio/AudioManager.h
    include/GravityPaint/level/Level.h
    include/GravityPaint/level/LevelManager.h
    include/GravityPaint/level/Objective.h
    include/GravityPaint/Types.h
    include/GravityPaint/Constants.h
)

# Create executable
add_executable(${PROJECT_NAME} ${GRAVITYPAINT_SOURCES} ${GRAVITYPAINT_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Emscripten/Web build configuration
if(PLATFORM_WEB)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GRAVITYPAINT_WEB)
    
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
    
    string(REPLACE ";" " " EMSCRIPTEN_FLAGS_STR "${EMSCRIPTEN_FLAGS}")
    
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".html"
        COMPILE_FLAGS "${EMSCRIPTEN_FLAGS_STR}"
        LINK_FLAGS "${EMSCRIPTEN_FLAGS_STR} -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html"
    )
    
    # Disable Box2D extras that don't work with Emscripten
    set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
    set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
    add_subdirectory(extern/box2d)
    target_link_libraries(${PROJECT_NAME} PRIVATE box2d)

elseif(PLATFORM_WINDOWS)
    # Windows build
    target_compile_definitions(${PROJECT_NAME} PRIVATE GRAVITYPAINT_WINDOWS)
    
    # Check if using MinGW (MSYS2)
    if(MINGW)
        message(STATUS "Using MinGW - linking against system SDL2 from MSYS2")
        # Use SDL2 from MSYS2/MinGW64
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(SDL2 REQUIRED sdl2)
        pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
        pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
        
        target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES} mingw32)
        target_link_directories(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARY_DIRS} ${SDL2_MIXER_LIBRARY_DIRS} ${SDL2_TTF_LIBRARY_DIRS})
        
        # For SDL2, we need to link with SDL2main for proper WinMain
        target_link_libraries(${PROJECT_NAME} PRIVATE SDL2main)
    else()
        # MSVC build with local SDL2 libraries
        set(SDL2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/SDL2-2.30.9")
        set(SDL2_MIXER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/SDL2_mixer-2.8.0")
        
        if(EXISTS "${SDL2_DIR}" AND EXISTS "${SDL2_MIXER_DIR}")
            message(STATUS "Using local SDL2 from deps folder")
            
            # SDL2
            target_include_directories(${PROJECT_NAME} PRIVATE "${SDL2_DIR}/include")
            target_link_directories(${PROJECT_NAME} PRIVATE "${SDL2_DIR}/lib/x64")
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 SDL2main)
            
            # SDL2_mixer
            target_include_directories(${PROJECT_NAME} PRIVATE "${SDL2_MIXER_DIR}/include")
            target_link_directories(${PROJECT_NAME} PRIVATE "${SDL2_MIXER_DIR}/lib/x64")
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_mixer)
            
            # Copy DLLs to output
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SDL2_DIR}/lib/x64/SDL2.dll"
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SDL2_MIXER_DIR}/lib/x64/SDL2_mixer.dll"
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
            
            # Copy any additional DLLs SDL2_mixer needs
            file(GLOB MIXER_DLLS "${SDL2_MIXER_DIR}/lib/x64/*.dll")
            foreach(DLL ${MIXER_DLLS})
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${DLL}"
                        $<TARGET_FILE_DIR:${PROJECT_NAME}>
                )
            endforeach()
        else()
            # Try to find SDL2 via find_package (vcpkg or system)
            find_package(SDL2 REQUIRED)
            find_package(SDL2_mixer REQUIRED)
            target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
            target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_mixer::SDL2_mixer)
        endif()
    endif()
    
    # OpenGL
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
    
    # Box2D
    add_subdirectory(extern/box2d)
    target_link_libraries(${PROJECT_NAME} PRIVATE box2d)
    
    # Windows subsystem
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

else()
    # Linux/macOS - use system packages
    find_package(SDL2 REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    find_package(OpenGL REQUIRED)
    
    add_subdirectory(extern/box2d)
    
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL2::SDL2
        SDL2::SDL2main
        SDL2_mixer::SDL2_mixer
        OpenGL::GL
        box2d
    )
    
    if(PLATFORM_MACOS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE GRAVITYPAINT_MACOS)
        set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
    elseif(PLATFORM_LINUX)
        target_compile_definitions(${PROJECT_NAME} PRIVATE GRAVITYPAINT_LINUX)
    endif()
endif()

# Copy assets to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
)

# Install rules
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(DIRECTORY assets DESTINATION share/gravitypaint)
