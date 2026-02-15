# Architecture: High-Performance Generalized Lenia (C++/OpenGL)

## 1. Project Philosophy & Objectives

This project implements a real-time, high-performance Lenia continuous cellular automaton simulation in C++ with OpenGL compute shaders.

**Core Principles:**
- **Zero-Copy Simulation:** The entire state resides in VRAM. The CPU never reads the grid data back unless explicitly requested for file I/O or analysis.
- **Generalized Modularity:** The architecture abstracts Kernel, Growth Function, and Topology to support Classic Lenia, Multi-Channel Lenia, and extended variants.
- **Compute-Driven:** Logic is executed via OpenGL 4.5+ Compute Shaders using Image Load/Store for massive parallel throughput.
- **Data-Oriented Design:** Data structures are aligned to GPU memory standards (`std140`/`std430`) to minimize padding and upload overhead.

## 2. Technology Stack

| Component | Technology |
|-----------|------------|
| Core Language | C++20 (Concepts, smart pointers, strict type safety) |
| Graphics API | OpenGL 4.5/4.6 Core Profile with Compute Shaders |
| Windowing | GLFW 3.3+ |
| Extension Loader | GLAD (OpenGL 4.6 Core) |
| UI Framework | Dear ImGui (Docking Branch) |
| Mathematics | GLM (OpenGL Mathematics) |
| Build System | Custom batch script (Windows), Makefile (Linux) |

## 3. Directory Structure

```
Lenia/
├── src/                        # Source files
│   ├── Main.cpp               # Entry point, window creation
│   ├── Application.hpp/cpp    # Main application loop, input handling
│   ├── LeniaEngine.hpp/cpp    # Core simulation orchestrator
│   ├── SimulationState.hpp/cpp # Ping-pong texture management
│   ├── KernelManager.hpp/cpp  # Convolution kernel generation
│   ├── Renderer.hpp/cpp       # Display rendering, colormaps
│   ├── UIOverlay.hpp/cpp      # ImGui interface, all UI sections
│   ├── AnalysisManager.hpp/cpp # Pattern analysis, statistics
│   ├── Presets.hpp/cpp        # Species presets and configurations
│   ├── PresetData.inc         # Embedded preset data
│   ├── AnimalData.inc         # Species cell data arrays
│   ├── EmitterPresetData.hpp  # Emitter pattern configurations
│   └── Utils/                 # Utility classes
│       ├── Shader.hpp/cpp     # Shader compilation and management
│       ├── Logger.hpp         # Logging utilities
│       ├── GLUtils.hpp        # OpenGL helper functions
│       └── NpyLoader.hpp/cpp  # NumPy .npy file loader
├── assets/
│   ├── shaders/               # GLSL shaders
│   │   ├── sim_spatial.comp   # Main single-channel simulation
│   │   ├── sim_multichannel.comp # Multi-channel simulation
│   │   ├── sim_noise.comp     # Noise/initialization patterns
│   │   ├── kernel_gen.comp    # Kernel texture generation
│   │   ├── analysis.comp      # Grid analysis compute shader
│   │   ├── display.vert       # Fullscreen quad vertex shader
│   │   └── display.frag       # Colormap/visualization fragment shader
│   ├── init/                  # Initial state files
│   └── icon.png               # Application icon (256x256 RGBA PNG)
├── Initialisation/            # Species .npy files
├── colormap/                  # Custom colormap images
├── libs/                      # External libraries (GLFW, GLAD, ImGui, GLM)
├── bin/                       # Build output
├── obj/                       # Object files
├── build_windows.bat          # Windows build script
└── Makefile                   # Linux/Unix build
```

## 4. Core Architecture

### 4.1 Class Hierarchy

```
Application (Main Loop Owner)
├── GLFWwindow* (Window handle)
├── LeniaEngine (Simulation Core)
│   ├── SimulationState (Ping-pong textures)
│   ├── KernelManager (Main kernel)
│   ├── KernelManager[16] (Per-rule kernels for multi-channel)
│   ├── Renderer (Display pipeline)
│   ├── AnalysisManager (Pattern analysis)
│   └── Shader objects (Compute shaders)
├── UIOverlay (ImGui Interface)
│   ├── Section Windows (Detachable UI sections)
│   └── Callbacks (Engine interaction)
└── LeniaParams (All simulation parameters)
```

### 4.2 Data Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                        MAIN LOOP                                │
├─────────────────────────────────────────────────────────────────┤
│  1. Input Processing                                            │
│     └─> LeniaParams updated via UI or keyboard                  │
│                                                                 │
│  2. Kernel Regeneration (if params changed)                     │
│     └─> KernelManager::generate() -> kernel_gen.comp            │
│                                                                 │
│  3. Simulation Step (if not paused)                             │
│     ├─> Single-channel: sim_spatial.comp                        │
│     │   - Reads: currentTexture, kernelTexture, wallTexture     │
│     │   - Writes: nextTexture                                   │
│     │   - SimulationState::swap()                               │
│     └─> Multi-channel: sim_multichannel.comp                    │
│         - Iterates through kernel rules                         │
│         - Each rule: source_channel -> dest_channel             │
│                                                                 │
│  4. Analysis (if enabled)                                       │
│     └─> AnalysisManager::analyze() -> analysis.comp             │
│         - Computes: mass, centroid, alive count, bounds         │
│         - Detects: stability, periodicity, emptiness            │
│                                                                 │
│  5. Rendering                                                   │
│     └─> Renderer::draw()                                        │
│         - display.vert: Fullscreen quad                         │
│         - display.frag: State -> Colormap -> Screen             │
│         - Modes: World, NeighborSums, Growth, Kernel, Delta     │
│                                                                 │
│  6. UI Rendering                                                │
│     └─> UIOverlay::render()                                     │
│         - Main window with collapsible sections                 │
│         - Detached section windows                              │
│         - Pause/play overlay animation                          │
└─────────────────────────────────────────────────────────────────┘
```

### 4.3 Double-Buffered State (Ping-Pong)

```
Frame N:                          Frame N+1:
┌──────────┐    ┌──────────┐     ┌──────────┐    ┌──────────┐
│ Texture A│    │ Texture B│     │ Texture A│    │ Texture B│
│ (Current)│───>│  (Next)  │     │  (Next)  │<───│ (Current)│
│   READ   │    │  WRITE   │     │  WRITE   │    │   READ   │
└──────────┘    └──────────┘     └──────────┘    └──────────┘
      After swap(), pointers exchange
```

**Texture Formats:**
- Single-channel: `GL_R32F` (32-bit float red channel)
- Multi-channel: `GL_RGBA32F` (4x 32-bit float for RGB + alpha/walls)
- Kernel: `GL_R32F` (normalized weights)
- Walls: `GL_RGBA32F` (RGBA for wall properties)

## 5. Simulation Engine Details

### 5.1 LeniaEngine

The central orchestrator managing all simulation components.

**Key Methods:**
- `update(params, steps)` - Single-channel simulation steps
- `updateMultiChannel(params, steps)` - Multi-channel with kernel rules
- `render(viewportW, viewportH, params)` - Visualization
- `reset(params)` - Initialize with species/noise
- `regenerateKernel(params)` - Rebuild kernel texture
- `applyBrush/Wall(x, y, params)` - Interactive drawing

**GPU Uniform Buffers:**
```cpp
struct GPUSimParams {          // For sim_spatial.comp
    int32_t gridW, gridH;
    int32_t radius;
    float   dt, mu, sigma;
    int32_t growthType;
    // + padding for std140
};

struct GPUMultiChannelParams { // For sim_multichannel.comp
    int32_t gridW, gridH, radius;
    float   dt, mu, sigma;
    int32_t growthType;
    int32_t sourceChannel, destChannel;
    float   growthStrength;
    int32_t rulePass, numRules;
};
```

### 5.2 SimulationState

Manages the ping-pong texture pair.

**Operations:**
- `init(width, height, format)` - Create texture pair
- `swap()` - Exchange current/next pointers
- `uploadRegion(x, y, w, h, data)` - CPU -> GPU transfer for brush/species
- `clear()` - Zero all cells

### 5.3 KernelManager

Generates convolution kernels via compute shader.

**Supported Kernel Types:**
1. Gaussian Shell - Standard Lenia
2. Bump4 - Polynomial bump function
3. Multi-ring Gaussian - Multiple concentric rings
4. Multi-ring Bump4 - Multi-ring with bump function
5. Game of Life - Moore neighborhood
6. Step Unimodal - Sharp-edged kernel
7. Cosine Shell - Cosine-based falloff
8. Mexican Hat - Center-surround (DoG)
9. Quad4 - Polynomial kernel
10. Multi-ring Quad4 - Multi-ring polynomial

### 5.4 AnalysisManager

Real-time pattern analysis via compute shader reduction.

**Computed Metrics:**
- Total mass (sum of all cell values)
- Alive count (cells above threshold)
- Centroid position (weighted center)
- Bounding box (min/max extent)
- Movement speed and direction
- Stability detection (unchanging mass)
- Periodicity detection (repeating patterns)

## 6. Shader Architecture

### 6.1 sim_spatial.comp (Single-Channel)

```glsl
layout(local_size_x = 16, local_size_y = 16) in;

// Shared memory tile for efficient convolution
shared float tile[TILE_SIZE + 2*MAX_R][TILE_SIZE + 2*MAX_R];

void main() {
    // 1. Cooperative tile loading (with halo)
    // 2. Barrier synchronization
    // 3. Convolution against kernel texture
    // 4. Growth function application
    // 5. Wall interaction
    // 6. Write to output texture
}
```

### 6.2 sim_multichannel.comp

Extends single-channel with:
- Source/destination channel selection
- Per-rule kernel texture binding
- Growth strength modulation
- Channel-specific wall effects

### 6.3 display.frag

Multiple visualization modes:
- **Mode 0 (World):** Apply colormap to cell states
- **Mode 1 (Neighbor Sums):** Show convolution potential
- **Mode 2 (Growth):** Show growth rate field
- **Mode 3 (Kernel):** Display kernel shape
- **Mode 4 (Delta):** Show change per step

## 7. UI System

### 7.1 UIOverlay Architecture

The UI is organized into collapsible sections, each can be detached into its own window.

**Sections:**
1. **Info** - Cursor position, grid stats, keybinds, theory documentation
2. **Performance** - FPS, frame timing, throughput metrics
3. **Grid** - Size, transformations, edge conditions
4. **Brush & Walls** - Interactive drawing tools
5. **Presets & Initialization** - Species selection, placement modes
6. **Simulation** - Pause, steps/frame control
7. **Growth Function** - Growth type, mu, sigma, dt parameters
8. **Kernel** - Kernel type, radius, ring weights
9. **Display** - Colormap, zoom, pan, visual effects
10. **Analysis** - Mass/alive graphs, movement tracking

### 7.2 Detachable Sections

Each section has a pop-out button (⬚) that moves it to a separate ImGui window, allowing flexible layout customization. Click again to re-dock.

### 7.3 Pause/Play Overlay

Visual feedback system:
- Large play (▶) or pause (⏸) icon appears in top-right corner
- Smooth fade-out animation over ~1 second
- Triggers on: Space key, autopause activation, UI toggle

## 8. Parameter System

### 8.1 LeniaParams Structure

Central parameter store (~250 fields) including:

**Simulation:**
- mu, sigma, dt, radius, numRings, ringWeights[]
- kernelType, growthType
- gridW, gridH

**Multi-Channel:**
- numChannels, numKernelRules
- kernelRules[16] (source, dest, strength, private params)

**Display:**
- zoom, panX, panY
- colormapMode, brightness, contrast, gamma
- displayMode, filterMode

**Brush:**
- brushShape, brushSize, brushStrength, brushFalloff
- brushMode, brushPattern, brushDrawMode
- brushSymmetryX/Y/Radial

**Walls:**
- wallEnabled, wallType, wallColor
- wallThickness, wallDamping, wallPermeability

**Edge Conditions:**
- edgeModeX/Y (Periodic, Clamp, Mirror)
- edgeFadeX/Y

**Analysis:**
- autoPause, showMassGraph, showAliveGraph
- analysisThreshold

## 9. Build System

### Windows (build_windows.bat)

```batch
g++ -std=c++20 -O2 -Wall -Wextra ^
    -Ilibs/include -Ilibs/imgui -Ilibs/glad/include ^
    src/*.cpp libs/glad/src/glad.c libs/imgui/*.cpp ^
    -o bin/Lenia.exe ^
    -Llibs/lib -lglfw3 -lopengl32 -lgdi32
```

### Linux (Makefile)

Standard recursive make with automatic dependency generation.

## 10. Extension Points

### Adding a New Growth Function
1. Add enum value to `GrowthType`
2. Implement in `sim_spatial.comp` switch statement
3. Add UI entry in `UIOverlay.cpp`

### Adding a New Kernel Type
1. Add enum value to `KernelType`
2. Implement in `kernel_gen.comp`
3. Add UI entry and parameters

### Adding a New Preset/Species
1. Add entry to `PresetData.inc`
2. If cell data needed, add to `AnimalData.inc`
3. Or place .npy file in `Initialisation/`

## 11. Performance Considerations

**GPU Optimization:**
- Shared memory tiling in compute shaders
- Coalesced memory access patterns
- Minimal CPU-GPU synchronization
- Immutable texture storage (`glTexStorage2D`)

**Typical Performance:**
- 512x512 grid, R=13: ~60+ FPS (RTX 3080)
- 1024x1024 grid, R=26: ~30 FPS
- GPU bottleneck: Kernel convolution (O(N²×R²))

## 12. File Formats

### Species Files (.npy)
NumPy array format containing cell state data:
- Single-channel: Shape (H, W), dtype float32
- Multi-channel: Shape (C, H, W) or (H, W, C)

### Colormap Files
PNG images (256×1 or 256×N pixels):
- Horizontal gradient from value 0 (left) to 1 (right)
- Placed in `colormap/` directory

### Application Icon
Place `icon.png` in `assets/` directory:
- Recommended size: 256×256 pixels
- Format: RGBA PNG
- Used for window icon and taskbar

### Configuration
`lenia_config.txt` - Simple key=value format:
```
showConsole=1
```

## 13. Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| GLFW | 3.3+ | Window/context management |
| GLAD | 4.6 Core | OpenGL extension loading |
| Dear ImGui | Latest docking | UI framework |
| GLM | 0.9.9+ | Math library |
| stb_image | Latest | Image loading (colormaps, icon) |
## 14. Cross-Platform Compilation

### Windows (Recommended: MSYS2 UCRT64)

**Prerequisites:**
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-glfw
```

**Build:**
```batch
.\build_windows.bat
```

**Options:**
- `--opt=O3` - Optimization level (O0/O1/O2/O3)
- `--clean` - Clean build
- `--run` - Run after build
- `--no-pause` - Don't pause on completion

### Linux (Debian/Ubuntu)

**Prerequisites:**
```bash
sudo apt-get install build-essential libglfw3-dev libgl1-mesa-dev
```

**Build:**
```bash
make
```

**Run:**
```bash
./bin/Lenia
```

### Linux (Arch Linux)

**Prerequisites:**
```bash
sudo pacman -S base-devel glfw-x11 mesa
```

**Build:**
```bash
make
```

### Linux (Fedora/RHEL)

**Prerequisites:**
```bash
sudo dnf install gcc-c++ glfw-devel mesa-libGL-devel
```

**Build:**
```bash
make
```

### macOS

**Prerequisites:**
```bash
brew install glfw
```

**Makefile modifications needed for macOS:**
```makefile
# Replace Linux LIBS line with:
LIBS := -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
```

**Build:**
```bash
make
```

**Note:** macOS only supports OpenGL up to 4.1 (deprecated). Full functionality requires OpenGL 4.5+ compute shaders. Consider using MoltenVK (Vulkan to Metal translation) for full feature support, or a native Metal port for future development.

### Alternative Windows Compilers

**Visual Studio (MSVC):**
- Open project in Visual Studio
- Configure include paths for libs/
- Link against: glfw3.lib, opengl32.lib
- Use C++20 standard (`/std:c++20`)

**MinGW-w64 (non-MSYS2):**
- Download MinGW-w64 from winlibs.com or sourceforge
- Install GLFW binaries
- Modify build_windows.bat paths accordingly

### Portable Builds

For distributing Windows binaries:
1. Build with `-static-libgcc -static-libstdc++`
2. Include required DLLs: `glfw3.dll`
3. Package entire `bin/` folder including `assets/` and `Initialisation/`

### GPU Requirements

- **Minimum:** OpenGL 4.5 capable GPU (most GPUs from 2012+)
- **Recommended:** OpenGL 4.6 with compute shader support
- **Memory:** 512MB+ VRAM for standard grids, 2GB+ for large grids (2048x2048+)

### Platform-Specific Notes

**Windows:**
- Icons and version info embedded via resource file (Lenia.rc)
- Uses WinMain for release builds (no console)
- Console shown when lenia_config.txt contains `showConsole=1`

**Linux:**
- X11 required (Wayland support via XWayland)
- May need to set `export MESA_GL_VERSION_OVERRIDE=4.6` for some drivers

**macOS Limitations:**
- OpenGL deprecated by Apple (last supported: 4.1)
- Compute shaders unavailable in native OpenGL
- Would require Vulkan/Metal port for full feature parity