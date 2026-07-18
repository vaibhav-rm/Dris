# 👁️ Dris
> **The no-nonsense, lightweight image viewer for Linux.**

![Dris Demo](demo.gif)

Dris (from Sanskrit *drishti*, meaning sight/vision) is built for speed and simplicity. It doesn't get in your way; it just shows your images.

## 📥 Downloads

Prebuilt binaries are available for Linux and Windows in the **[Releases](../../releases)** section.

- **Windows**: Download `dris-windows.zip`. Unzip and run `DrisInstaller.exe` to install, or use `dris.exe` for portable mode.
- **Linux**: Download `dris-linux.tar.gz`. Extract and run `./install.sh`.

## Building on Linux

To compile and install `dris`, simply run the install script:

```bash
./install.sh
```

This will:
1. Compile the `dris` binary.
2. Install it to `~/.local/bin/`.
3. Create a desktop entry, so you can set it as your default image viewer.

## Building on Windows

1.  **Install MinGW**: Ensure you have GCC installed via MinGW or similar.
2.  **Install SDL2**: distinct download the SDL2 development libraries for MinGW. Extract them and ensure the `include` and `lib` folders are accessible to your compiler, or copy the contents of `x86_64-w64-mingw32` (or `i686-w64-mingw32`) from the SDL2 development package into your MinGW installation directory.
3.  **Compile**:
    *   Double-click `build.bat`
    *   **OR** run command line: `gcc dris.c -o dris.exe -lmingw32 -lSDL2main -lSDL2 -lm`
4.  **Run**: Make sure `SDL2.dll` is in the same folder as `dris.exe`.


## ✨ Features
- **🚀 Blazing Fast**: Powered by `stb_image` for instant loading of PNG, JPG, BMP, WEBP, and more.
- **🔍 Deep Zoom**: Smooth-scrolling zoom centered on your cursor.
- **✋ Intuitive Pan**: Grab and drag or use Ctrl + Arrow keys to navigate around large images.
- **🔄 Instant Rotation**: Rotate images 90° clockwise or counter-clockwise with a single keystroke.
- **📁 Directory Navigation**: Scan directories automatically and switch to adjacent images with Left/Right Arrows or Space.
- **🎛️ Toggleable UI**: Sleek, unobtrusive toolbar that can be hidden for borderless distraction-free viewing.
- **📦 Zero Bloat**: No heavy web framework or electron wrapper. Just native C and SDL2.

## 🛠️ Build
Requirement: `libsdl2-dev` (Debian/Ubuntu) or `SDL2-devel` (Fedora/RHEL)

```bash
gcc dris.c -o dris -lSDL2 -lm
```

## 🎮 Usage
Open an image directly from the terminal:

```bash
./dris path/to/image.png
```

### Keyboard & Mouse Controls

| Control | Action |
| :--- | :--- |
| **Scroll / + / -** | Zoom In / Out (Centered on cursor) |
| **Drag Mouse** | Free Pan Image |
| **Ctrl + Arrows** | Pan Image (Up, Down, Left, Right) |
| **Right / Space / PgDn** | Next Image in directory |
| **Left / Backspace / PgUp** | Previous Image in directory |
| **R** | Rotate 90° Clockwise |
| **L** | Rotate 90° Counter-Clockwise |
| **F / F11** | Toggle Fullscreen (hides toolbar) |
| **T / H** | Toggle Toolbar visibility |
| **0 / Home** | Reset Zoom and Fit to Window |
| **ESC** | Close About Box / Exit Fullscreen / Quit |


*Powered by SDL2 & [stb](https://github.com/nothings/stb).*
