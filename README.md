## e2 SAT Editor

An enigma2 📡 channel list editor – under development.

[![A screenshot of e2 SAT Editor](https://github.com/ctlcltd/e2-sat-editor/blob/main/res/screenshot.png "e2 SAT Editor (screenshot)")](https://github.com/ctlcltd/e2-sat-editor/blob/main/res/screenshot.png?raw=true)

**e2 SAT Editor** is an Enigma 2 channel list editor, fully cross-platform (Windows, macOS, Linux) and free to use.

Many features are under the hood and many are coming. It has tabbed navigation with global Cut-Copy-Paste support; Drag and Drop support; import and export of Lamedb, bouquets and userbouquets; FTP support; tools for sorting and management of channel lists; a native OS look and feel with dark and light themes.

It supports both Lamedb 4 and Lamedb 5 formats (2.4 and 2.5).

<img src="res/e2-sat-editor.svg" width="192" height="192" alt="e2 SAT Editor (icon)" title="Icon">


### Requirements

* build tools (gcc, g++, make, cmake, qmake)
* C++ 17
* Qt 6 \| Qt 5


### Dependencies

* libstdc++ >= 3
* libcurl
* qt6-base \| qt5-base


&nbsp;
## Get involved

Your contribution to this project are amazing.

You can open [issues](https://github.com/ctlcltd/e2-sat-editor/issues) to report bug, request features and send [Pull Request](https://github.com/ctlcltd/e2-sat-editor/pulls).

There are a few important things you should know to better contribute to this project.

Please read the [contribution guidelines](https://github.com/ctlcltd/e2-sat-editor/blob/main/CONTRIBUTING.md).


## Development

- Before start, you need to install build tools and Qt: https://www.qt.io/download

- You might need libcurl installed with headers, details here: https://curl.se/download.html

- *Please note:* depending on your OS environment, you should install some dependencies (devel).

- Then clone the repository:

```git clone https://github.com/ctlcltd/e2-sat-editor.git```

- In linux and \*bsd use scripts/build.sh to build

- In macos w/ xcode use scripts/darwin.sh to build

- With mingw32 use scripts/mingw32.sh to build


## Contents

|Folder|Description|
|-|-|
|**dist**|Specific target files, required during build.|
|**res**|Graphic resources, used in this software.|
|**scripts**|Utility bash scripts, used to build.|
|**[seeds.](https://ctlcltd.github.io/e2-sat-editor/seeds./)**|Source seeds, Enigma2 settings format.|
|**src**|The main source of this software.|
|**vendor**|It contains symbolic links to external libraries.|
|**workspace**|Utility python scripts, used for emulation: ftp, telnet, webif.|


&nbsp;
### TODO

- [x] search filters
- [x] terrestrial.xml and cable.xml support
- [ ] satellites.xml editing
- [ ] print and import/export CSV
- [ ] stable build
- [ ] smart edit userbouquets
- [ ] minisite and promotion
- [ ] cli
- [ ] optimizations
- [ ] basic translations
- [ ] multi-windowed gui
- [ ] plug-in system
- [ ] picons support
- [ ] automated build


&nbsp;
## License

Source code licensed under the terms of the [MIT License](https://github.com/ctlcltd/e2-sat-editor/blob/main/LICENSE). It is also licensed under the terms of the [GNU GPLv3](https://github.com/ctlcltd/e2-sat-editor/blob/main/COPYING).

This software will be distribuited under the terms of the GNU GPLv3.
