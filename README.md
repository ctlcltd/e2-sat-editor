## e2 SAT Editor

e2 SAT Editor is a satellite channel lists editor, for Enigma2, Neutrino, dreambox lists.

Cross-platform (Windows, macOS, Linux, Unix-like BSD) and free to use.

<img src="https://github.com/ctlcltd/e2-sat-editor/raw/main/res/e2-sat-editor.svg" width="192" height="192" alt="e2 SAT Editor (icon)" title="Icon">

It has tabbed navigation with global Cut-Copy-Paste support; Drag and Drop support; import and export of Lamedb, bouquets and userbouquets; FTP support; built-in Picons editor; tools for sorting and management of channel lists; non-destructive editing; dark and light themes.

It supports both Enigma2, Lamedb 4 and Lamedb 5 formats (2.4 and 2.5) and Enigma1 (2.3 and 2.2).

And it supports also Neutrino xml (api v1, v2, v3, v4).

&nbsp;

🆕 **Releases available to download from [Releases](https://github.com/ctlcltd/e2-sat-editor/releases)** 📡

*Sample of the software running on macOS 12 with default appearance variant (Vibrant Light)*
[![A screenshot of e2 SAT Editor](https://github.com/ctlcltd/e2-sat-editor/raw/main/res/screenshot.webp "e2 SAT Editor (screenshot)")](https://github.com/ctlcltd/e2-sat-editor/blob/main/res/screenshot.webp?raw=true)


## Try the demo

Experiment online **DEMO**: https://github.com/ctlcltd/e2se-wasm-demo

The demo was built with *Qt for WebAssembly*.

&nbsp;

## Get involved

Your contribution to this project are amazing.

You can open [issues](https://github.com/ctlcltd/e2-sat-editor/issues) to report bug, request features and send [Pull Request](https://github.com/ctlcltd/e2-sat-editor/pulls)

Please read the [contribution guidelines](https://github.com/ctlcltd/e2-sat-editor/blob/main/CONTRIBUTING.md)


## Roadmap

The Roadmap contains ± ambitious points.

A list of points from the Roadmap:
- Enhancements
- Scripting support
- Extensions support
  - Add-on: JavaScript built-in
  - Add-on: Standard streams
- Smart Userbouquet
- Multiple files: Enigma2 directory format
  - Flatpak sandbox (save multiple files)
  - WebAssembly (save and open multiple files)
- Multi-windowed GUI

Please read the full [Roadmap](https://github.com/ctlcltd/e2-sat-editor/blob/main/ROADMAP.md)


## Translation

The software is available in 33 languages: Arabic, Bulgarian, Catalan, Czech, Danish, German, Spanish, Persian, Finnish, French, Gaelic, Galician, Hebrew, Croatian, Hungarian, Italian, Japanese, Korean, Lithuanian, Latvian, Dutch, Norwegian Nynorsk, Polish, Portuguese (Brazil), Portuguese (Portugal), Russian, Slovak, Slovenian, Swedish, Turkish, Ukrainian, Chinese (China), Chinese (Taiwan)

To contribute there is **Translations** app *\[beta\]* on the website: https://e2sateditor.com/translate/

Same languages as *Qt*, new languages could be added using the website app.


## Development

Before start, you need to install build tools and Qt: https://www.qt.io/download

You might need Curl installed with headers, details here: https://curl.se/download.html

> [!NOTE]
> Depending on your OS environment, you should install some dependencies (devel)

> [!TIP]
> Instructions on how to build in [BUILD.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/BUILD.md) file

Clone the repository:
```
git clone https://github.com/ctlcltd/e2-sat-editor.git
```

**Requirements:**
- build tools (`qmake` `cmake` `ninja`)
- C++17 (`gcc` `clang`)
- `qt6-base` \| `qt5-base`
- `libcurl4`

&nbsp;

## Contents

|Folder|Description|
|-|-|
|**dist**|Specific target files, required during build|
|**res**|Resource files, used in this software|
|**scripts**|Utility bash scripts, used to build|
|**src**|The main source of this software|
|**translations**|Translation source files for gui (Qt)|
|**workspace**|Utility python scripts, used for emulation: ftp, telnet, webif|

&nbsp;

## License

Source code licensed under the terms of the [MIT License](https://github.com/ctlcltd/e2-sat-editor/blob/main/LICENSE-MIT)

It is also licensed under the terms of the [GNU GPLv3 License](https://github.com/ctlcltd/e2-sat-editor/blob/main/LICENSE-GPL-3.0-or-later)

This software will be distribuited under the terms of the GNU GPLv3 License.

