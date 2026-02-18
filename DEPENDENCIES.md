# Dependencies

This project uses the following external libraries:

## Core Dependencies

### GLFW 3.3.8
- **Purpose**: Window creation and input handling
- **License**: zlib
- **Website**: https://www.glfw.org/
- **Description**: A free, open-source, portable framework for OpenGL, OpenGL ES and Vulkan application development

### OpenGL (GLAD)
- **Purpose**: Graphics rendering
- **License**: MIT (GLAD loader)
- **Included**: `libs/glad/`
- **Description**: Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator

### GLM 0.9.9.8
- **Purpose**: Mathematics library (vectors, matrices, quaternions)
- **License**: Happy Bunny License (Modified MIT)
- **Website**: https://glm.g-truc.net/
- **Description**: OpenGL Mathematics - header-only library

### ImGui 1.89.9
- **Purpose**: User interface overlay
- **License**: MIT
- **Website**: https://github.com/ocornut/imgui
- **Description**: Bloat-free graphical user interface library with minimal dependencies

## Optional/Included Dependencies

### stb_image
- **Purpose**: Image loading (for window icon)
- **License**: MIT / Public Domain
- **Included**: `src/stb_image.h`
- **Description**: Single header file image loader

## Building with Package Managers

### Using Conan
```bash
conan install . --build=missing
```

### Using vcpkg
```bash
vcpkg install glfw3:x64-windows glm:x64-windows imgui:x64-windows
```

## Requirements

- **C++ Standard**: C++17 or later
- **OpenGL**: 4.3 or higher
- **Platform**: Windows, Linux, macOS
- **Compiler**: 
  - MSVC 2019 or later (Visual Studio)
  - GCC 9 or later
  - Clang 10 or later

## System Requirements

### Windows (Primary Platform)
- Windows 10 or later
- GPU with OpenGL 4.3+ support
- 512 MB RAM minimum
- 100 MB disk space

### Linux
- GCC/Clang toolchain
- X11 development headers
- GPU driver with OpenGL 4.3+ support

### macOS
- Xcode 12 or later
- GPU with Metal/OpenGL support

## Building

See [README.md](README.md) and [architecture.md](architecture.md) for build instructions.
