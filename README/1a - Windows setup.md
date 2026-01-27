### Windows Setup Guide

#### Prerequisites

- Windows 10/11 (64-bit)
- Visual Studio 2024 Build Tools (or full Visual Studio)
    - Go to https://visualstudio.microsoft.com/downloads/
    - Scroll to bottom and expand "Tools for Visual Studio".
    - Next to "Build Tools for Visual Studio 2026" look for
      "Are you looking for one of the Visual Studio 2022 long term servicing baselines (LTSCs)? You can find
      them here." and click "here".
    - Click "Build Tools" next to version 17.14.
    - Run the installer.
    - In install options, tick "Desktop development with C++".
- CLion 2025.2.5+
- Git
- Node.js 20+ (LTS) – installer from https://nodejs.org
    - Download the Windows .msi from https://nodejs.org
    - Run the installer (accept defaults)
    - In a new PowerShell/Command Prompt:
      ```
      node -v
      npm -v
      ```
    - Make sure both commands work.
- vcpkg
    - Open PowerShell:
      ```powershell
      cd $env:USERPROFILE
      git clone https://github.com/microsoft/vcpkg.git
      cd vcpkg
      .\bootstrap-vcpkg.bat
      ```
    - Set `VCPKG_ROOT` permanently:
      ```powershell
      setx VCPKG_ROOT "$env:USERPROFILE\vcpkg"
      ```
    - Then restart your terminal and verify:
      ```powershell
      echo $env:VCPKG_ROOT
      ```
    - You should see something like:
      ```
      C:\Users\<You>\vcpkg
      ```

#### Project setup (for each new game)

1) Use terraform (or ask Terry/Gabriel) to create a new repo from the cpp-template-game template.

2) Clone your fork

    - Clone by HTTP or SSH. On the repo page for your forked repo, click the green "Code" button. Then copy
      the
      URI under "Clone with HTTPS" or "Clone with SSH".
    - In Terminal:
      ```
      cd "$HOME/Projects"    # or wherever you keep code
      git clone <copied-URI>
      cd cpp-{your-game-name}
      ```
    - You can use Sourcetree if you prefer.
    - Another projects folder is fine, but this one is recommended to avoid problems with long paths.

3) Set up the frontend
    - In the repo root:
      ```
      cd srcFrontend
      npm install
      npm run build
      ```

4) Open in CLion

    - Start CLion.
    - Open the folder `C:\Projects\cpp-{your-game-name}`.

5) Select Toolchain:
    - Go to: File → Settings → Build, Execution, Deployment → Toolchains
    - Add a "Visual Studio" toolchain if not already present.
    - Make the Visual Studio toolchain the default.
    - CLion should detect the VS compiler and bundled CMake/Ninja.

6) Enable CMake presets:
    - Go to: File → Settings → Build, Execution, Deployment → CMake
    - Ensure all four `windows-*` presets are enabled.
7) Test the sim:
    - In the top-right configuration selector, choose `windows-sim`.
    - Click the Run/Build button to ensure it compiles.
8) Test the frontend:
    - Select the `windows-frontend` config.
    - Build and run.
    - Open the browser at the 'Local' URL printed in the run output by the backend and try a few spins.