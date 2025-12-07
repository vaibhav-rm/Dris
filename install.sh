#!/bin/bash

# Compile Dris
echo "Compiling Dris..."
gcc dris.c -o dris -lSDL2 -lm

if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# Create directories if they don't exist
mkdir -p ~/.local/bin
mkdir -p ~/.local/share/applications

# Copy binary
echo "Installing binary to ~/.local/bin/..."
cp dris ~/.local/bin/

# Copy desktop file
echo "Installing desktop entry..."
# We need to make sure the Exec path in the desktop file matches the user's home
# sed can replace /home/vaibhav with $HOME if needed, but for now we hardcoded it or use generic expansion if supported.
# Better: generic desktop file uses full path or just command if in path.
# Let's rewrite the desktop file with the correct HOME path dynamically.

cat > ~/.local/share/applications/dris.desktop <<EOF
[Desktop Entry]
Type=Application
Name=Dris
Comment=Lightweight Image Viewer
Exec=$HOME/.local/bin/dris %f
Icon=utilities-terminal
Terminal=false
Categories=Graphics;Viewer;
MimeType=image/png;image/jpeg;image/bmp;image/gif;image/webp;
StartupNotify=true
EOF

# Update desktop database
echo "Updating desktop database..."
update-desktop-database ~/.local/share/applications

echo "Installation complete!"
echo "You can now set Dris as your default image viewer."
