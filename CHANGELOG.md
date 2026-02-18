# Changelog

All notable changes to this project will be documented in this file.

## [1.1.0] - 2026-02-18

### Added

#### Accessibility

- New Accessibility section in the UI panel
- Adjustable UI scale (0.75x to 3.0x) for high-DPI displays and user preference
- Configurable font size (10pt to 32pt) for improved readability
- High contrast mode for users with visual impairments
- Reduce motion option to minimize animations
- Keyboard navigation support with Tab/Shift+Tab traversal
- Visible focus indicators for keyboard navigation
- System DPI awareness with automatic scaling detection
- Persistent accessibility settings saved to configuration file

#### Internationalization

- Complete localization system with TR() macro for all UI strings
- Full French translation (Francais) with 450+ translated strings
- Language selection in Accessibility settings
- Language preference persistence across sessions
- Runtime language switching without restart

### Changed

- Refactored UI text rendering to use centralized localization system
- Improved text rendering for variable-width fonts
- Enhanced UI element spacing and alignment for different font sizes

### Technical

- Added Localization class (singleton) for translation management
- Added AccessibilitySettings struct with configurable parameters
- New TextId enum with compile-time safe translation identifiers
- Language preference stored in lenia_lang.cfg
- Accessibility settings stored in lenia_accessibility.cfg

---

## [1.0.0] - 2026-02-16

### Initial Release

High-performance GPU-accelerated Lenia continuous cellular automaton simulator built with C++20 and OpenGL compute shaders.

#### Core Features
- Real-time GPU simulation using OpenGL 4.5/4.6 compute shaders
- Zero-copy architecture with full state in VRAM
- Support for Classic Lenia, Multi-channel Lenia, and extended variants
- 548 pre-loaded species across multiple categories
- 11 growth functions and 10 kernel types
- Interactive mouse panning (middle-click or Ctrl+right-click drag)
- Zoom controls with mouse wheel
- Real-time parameter adjustment

#### Multichannel System
- 1-3 independent channels with RGB visualization
- Up to 16 simultaneous interaction rules
- Cross-channel growth dynamics
- Per-rule kernel configurations with multi-ring support

#### Visualization
- 12+ built-in colormaps (Viridis, Magma, Inferno, Plasma, Jet, Lenia, Rainbow, Grayscale)
- Custom colormap support from external files
- Display modes: World view, Kernel overlay, Growth field, Neighbor sums, Delta view, Vector field, Contour lines, Heat map, Activity map
- Real-time colormap controls (hue shift, saturation, range, power curve, offset, reverse)
- Multichannel blend modes with 7 options
- Boundary visualization with 5 styles (Solid, Dashed, Dotted, Double, Glow)
- Boundary animation and customizable thickness

#### Analysis & Monitoring
- Live pattern analysis (mass, alive count, centroid, speed, direction)
- Periodic behavior detection and stability monitoring
- Auto-pause on pattern extinction or stability
- Professional graphs with labeled axes and grid lines
- Performance metrics (FPS, frame time, throughput in Gcells/s)

#### User Interface
- Comprehensive ImGui-based interface with detachable windows
- Searchable preset browser with category filtering
- Species preview with current colormap
- Grid transformations (flip, rotate)
- Multiple placement modes (center, corners, scatter, grid, random)
- Brush tool with configurable size, strength, and stroke settings
- Wall editor with various wall types
- Infinite World Mode with chunk-based navigation
- Pause overlay with smooth animations

#### Technical Implementation
- C++20 with modern features (concepts, smart pointers)
- Data-oriented design with GPU-aligned structures (std140/std430)
- Ping-pong texture buffering for efficient state management
- Immutable texture storage for optimal GPU performance
- Windows resource file with embedded icon and version info
- Custom build system with MSYS2 MinGW-w64 support

#### Supported Platforms
- Windows 10/11 (primary target)
- Linux (via Makefile)
- Cross-platform compilation instructions provided

#### Build System
- Automated Windows build script (build_windows.bat)
- Makefile for Linux/Unix systems
- Automatic library detection and configuration
- Static linking for portable binaries
- Build options: optimization levels, clean builds, run-after-build

#### Documentation
- Comprehensive architecture documentation
- Detailed README with features and usage instructions
- Cross-platform compilation guide
- Code structure and design patterns documented
- MIT License

[1.1.0]: https://github.com/Wartets/Lenia/releases/tag/v1.1.0
[1.0.0]: https://github.com/Wartets/Lenia/releases/tag/v1.0.0
