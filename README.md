# vdk2dsk

A tool to convert VDK disk images to DSK/JVC format, with options to control header output.

## Getting the Source

You can obtain the source code in two ways:

### 1. Download ZIP or TGZ from GitHub
- Go to the GitHub repository page.
- Click the green "Code" button and select "Download ZIP" (recommended for Windows), or select "Download TAR.GZ" (recommended for Linux/macOS).
- Extract the downloaded archive to a directory of your choice.

### 2. Clone with Git
```sh
git clone https://github.com/bluearcus/vdk2dsk.git
cd vdk2dsk
```

## Usage

```
vdk2dsk [-f] [-[h|m]] filename.vdk

  -f    Force output even if the image size is not as expected
  -h    Write headerless .dsk output (no header at all)
  -m    Write minimal JVC/DSK header (only as many bytes as needed)

Note: The -h and -m options are mutually exclusive; only one may be used at a time.
If neither -h nor -m is specified, a full-length header is written by default.
```

## Building with CMake

### Linux/macOS

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

### Windows (MSVC, MinGW, WSL)

```sh
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"   # or use "MinGW Makefiles" for MinGW
cmake --build .
```

This will produce `vdk2dsk` (or `vdk2dsk.exe` on Windows) in the `build` directory. 