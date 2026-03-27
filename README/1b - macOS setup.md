### macOS Setup Guide

#### Prerequisites

- macOS 13+ (Apple Silicon or Intel)
- Xcode command line tools:
  ```
      xcode-select --install
  ```
- Homebrew:
  ```
      /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  ```
- Node.js 20+ (via Homebrew):
  ```
      brew install node
  ```
  Verify with:
  ```
      node -v
      npm -v
  ```
- Ninja build system:
  ```
      brew install ninja
  ```
  Verify with:
  ```
      ninja --version
  ```
- pkg-config:
  ```
      brew install pkg-config
  ```
- Drogon
  ```
      brew install drogon
  ```
- CLion 2025.2.5
- Git
- vcpkg
  ```
      cd "$HOME"
      git clone https://github.com/microsoft/vcpkg.git
      cd vcpkg
      ./bootstrap-vcpkg.sh
  ```
  Add this to your shell config (`~/.zshrc` or `~/.bashrc`):
  ```
      export VCPKG_ROOT="$HOME/vcpkg"
  ```
  Then reload your shell:
  ```
      source ~/.zshrc    # or ~/.bashrc
  ```
  Verify that:
  ```
      echo "$VCPKG_ROOT"
  ```
  returns
  ```
      /Users/<you>/vcpkg
  ```

#### Project setup (for each new game)

1) Use terraform (or ask Terry/Gabriel) to create a new repo from the cpp-template-game template.
2) Clone your fork.
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
    - Another projects folder is fine, but this is recommended to avoid problems with long paths.
3) Set up the frontend
    - In the repo root:
      ```
      cd srcFrontend
      npm install
      npm run build
      ```
4) Open in CLion
    - Start CLion.
    - Go to File > Open and select `C:\Projects\cpp-{your-game-name}\CMakeLists.txt`.
    - When prompted, click "Open as Project".
5) Replace `VCPKG_ROOT`
    - In `CMakePresets.json`, replace each occurrence of `$env{VCPKG_ROOT}` with the path returned by:
      ```
      echo "$VCPKG_ROOT"
      ```
6) Select toolchain
    - File → Settings → Build, Execution, Deployment → Toolchains
    - Use the default "macOS" toolchain (Apple Clang).
    - Ensure CMake and Ninja are detected (CLion usually finds `/opt/homebrew/bin/ninja`).
7) Enable CMake presets
    - File → Settings → Build, Execution, Deployment → CMake
    - Enable all the `macos-*` presets by ticking the check boxes.
8) Test the sim
    - In the configuration selector, choose: `macos-sim` then Build/Run.
9) Test the frontend:
    - Select `macos-frontend` (or equivalent) and Run.
    - Open your browser at the URL printed by the server (typically `http://localhost:5173`).
10) Add template as remote to allow getting updates in future:
    - `git remote add upstream <copied URI from step 2>`

