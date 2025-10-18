# Build Instructions

Instructions to help with compiling the software on different platforms.

Utils scripts in `scripts` folder.

- For *Linux* | *BSD* | *Unix* use `scripts/build-qmake.sh`
- For *macOS* with Xcode use `scripts/build-darwin.sh`
- For *Windows* with MinGW-64 use `scripts/build-mingw.sh`

Automated build tasks in `.github/workflows` folder.


## Required Dependencies

Requirements needed to compile:
- build tools (`cmake` `ninja` `qmake`)
- C++17 (`gcc` `clang`)

Libraries required as dependencies:
- `qt6-base`
- `qt6-base-dev`
- `qt6-translations`
- `libcurl4` `libcurl-dev`

Additional requirements for `Qt6PrintSupport` in Linux hosts:
- `cups-dev`

Additional requirements for `Qt for WebAssembly`:
- `Qt for WebAssembly` to cross-compile
- `emsdk` `emscripten`
- `qt6-base`
- `cmake`


## Windows MinGW-64

Build with CMake and MinGW-64 for Windows.

In order to compile, the following packages are required:  
`base-devel` `cmake` `mingw-w64-cmake-static` `mingw-w64-qt6-base-static` `mingw-w64-curl`
 
To build with shared libraries, the following:  
`mingw-w64-cmake` `mingw-w64-qt6-base`

On *MSYS2*, the following:  
`cc` `cmake` `ninja` `qt6-base` `qt6-translations` `curl`

Clone the repository, then from the source directory, execute commands:
```
cd src
x86_64-w64-mingw32-cmake-static -B build
cmake --build build
```

For older *Windows 32-bit* versions:
```
cd src
i686-mingw32-cmake-static -B build -DWITH_QT5=ON
cmake --build build
```


## macOS

Build with CMake or qmake for macOS.

In order to compile, the following packages are required:  
`Xcode` `Xcode Command Line Tools` `Qt` `Curl`

And at your choice:  
`cmake` or `qmake`

Clone the repository, then from the source directory, execute commands:
```
cd src
cmake -B build
cmake --build build
```

If you prefer qmake:
```
cd src
qmake -spec macx-clang e2-sat-editor.pro
make && qmake
qmake -spec macx-xcode
```


## Linux | BSD | Unix

Build with CMake or qmake for Linux | BSD | Unix.

In order to compile, the following packages are required, on *Arch Linux*:  
`base-devel` `qt6-base` `curl`

On *Ubuntu*, the following:  
`curl` `libcurl4-openssl-dev`  
`qt6-base-dev` `qt6-translations-l10n`  
`libgl1-mesa-dev` `libglvnd-dev` `libxcb-xkb-dev` `libxkbcommon-x11-dev` `libvulkan-dev`  
`libcups2-dev`

And at your choice:  
`cmake` or `qmake`

Clone the repository, then from the source directory, execute commands:
```
cd src
cmake -B build
cmake --build build
```

If you prefer qmake:
```
cd src
qmake
make
```


## Qt for WebAssembly

Build with CMake for Qt for WebAssembly on supported host.

In order to cross-compile, the following packages are required:  
`emsdk` `emscripten` `qt6-base` `cmake`

Follow the instructions from the Qt Documentation website to get *Qt for WebAssembly* on your host.

Clone the repository, then from the source directory, execute commands:
```
source /path/to/emsdk/emsdk_env.sh
cd e2-sat-editor
cp -R e2se-seeds/enigma_db res/enigma_db
mv src/CMakeLists.txt src/CMakeLists.txt.bak
cp dist/wasm/CMakeLists.txt src/CMakeLists.txt
cd src
./path/to/Qt/wasm_singlethread/bin/qt-cmake .
cmake --build .
```

You need also `e2se-seeds/enigma_db` folder from the repository `e2se/e2se-seeds`.
```
git clone https://github.com/e2se/e2se-seeds.git
cp -R e2se-seeds/enigma_db res/enigma_db
```

An example on how to build *Qt for WebAssembly* from your host environment.

```
export EMSDK=/path/to/emsdk
em++ --version
cd qt-everywhere-src
./configure -qt-host-path /path/to/Qt -platform wasm-emscripten -prefix qtbase
cmake --build . --parallel -t qtbase -t qtimageformats -t qtsvg
```

