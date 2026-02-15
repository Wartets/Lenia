# Lenia - Real-Time GPU-Accelerated Artificial Life Simulator

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg" alt="C++20"/>
  <img src="https://img.shields.io/badge/OpenGL-4.5%2F4.6-green.svg" alt="OpenGL"/>
  <img src="https://img.shields.io/badge/Platform-Windows-lightgrey.svg" alt="Platform"/>
</p>

A high-performance, real-time implementation of **Lenia**, a continuous cellular automaton system that produces life-like emergent behaviors. This project leverages GPU compute shaders for massively parallel simulation, enabling smooth interactive exploration of complex artificial life patterns.

## Features

### Core Simulation
- **Multiple Lenia Variants**: Classic Lenia, Multi-channel Lenia, Game of Life, SmoothLife, Larger-Than-Life
- **548 Pre-loaded Species**: Extensive library including Orbium, Scutium, Hydrogeminium, and multichannel creatures
- **11 Growth Functions**: Lenia Gaussian, Step, SmoothLife, Polynomial, Exponential, Double Peak, Asymptotic, SoftClip, Quad4, and more
- **10 Kernel Types**: Gaussian Shell, Bump4, Multi-ring variants, Mexican Hat, Cosine Shell, and custom ring configurations

### Multichannel System
- Support for 1-3 independent channels (RGB visualization)
- Up to 16 simultaneous interaction rules
- Cross-channel growth dynamics
- Channel-specific kernel configurations

### Visualization
- **12+ Colormaps**: Viridis, Magma, Inferno, Plasma, Jet, Lenia, Rainbow, Grayscale, and custom colormaps
- **Display Modes**: Normal, Kernel overlay, Growth field, Neighbor sums, Delta view
- **Real-time Colormap Controls**: Range, power curve, offset, reverse
- **Mouse Hover**: Real-time cell value display with coordinates

### Analysis & Monitoring
- **Live Analysis**: Mass, alive cell count, centroid tracking, speed, direction
- **Pattern Detection**: Periodic behavior detection, stability monitoring, auto-pause
- **Graphs with Axes**: Professional visualizations with labeled axes, ticks, and grid lines
- **Performance Metrics**: FPS, frame time statistics, throughput (Gcells/s), efficiency rating

### User Interface
- Searchable preset browser with category filtering
- Species preview with current colormap
- Grid transformations (flip horizontal/vertical, rotate CW/CCW)
- Placement options: Center, corners, scatter, grid, random
- Real-time parameter adjustment with slider markers for default values

## Requirements

### Build Requirements
- **Compiler**: MinGW-w64 (MSYS2 UCRT64) with GCC 13+ or GCC 15 recommended
- **Graphics**: OpenGL 4.5 compatible GPU (4.6 preferred)
- **OS**: Windows 10/11

### Runtime Requirements
- OpenGL 4.5+ capable graphics card
- GPU with compute shader support

## Building

### Quick Build
```batch
.\build_windows.bat
```

### Build Options
```batch
.\build_windows.bat --clean      # Clean rebuild
.\build_windows.bat --no-pause   # No pause at end (for scripts)
.\build_windows.bat --help       # Show options
```

The build script automatically:
- Detects MSYS2/MinGW-w64 installation
- Downloads and sets up required libraries (GLFW, GLAD, GLM, Dear ImGui)
- Compiles with optimizations (-O3, LTO)
- Links statically for easy distribution

### Manual Build
If you prefer using Make directly:
```batch
mingw32-make -j8
```

## Usage

### Controls
| Key | Action |
|-----|--------|
| Space | Pause/Resume simulation |
| S | Single step (hold for continuous stepping) |
| Shift+S | Fast continuous stepping |
| R | Reset with current preset |
| C | Clear grid |
| Arrow Keys | Pan view |
| Scroll | Zoom in/out |
| Middle Mouse | Pan view (drag) |
| F | Fit view to grid |

### Getting Started
1. Launch `bin/lenia.exe`
2. Browse presets in the **Presets** section
3. Select a category (e.g., "Multichannel") and choose a species
4. Press Space to start the simulation
5. Adjust parameters in real-time using the UI panels

## Architecture

### GPU-Accelerated Pipeline
```
Input → Kernel Generation → Convolution (Compute Shader) → Growth Function → State Update → Render
```

- **Zero-Copy Simulation**: Grid state lives entirely in VRAM
- **Double-Buffered State**: Ping-pong rendering for consistent updates
- **Compute Shaders**: Massively parallel simulation (millions of cells per frame)

### Key Components
| Component | Description |
|-----------|-------------|
| `LeniaEngine` | Core simulation controller |
| `SimulationState` | Double-buffered texture management |
| `KernelManager` | Kernel generation and caching |
| `Renderer` | Display and colormap application |
| `AnalysisManager` | Pattern tracking and statistics |
| `UIOverlay` | Dear ImGui interface |

### Performance
- Typical performance: 60+ FPS at 512×512 grid with R=13 kernel
- Throughput: 10+ Gcells/s on modern GPUs
- Minimal CPU overhead (GPU-bound simulation)

## Project Structure
```
Lenia/
├── src/                   # C++ source files
│   ├── Main.cpp           # Entry point
│   ├── Application.*      # Main loop and coordination
│   ├── LeniaEngine.*      # Simulation engine
│   ├── KernelManager.*    # Kernel texture generation
│   ├── Renderer.*         # Display rendering
│   ├── UIOverlay.*        # ImGui interface
│   ├── AnalysisManager.*  # Pattern analysis
│   └── Presets.*          # Species database
├── assets/
│   ├── shaders/           # GLSL compute and fragment shaders
│   └── init/              # Initialization patterns
├── colormap/              # Custom colormap files
└── libs/                  # Pre-built libraries
```

## Technical Details

### Simulation Parameters
| Parameter | Range | Description |
|-----------|-------|-------------|
| R (Radius) | 1-50 | Kernel radius in cells |
| μ (mu) | 0.0-1.0 | Growth function center |
| σ (sigma) | 0.001-0.3 | Growth function width |
| dt | 0.01-1.0 | Time step per update |

### Supported Grid Sizes
- Minimum: 64×64
- Default: 350×350
- Maximum: 2048×2048 (GPU memory dependent)

## Credits

### Lenia Research
- **Bert Wang-Chak Chan**: Creator of Lenia ([Original Paper](https://arxiv.org/abs/1812.05433))
- Lenia community for documenting species and parameter spaces

### Libraries
- [GLFW](https://www.glfw.org/) - Window and input handling
- [GLAD](https://glad.dav1d.de/) - OpenGL loader
- [GLM](https://github.com/g-truc/glm) - Mathematics library
- [Dear ImGui](https://github.com/ocornut/imgui) - Immediate mode GUI