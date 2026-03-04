# Building SimulIDE on Windows

This guide will help you build SimulIDE using **Qt Creator** on Windows.

## Prerequisites

1.  **Qt Creator** installed. You can get it from [Qt's website](https://www.qt.io/download).
2.  A **Compiler Kit** (MinGW or MSVC) configured in Qt Creator.
    - We recommend **MinGW 11.2.0** or later (shipped with Qt 6.2+).
    - **MSVC 2019/2022** also works.

## Instructions

### 1. Run the Setup Script
Before opening the project, run the setup script to download the required `libelf` dependency.
- Open **PowerShell** in the project root.
- Run:
  ```powershell
  .\scripts\setup-windows.ps1
  ```
- This will download and prepare `libelf` in the `3rdparty/libelf` folder.

### 2. Open in Qt Creator
- Open **Qt Creator**.
- Click **File** -> **Open File or Project...**.
- Select the `SimulIDE.pro` file in the project root.
- Select your **Kit** (e.g., Desktop Qt 6.x.x MinGW 64-bit).

### 3. Build and Run
- Wait for the project to load.
- Click the **Run** button (Green arrow in the bottom-left corner).
- Qt Creator will compile, link, and run the simulator.

## Troubleshooting

- **Missing libelf.h**: Make sure you ran the `setup-windows.ps1` script first.
- **Linker Errors**: If you switch kits, you may need to **Clean** and **Re-run qmake** (Right-click project -> Clean, then Right-click -> Run qmake).
- **Paths with Spaces**: If your project is in a folder with spaces, our `.pro` file is already optimized for this, but Windows may still be picky. Try moving to a simpler path like `C:\Projects\Simul-DE`.

Enjoy simulating!
