Installation requirements
=========================

----------------------------------------------------------------------
Windows setup
----------------------------------------------------------------------

# Prerequisites

- Windows 10/11 (64-bit)
- Visual Studio 2024 Build Tools (or full Visual Studio)
    - Go to https://visualstudio.microsoft.com/downloads/.
    - Scroll to bottom and expand "Tools for Visual Studio".
    - Next to "Build Tools for Visual Studio 2026" look for
        "Are you looking for one of the Visual Studio 2022 long term servicing baselines (LTSCs)?  You can find them here."
        and click "here".
    - Click "Build Tools" next to version 17.14.
    - Run the installer.
    - In install options, tick "Desktop development with C++".
- CLion 2025.2.5+
- Git
- Node.js 20+ (LTS) – installer from https://nodejs.org
    - Download the Windows .msi from https://nodejs.org
    - Run the installer (accept defaults)
    - In a new PowerShell/Command Prompt:
        node -v
        npm -v
    - Make sure both commands work.
- vcpkg
    - Open PowerShell:
        cd $env:USERPROFILE
        git clone https://github.com/microsoft/vcpkg.git
        cd vcpkg
        .\bootstrap-vcpkg.bat
    - Set `VCPKG_ROOT` permanently:
        setx VCPKG_ROOT "$env:USERPROFILE\vcpkg"
    - Then restart your terminal (or log out/in) and verify:
        echo $env:VCPKG_ROOT
    - You should see something like:
        C:\Users\<You>\vcpkg


# Project setup (for each new game)

1. Fork the template repo

   - Fork `cpp-frontend-dev` to `cpp-{your-game-name}` on GitLab.

2. Clone your fork

    Clone by HTTP or SSH. On the repo page for your forked repo, click the green "Code" button.
    Then copy the URI under "Clone with HTTPS" or "Clone with SSH".

   In Terminal:

       cd "$HOME/Projects"    # or wherever you keep code
       git clone *copied URI*
       cd cpp-{your-game-name}

   You can use sourcetree if you prefer.
   Another projects folder is fine, but this is recommended to avoid problems with long paths.

3. Set up the frontend

   In the repo root:

       cd srcFrontend
       npm install
       npm run build

4. Open in CLion

   - Start CLion.
   - Open the folder `C:\Projects\cpp-{your-game-name}`.

   Toolchains:

   - Go to: File → Settings → Build, Execution, Deployment → Toolchains
   - Add a "Visual Studio" toolchain if not already present.
   - Make the Visual Studio toolchain the default.
   - CLion should detect the VS compiler and bundled CMake/Ninja.

   CMake presets:

   - Go to: File → Settings → Build, Execution, Deployment → CMake
   - Ensure all four windows-* presets enabled.

5. Build & run

   To test the sim:
   - In the top-right configuration selector, choose `windows-sim`.
   - Click the Run/Build button to ensure it compiles.

   To test the frontend:

   - Select the `windows-frontend` config.
   - Build and run.
   - Open the browser at the 'Local' URL printed in the run output by the backend and try a few spins.


----------------------------------------------------------------------
macOS setup
----------------------------------------------------------------------

# Prerequisites

- macOS 13+ (Apple Silicon or Intel)
- Xcode command line tools:
    - In Terminal:
        xcode-select --install
- Homebrew:
    - In Terminal:
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
- Node.js 20+ (via Homebrew):
    - In Terminal:
        brew install node
    - Verify with:
        node -v
        npm -v
- Ninja build system:
    - In Terminal:
        brew install ninja
    - Verify with:
        ninja --version
- pkg-config:
    - In Terminal:
        brew install pkg-config
- CLion 2025.2.5
- Git
- vcpkg
    - In Terminal:
        cd "$HOME"
        git clone https://github.com/microsoft/vcpkg.git
        cd vcpkg
        ./bootstrap-vcpkg.sh
    - Add this to your shell config (`~/.zshrc` or `~/.bashrc`):
        export VCPKG_ROOT="$HOME/vcpkg"
    - Then reload your shell:
       source ~/.zshrc    # or ~/.bashrc
    - Verify that
        echo "$VCPKG_ROOT"
    returns
        /Users/<you>/vcpkg


# Project setup (for each new game)

1. Fork the template repo

   - Fork `cpp-frontend-dev` to `cpp-{your-game-name}` on GitLab.

2. Clone your fork

    Clone by HTTP or SSH. On the repo page for your forked repo, click the green "Code" button.
    Then copy the URI under "Clone with HTTPS" or "Clone with SSH".

   In Terminal:

       cd "$HOME/Projects"    # or wherever you keep code
       git clone *copied URI*
       cd cpp-{your-game-name}

   You can use sourcetree if you prefer.
   Another projects folder is fine, but this is recommended to avoid problems with long paths.

3. Set up the frontend

       cd srcFrontend
       npm install
       npm run build

4. Open in CLion

   - Start CLion.
   - Open the folder `~/Projects/cpp-{your-game-name}`.

   a) Replace VCPKG_ROOT

        - In CMakePresets.json, replace each occurrence of $env{VCPKG_ROOT} with the path returned by:
           echo "$VCPKG_ROOT"

   b) Select toolchains:

        - File → Settings → Build, Execution, Deployment → Toolchains
        - Use the default "macOS" toolchain (Apple Clang).
        - Ensure CMake and Ninja are detected (CLion usually finds `/opt/homebrew/bin/ninja`).

   c) Enable CMake presets:

        - File → Settings → Build, Execution, Deployment → CMake
        - Enable all the macos-* presets by ticking the check boxes.


5. Build & run

   - To test the sim, in the configuration selector, choose:
       - `macos-sim` (backend simulation build), then Build/Run.
   - To test the frontend:
       - Select `macos-frontend` (or equivalent) and Run.
       - Open your browser at the URL printed by the server
         (typically `http://localhost:5173`).


----------------------------------------------------------------------
Dockerising
----------------------------------------------------------------------

When you're ready for other team members to play your game, follow these instructions to prepare it for hosting.

# Prerequisites
    - Docker Desktop (Windows/macOS)
        - Go to https://docs.docker.com/desktop/
        - Follow "Install Docker Desktop" links at the bottom of the page.

# To build the docker image:

1. Make sure Docker Desktop is running.

2. From the repo root:

       docker build -t cpp-game-name .
       # (rename to your game name)

# To test:

1. Run:
       docker run --rm -p 8080:8080 cpp-game-image

2. Open your browser at:
       http://localhost:8080