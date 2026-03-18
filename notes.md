# Torque3D - Virtual Office Platform

## What is this?
Full clone of the Torque3D open-source game engine (MIT license), version 4.0.3,
with custom modules for AI-driven virtual environments.

- **Upstream**: https://github.com/TorqueGameEngines/Torque3D
- **Our repo**: https://github.com/drelf/torque3dMIIT3

## Product Vision
Three AI-driven 3D virtual environment products:
1. **Peak AI Client Room** — Customers meet in virtual room, screen sharing on 3D displays
2. **Virtual Office** — Remote teams in shared 3D office, each with desk + browser screen
3. **Virtual Campus** — Students attend classes, AI tutors, instructor screen broadcast

## Custom Modules (Already Scaffolded)

### browserRender — CEF Browser on 3D Surfaces
- `Engine/source/browserRender/` — All source files
- Renders web pages as textures on 3D objects via CEF (Chromium Embedded Framework)
- Key classes: `BrowserTexture` (SimObject), `BrowserRenderHandler` (CEF→GPU), `BrowserManager` (lifecycle)
- Input injection: mouse, keyboard, scroll all forwarded to CEF
- **Needs CEF binaries to activate** — build with `-DTORQUE_CEF_ENABLED=ON`

### aiBridge — LLM ↔ Torque3D Bridge
- `Engine/source/aiBridge/` — All source files
- TCP JSON protocol for external AI/LLM to control AIPlayer characters + browser screens
- Key class: `AIBridgeManager` (singleton, TCP listener, agent registry)
- Default port: 9090
- Commands: move, aim, fire, browse, js, spawn, query, exec

### Example Files
- `Engine/source/browserRender/examples/virtualOffice.tscript` — Full office setup script
- `Engine/source/aiBridge/examples/llm_controller.py` — Python LLM controller using OpenAI

## Dev2 Setup (M2 Pro Mac)

### 1. Clone
```bash
git clone https://github.com/drelf/torque3dMIIT3.git ~/projects/torque3d
cd ~/projects/torque3d
```

### 2. Install Build Tools
```bash
# Xcode command line tools (if not installed)
xcode-select --install

# Homebrew packages
brew install cmake nasm
```

### 3. Build (without CEF — engine only)
```bash
cd ~/projects/torque3d
mkdir build && cd build
cmake .. -G Xcode
cmake --build . --config Release
```

### 4. Build (with CEF — full browser support)
```bash
# Download CEF binary distribution for macOS arm64
# https://cef-builds.spotifycdn.com/index.html — pick "macOS ARM64", Minimal Distribution
# Extract to Engine/lib/cef/

cd ~/projects/torque3d
mkdir build && cd build
cmake .. -G Xcode -DTORQUE_CEF_ENABLED=ON -DCEF_ROOT=../Engine/lib/cef
cmake --build . --config Release
```

### 5. Run
```bash
# The binary lands in:
# My Projects/VirtualOffice/game/VirtualOffice
```

### 6. Test AI Bridge
```bash
# Start the engine, then from another terminal:
cd Engine/source/aiBridge/examples
pip install openai
python llm_controller.py --port 9090 --api-key YOUR_OPENAI_KEY
```

## TCP JSON Protocol (aiBridge)

Send newline-delimited JSON to port 9090:

```json
{"action": "move", "id": 1, "x": 100, "y": 200, "z": 30}
{"action": "aim", "id": 1, "x": 0, "y": 0, "z": 50}
{"action": "browse", "id": 1, "url": "https://example.com"}
{"action": "js", "id": 1, "code": "document.title"}
{"action": "spawn", "datablock": "OfficeWorkerData", "x": 10, "y": 5, "z": 0}
{"action": "query", "id": 1}
{"action": "exec", "script": "echo(\"hello\");"}
```

## Key Info
- **Language**: C++ with TorqueScript
- **License**: MIT
- **Branch**: `development`
- **Platforms**: Windows, macOS, Linux
- **Rendering**: PBR, Metal (macOS), OpenGL (Linux), DirectX (Windows)
- **Networking**: Built-in multiplayer (world-class)
- **Physics**: PhysX / Bullet

## Build Dependencies

### macOS (M2 Pro)
Xcode, cmake, nasm (via Homebrew). vcpkg auto-bootstraps during cmake.

### Ubuntu (Dev1 VM)
```bash
sudo apt-get install -y cmake build-essential libsdl2-dev libgl1-mesa-dev \
  libopenal-dev libfreetype-dev libx11-dev libxext-dev libxi-dev \
  libxrandr-dev libxxf86vm-dev nasm libglu1-mesa-dev libgtk-3-dev
```

## Build Docs
- https://docs.torque3d.org
- https://reference.torque3d.org
