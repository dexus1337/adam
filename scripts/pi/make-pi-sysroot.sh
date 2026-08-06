#!/bin/bash
set -e # Exit immediately if a command exits with a non-zero status

SYSROOT="$HOME/pi-sysroot"

# Clean and recreate local sysroot structure
rm -rf "$SYSROOT"
mkdir -p "$SYSROOT/usr/include"
mkdir -p "$SYSROOT/usr/lib/aarch64-linux-gnu"
mkdir -p "$SYSROOT/usr/lib/gcc/aarch64-linux-gnu"
mkdir -p "$SYSROOT/lib/aarch64-linux-gnu"

echo "=== Syncing headers and libraries ==="
# Dropped sudo since these directories are world-readable
rsync -avzL pi@pi:/usr/include/ "$SYSROOT/usr/include/"
rsync -avzL pi@pi:/usr/lib/aarch64-linux-gnu/ "$SYSROOT/usr/lib/aarch64-linux-gnu/"
rsync -avzL pi@pi:/usr/lib/gcc/aarch64-linux-gnu/ "$SYSROOT/usr/lib/gcc/aarch64-linux-gnu/"
rsync -avzL pi@pi:/lib/aarch64-linux-gnu/ "$SYSROOT/lib/aarch64-linux-gnu/"

echo "=== Creating dynamic linker symlink ==="
ln -sf "$SYSROOT/lib/aarch64-linux-gnu/ld-linux-aarch64.so.1" "$SYSROOT/lib/ld-linux-aarch64.so.1"

echo "=== Patching absolute paths in linker scripts ==="
# -r stops xargs from running if grep finds nothing
# We look for text files ending in .so to fix absolute paths
find "$SYSROOT/usr/lib" "$SYSROOT/lib" -name "*.so" -type f -exec grep -lE '^GROUP|^INPUT' {} \; | xargs -r sed -i 's|/usr/lib/aarch64-linux-gnu/||g'
find "$SYSROOT/usr/lib" "$SYSROOT/lib" -name "*.so" -type f -exec grep -lE '^GROUP|^INPUT' {} \; | xargs -r sed -i 's|/lib/aarch64-linux-gnu/||g'

echo "=== Converting absolute symlinks to relative ==="
# Ensure symlinks utility is installed locally (sudo apt install symlinks)
symlinks -cr "$SYSROOT"

echo "=== Sysroot configuration complete! ==="
