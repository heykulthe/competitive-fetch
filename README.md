# CPF - Competitive Programming Fetch

A blazingly fast CLI tool to fetch and integrate competitive programming templates into your code. Stop copy-pasting boilerplates and let `cpf` handle it for you.

## What is CPF?

`cpf` is a tool that fetches data structure and algorithm templates from a remote repository and automatically creates a `main.cpp` file along with an `input.txt` and `output.txt` in your current directory. 

## Quick Install

```bash
# Clone the repository
git clone https://github.com/heykulthe/competitive-fetch.git
cd competitive-fetch

# Build and install (requires sudo for system-wide install)
make install
```

That's it! Now you can use `cpf` from anywhere.

## Usage

### Basic Usage

```bash
cpf -f <output_file> -t <template1> <template2> ...
```

### Examples

```bash
# Fetch a single template
cpf -f solution.cpp -t dsu

# Fetch multiple templates
cpf -f solution.cpp -t dsu segment_tree fenwick

# Force refresh to get latest templates
cpf -f solution.cpp -t dsu --refresh
```

## Template Repository

Templates are fetched from: [github.com/heykulthe/cp-templates](https://github.com/heykulthe/cp-templates)

To use your own repositories, you need to add an environment variable: `CPF_REPO`. This helps `cpf` always use your own repository instead of the default one. Note that changes have been tested only for GitHub repositories, however support has been added for BitBucket, GitLab, and custom repos too. Custom repositories should not end with a trailing (`/`) slash.

### Command-Line Options

| Option | Short | Description |
|--------|-------|-------------|
| `--file` | `-f` | Output file where templates will be written (required) |
| `--templates` | `-t` | Space-separated list of template names to fetch (required) |
| `--refresh` | `-r` | Force refresh cache to fetch latest templates from remote |

### Output Structure

CPF generates a complete C++ file with:
- With the following headers: `#include <bits/stdc++.h>`
- Requested templates
- File I/O setup

### Cache Behavior

- **Location**: `~/.cache/cpf/`
- **Index Cache**: Expires after 30 minutes
- **Force Refresh**: Use `-r` or `--refresh` to bypass cache and fetch latest templates

## Manual Installation

If you prefer manual steps:

```bash
# Configure and build
cmake -S . -B build
cmake --build build

# Install to /usr/local/bin
sudo cmake --install build
```

If you use a custom prefix, add it to your PATH:

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

## Requirements

- **CMake**: 3.20 or higher
- **Compiler**: C++17 compatible (GCC 7+, Clang 5+)
- **curl**: For fetching templates from remote repository
- **pthread**: For parallel downloads (usually pre-installed)

## Troubleshooting

### Templates not updating?

Use the refresh flag to force fetch latest templates:
```bash
cpf -f solution.cpp -t dsu --refresh
```

### "No template matched" error?

List available templates by checking the repository, or try a more specific query string.