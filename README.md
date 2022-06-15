## e2 SAT Editor

An enigma2 📡 channel list editor – under development.

[![A screenshot of e2 SAT Editor](res/screenshot.jpg "e2 SAT Editor (screenshot)")](res/screenshot.png?raw=true)

**e2 SAT Editor** is an Enigma 2 channel list editor, fully cross-platform (Windows, macOS, Linux) and free to use.

Many features are under the hood and many are coming. It has tabbed navigation with global Cut-Copy-Paste support; Drag and Drop support; import and export of Lamedb, bouquets and userbouquets; FTP support; tools for sorting and management of channel lists; a native OS look and feel with dark and light themes.

It supports Lamedb 4 and Lamedb 5 formats.

<img src="res/e2-sat-editor.svg" width="192" height="192" alt="e2 SAT Editor (icon)" title="Icon">


## Requirements

* build tools (gcc, g++, make, cmake, qmake)
* C++ 17
* Qt 6 / Qt 5


## Dependencies

* libstdc++ >= 3
* libcurl
* qt6-base | qt5-base


## Development

- Before start, you need to install build tools and Qt: https://www.qt.io/download

- You might need libcurl installed with headers, details here: https://curl.se/download.html

- *Please note:* depending on your OS environment, you should install some dependencies (devel).

- Then clone the repository:

```git clone https://github.com/ctlcltd/e2-sat-editor.git```

- In linux and \*bsd use scripts/build.sh to build

- In macos w/ xcode use scripts/darwin.sh to build

- With mingw32 use scripts/mingw32.sh to build

&nbsp;
### TODO

- [ ] stable build
- [x] search filters
- [ ] satellites.xml editing
- [ ] multi-windowed gui
- [ ] minisite and promotion
- [ ] basic translations
- [ ] automated build
- [ ] terrestrial.xml and cable.xml support
- [ ] smart edit userbouquets
- [ ] picons support
- [ ] plug-in system
- [ ] cli
- [ ] optimizations

&nbsp;
## Get involved

You can open [issues](https://github.com/ctlcltd/e2-sat-editor/issues) to report bug, request features or send a [Pull Request](https://github.com/ctlcltd/e2-sat-editor/pulls).

&nbsp;
## License

Source code licensed under the terms of the [MIT License](LICENSE). It is also licensed under the terms of the [GNU GPLv3](src/COPYING).

This software will be distribuited under the terms of the GNU GPLv3.

[MIT License](LICENSE).
