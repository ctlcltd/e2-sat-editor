# Installation instructions

Generic instructions to help with compiling e2 SAT Editor on different platforms.

&nbsp;

Utils scripts in **scripts** folder.

- For Linux or BSD or Unix use `scripts/build.sh`

- For macOS with Xcode use `scripts/darwin.sh`

- For Windows with MinGW-64 use `scripts/mingw.sh`


### Required dependencies

Requirements needed to compile:

* build tools
* gcc, g++, clang
* make
* cmake, qmake

Libraries required as dependencies:

* qt6-base \| qt5-base
* libstdc++
* libcurl4

Additional requirements for WebAssembly:

* Qt Wasm
* emscripten
* emsdk
* ninja
* python3


## Windows – MinGW-64 – CMake

Compiling with **CMake** and **MinGW-64** for Windows in Linux host.

I am testing under **Arch Linux** and using **mingw-w64** as guest.

In order to compile, the following packages are required:
base-devel, cmake, mingw-w64-cmake-static, mingw-w64-qt6-base-static, mingw-w64-curl
 
To compile with shared libraries, the following:
mingw-w64-cmake, mingw-w64-qt6-base

&nbsp;

Clone the repository, then from the source directory, execute commands:
```
cd src
x86_64-w64-mingw32-cmake-static -B build
cmake --build build
```

For older Windows 32-bit versions:
```
cd src
i686-mingw32-cmake-static -B build -DWITH_QT5=ON
cmake --build build
```


## macOS - qmake – CMake

Compiling with **qmake** or **CMake** for macOS.

I am testing under **macOS** and using **qmake**.

In order to compile, the following packages are required:
Xcode, Xcode Command Line Tools, Qt, curl

And at your choice:
cmake -or- qmake

&nbsp;

Clone the repository, then from the source directory, execute commands:
```
cd src
qmake -spec macx-clang e2-sat-editor.pro
make && qmake
qmake -spec macx-xcode
```

If you prefer CMake:
```
cd src
cmake -B build
cmake --build build
```


## Linux – BSD - Unix – qmake – CMake

Compiling with **qmake** or **CMake** for Linux or BSD or Unix.

I am testing under **Arch Linux** with Qt6 and under **Ubuntu 20.04 LTS** with Qt5, using **qmake**.

I am testing under **FreeBSD 13** with Qt6, using **qmake**.

In order to compile, the following packages are required, on Arch:
base-devel, qt6-base, curl

On Ubuntu, the following:
build-essentials, qt5, qt5-dev, curl

And at your choice:
cmake -or- qmake

&nbsp;

Clone the repository, then from the source directory, execute commands:
```
cd src
qmake
make
```

If you prefer CMake:
```
cd src
cmake -B build
cmake --build build
```


## WebAssembly – Qt for WebAssembly - CMake

Compiling with **CMake** for Qt for WebAssembly in supported host.

In order to compile, the following packages are required:
g++, clang, cmake, ninja, qt6-base, emscripten, emsdk, python3

Follow the instructions from the Qt Documentation website to right compile Qt for Wasm in your host.

*Note:* Qt Wasm versions upper then 6.4.3 seems to have issues with QDialog, showing accessibility warns instead of window content.

&nbsp;

Clone the repository, then from the source directory, execute commands:
```
export EMSDK=/path/to/emsdk
cp -R e2se-seeds/enigma_db res/enigma_db
mv src/CMakeLists.txt src/CMakeLists.txt.bak
cp dist/wasm/CMakeLists.txt src/CMakeLists.txt
cd src
./qt-everywhere-src/qtbase/bin/qt-cmake .
cmake --build .
```

To match the requirement of the demo, you need also *e2se-seeds/enigma_db* folder from the repository **e2se-seeds**.
```
cp -R e2se-seeds/enigma_db res/enigma_db
```

This is an example on how to compile Qt for WebAssembly from your host environment:
```
export EMSDK=/path/to/emsdk
em++ --version
cd qt-everywhere-src
./configure -qt-host-path /path/to/Qt -platform wasm-emscripten -prefix qtbase
cmake --build . --parallel -t qtbase -t qtimageformats -t qtsvg
```

