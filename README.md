# VirtualLogiGate

VirtualLogiGate is a digital logic simulator developed by **Semaja Voyé**. Its primary purpose is the **visualization and accurate simulation of digital logic gates**.

It is engineered entirely in **C**, leveraging the language for **maximum performance, memory efficiency, and deterministic, low-level execution control**—a perfect fit for a systems-level application.

VirtualLogiGate is currently **under active development**. Contributions that enhance core performance, stability, architectural elegance, or usability are highly valued. Let’s build something we can be proud of—**a clean, highly performant, and educational logic simulator developed from the ground up.**

---
# Installation and Build Process

## 1. Prerequisites

To build and run this project, you need the following installed:

1. **C Compiler**: A standard C compiler (like GCC, Clang, or MSVC).
2. **CMake**: The cross-platform build system.
3. **SDL3 Development Libraries**: The project relies on SDL3 for rendering and input handling.

### Windows Setup (Recommended: VS Code + CMake Tools)

The easiest way to get the compiler, CMake, and SDL3 dependencies is by using a system like MSYS2 or by integrating Vcpkg (Microsoft's C/C++ package manager) with Visual Studio Code.

### Using Vcpkg (Easiest for most users):

1. Install Vcpkg (if not already installed).
2. Install SDL3 (Execute this is a normal terminal not in this project folder as we do not need it here. It's just for CMake to get the dependencies but they don't have to be in the project folder!):
   ```
   vcpkg install sdl3:x64-windows
   ```
3. Configure VS Code: The CMake Tools extension will automatically detect and use Vcpkg's toolchain file to find SDL3.

## 2. Building the Project

We use CMake to handle the build process, making it compatible across different operating systems without hardcoding paths.

### Option A: Using the VS Code CMake Extension (Recommended)

1. Install the official CMake Tools extension for VS Code.
2. Open the project folder in VS Code.
3. VS Code will prompt you to select a Kit (compiler). Choose your installed compiler (e.g., GCC 13.x.x x86_64-w64-mingw32).
4. The extension will automatically Configure and Build the project. The executable will be placed in the `build/` directory.

### Option B: Manual Command Line Build

If you prefer the command line:

1. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

2. Configure the project (if using Vcpkg, ensure you pass the toolchain file):
   ```bash
   # Standard configuration (requires SDL3 to be found in system path)
   cmake ..
   
   # If using Vcpkg
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<path_to_vcpkg>/scripts/buildsystems/vcpkg.cmake
   ```

3. Build the project:
   ```bash
   cmake --build .
   ```

The executable (`VirtualLogiGate.exe` on Windows, or just `VirtualLogiGate` on Linux/macOS) will be located in the build directory.

## 3. Project Structure

| File/Folder       | Description                                                                 |
|-------------------|-----------------------------------------------------------------------------|
| `src/`            | Contains all source (.c) files.                                             |
| `build/`          | Output directory for the compiled executable (created by CMake).            |
| `CMakeLists.txt`  | Defines the project structure and dependencies for the build system.        |
| `.vscode/`        | Contains configuration files for Visual Studio Code.                        |
---

## Features

### ✅ **Implemented (Core Logic):**

* **Fundamental Logic Gates:** AND, OR, INVERT (NOT), NAND, NOR, XOR, XNOR.
* **Debug Visualization:** Console-based output for initial structure verification and logic debugging.
* **Modular Architecture:** Clean separation of concerns using C header files for component definitions and function prototypes.

### 🚧 **In Active Development (Visualization & Simulation Flow):**

* **Interactive GUI:** Planned implementation using the SDL3 library.
* **Signal Propagation Model:** Robust simulation of wire connection logic and signal flow across gates.
* **Persistence:** Implementation of save/load functionality for circuit files (planned `.vlg` format).
* **Logical Simplification Engine:** Algorithms for optimizing and reducing complex gate structures.

### 🧠 **Future Scope (Advanced Systems):**

* **Parallel Processing:** Multi-threaded simulation engine for scaling performance with large circuits.
* **Timing Analysis:** Integration of signal delay and critical path analysis capabilities.
* **Graphical Editor:** Drag-and-drop circuit construction interface.
* **Educational Mode:** Features for step-by-step logic tracing and gate function explanation.

---

## Contributing

Contributions from experienced developers and skilled apprentices are **highly valued and encouraged**.
To contribute:

1.  **Fork** the repository.
2.  **Create a new, dedicated branch** for your feature or fix.
3.  **Commit clean, atomic, and descriptive changes** (Conventional Commits encouraged).
4.  **Submit a well-documented Pull Request.**

Please adhere to **C99 standards** and maintain **modular, well-documented, and consistent** code quality throughout the project.

---

## License

VirtualLogiGate is released under the **MIT License**.
You are free to use, modify, and distribute the software, provided proper attribution to the original author is maintained.

---

## Author

**Semaja Voyé**
Apprentice IT Specialist & Software Developer
Driven by curiosity, foundational logic, and the pursuit of technical excellence.