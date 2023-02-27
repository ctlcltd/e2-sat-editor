## e2 SAT Editor

e2 SAT Editor is an Enigma 2 channel list editor, cross-platform (Windows, macOS, Linux) and free to use.

<img src="https://github.com/ctlcltd/e2-sat-editor/raw/main/res/e2-sat-editor.svg" width="192" height="192" alt="e2 SAT Editor (icon)" title="Icon">

It has tabbed navigation with global Cut-Copy-Paste support; Drag and Drop support; import and export of Lamedb, bouquets and userbouquets; FTP support; tools for sorting and management of channel lists; a native OS look and feel with dark and light themes.

It supports both Enigma2, Lamedb 4 and Lamedb 5 formats (2.4 and 2.5) and Enigma1 (2.3 and 2.2).

&nbsp;

🆕 **There are pre-releases available for testing** 📡

[![A screenshot of e2 SAT Editor](https://github.com/ctlcltd/e2-sat-editor/raw/main/res/screenshot.png "e2 SAT Editor (screenshot)")](https://github.com/ctlcltd/e2-sat-editor/blob/main/res/screenshot.png?raw=true)


### Requirements

* build tools (gcc, g++, make, cmake, qmake)
* C++ 17
* Qt 6 \| Qt 5


### Dependencies

* qt6-base \| qt5-base
* libstdc++ >= 3
* libcurl

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

&nbsp;

Instructions on how to compile in [INSTALL.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/INSTALL.md) file.


## Contents

|Folder|Description|
|-|-|
|**dist**|Specific target files, required during build.|
|**res**|Graphic resources, used in this software.|
|**scripts**|Utility bash scripts, used to build.|
|**src**|The main source of this software.|
|**vendor**|It contains symbolic links to external libraries.|
|**workspace**|Utility python scripts, used for emulation: ftp, telnet, webif.|

&nbsp;

### TODO

- [x] search filters
- [x] terrestrial.xml and cable.xml support
- [x] satellites.xml editing
- [x] print and import/export CSV
- [ ] smart edit userbouquets
- [ ] cli
- [ ] basic translations
- [ ] multi-windowed gui
- [ ] extensions and/or scripting
- [ ] picons support
- [ ] automated build

&nbsp;

## License

Source code licensed under the terms of the [MIT License](https://github.com/ctlcltd/e2-sat-editor/blob/main/LICENSE-MIT). It is also licensed under the terms of the [GNU GPLv3](https://github.com/ctlcltd/e2-sat-editor/blob/main/LICENSE-GPL-3.0-or-later).

This software will be distribuited under the terms of the GNU GPLv3.

