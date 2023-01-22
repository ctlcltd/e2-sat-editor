## Installation instructions

Generic instructions to help with compiling e2 SAT Editor on different platforms.

&nbsp;
Utils scripts in **scripts** folder.

- For Linux or BSD or unix use scripts/build.sh

- For macOS with Xcode use scripts/darwin.sh

- For Windows with MinGW-64 use scripts/mingw32.sh


### Required dependencies

Requirements needed to compile:

* build tools
* gcc, g++, clang
* make
* cmake, qmake

Libraries required as dependencies:

* qt6-base \| qt5-base
* libstdc++ >= 3
* libcurl


## Windows – MinGW-64 – CMake

Compiling with **CMake** and **MinGW-64** for Windows in Linux host.

I am testing under **Arch Linux** and using **mingw-w64** as guest.

In order to compile, the following packages are required:
base-devel, make, mingw-w64-cmake-static, mingw-w64-qt6-base-static, mingw-w64-curl
 
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


## Linux – BSD - unix – qmake – CMake

Compiling with **qmake** or **CMake** for Linux or BSD or unix.

I am testing under **Arch Linux** with Qt6 and under **Ubuntu LTS** with Qt5, using **qmake**.

In order to compile, the following packages are required, on Arch:
base-devel, make, qt6-base, curl

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


<!-- ### Releases

🆕 **There are pre-releases available for testing** 📡

Downloads from the Release page: https://github.com/ctlcltd/e2-sat-editor/releases -->

