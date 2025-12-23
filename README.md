# Installation Guide

## Quick Install

```bash
# Clone the repository
git clone https://github.com/heykulthe/competitive-fetch.git
cd competitive-fetch

# Build and install (requires sudo for system-wide install)
make install
```

That's it! Now you can use `cpf` from anywhere:

```bash
cpf -t lru dsu -f main.cpp
```

## Manual Installation

If you prefer manual steps:

```bash
# Configure and build
cmake -S . -B build
cmake --build build

# Install to /usr/local/bin (default)
sudo cmake --install build

# Or install to custom location (no sudo needed)
cmake --install build --prefix ~/.local
```

If you use a custom prefix, make sure `~/.local/bin` is in your PATH:

```bash
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

## Uninstall

```bash
make uninstall
# or
sudo cmake --build build --target uninstall
```

## Install Locations

- **Binary**: `/usr/local/bin/cpf` (or `~/.local/bin/cpf` with custom prefix)
- **Cache**: `~/.cache/cpf/` (created automatically on first run)

## System Requirements

- CMake 3.20+
- C++17 compatible compiler (GCC 7+, Clang 5+)
- curl (for fetching templates)
- pthread support

## Verifying Installation

```bash
# Check if cpf is installed
which cpf

# Test it
cpf -t lru -f test.cpp
```

## Building from Source (Development)

```bash
# Build without installing
make build

# Run from build directory
./build/cpf -t lru -f sample.cpp
```