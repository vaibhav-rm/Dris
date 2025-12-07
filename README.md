# 👁️ Dris
> **The no-nonsense, lightweight image viewer for Linux.**

Dris (from Sanskrit *drishti*, meaning sight/vision) is built for speed and simplicity. It doesn't get in your way; it just shows your images.

## ✨ Features
- **🚀 Blazing Fast**: Powered by `stb_image` for instant loading of PNG, JPG, BMP, and more.
- **🔍 Deep Zoom**: Smooth-scrolling zoom centered on your cursor.
- **✋ Intuitive Pan**: Grab and drag to move around large images.
- **🎛️ Minimal UI**: A sleek, unobtrusive toolbar for quick controls.
- **📦 Zero Bloat**: No heavy frameworks. Just C and SDL2.

## 🛠️ Build
Requirement: `libsdl2-dev`

```bash
gcc dris.c -o dris -lSDL2 -lm
```

## 🎮 Usage
Open an image directly from the terminal:

```bash
./dris path/to/image.png
```

| Control | Action |
| :--- | :--- |
| **Scroll** | Zoom In / Out |
| **Drag** | Pan Image |
| **ESC** | Quit / Close Overlay |


*Powered by SDL2 & [stb](https://github.com/nothings/stb).*
