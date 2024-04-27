## e2 SAT Editor

e2 SAT Editor is a satellite channel lists editor, for Enigma2, Neutrino, dreambox lists.

Cross-platform (Windows, macOS, Linux, Unix-like BSD) and free.

<img src="https://github.com/ctlcltd/e2-sat-editor/raw/main/res/e2-sat-editor.svg" width="192" height="192" alt="e2 SAT Editor (icon)" title="Icon">

It has tabbed navigation with global Cut-Copy-Paste support; Drag and Drop support; import and export of Lamedb, bouquets and userbouquets; FTP support; built-in Picons editor; tools for sorting and management of channel lists; non-destructive editing; dark and light themes.

It supports both Enigma2, Lamedb 4 and Lamedb 5 formats (2.4 and 2.5) and Enigma1 (2.3 and 2.2).

And it supports also Neutrino xml (api v1, v2, v3, v4).

&nbsp;

📡 **Download new releases from [Releases](https://github.com/ctlcltd/e2-sat-editor/releases)**  

💡 **Try now online DEMO: https://ctlcltd.github.io/e2se-wasm-demo/**  

&nbsp;

> [!NOTE]
> Demo has limitations imposed by remote use, you cannot open and save files other than the demo ones included.


## Screenshot

*Sample of the software running on Windows*
[![e2 SAT Editor (screenshot sample on Windows)](https://github.com/ctlcltd/e2-sat-editor/raw/main/res/screenshot-wlw.webp "Sample of the software running on Windows")](https://github.com/ctlcltd/e2-sat-editor/blob/main/res/screenshot-wlw.webp?raw=true)

*Sample of the software running on macOS with dark appearance variant*
[![e2 SAT Editor (screenshot sample on macOS)](https://github.com/ctlcltd/e2-sat-editor/raw/main/res/screenshot-mdm.webp "Sample of the software running on macOS with dark appearance variant")](https://github.com/ctlcltd/e2-sat-editor/blob/main/res/screenshot-mdm.webp?raw=true)

*Sample of the software running with (Qt) Fusion appearance*
[![e2 SAT Editor (screenshot sample)](https://github.com/ctlcltd/e2-sat-editor/raw/main/res/screenshot-flf.webp "Sample of the software running with (Qt) Fusion appearance")](https://github.com/ctlcltd/e2-sat-editor/blob/main/res/screenshot-flf.webp?raw=true)



## Get involved

You can open [issues](https://github.com/ctlcltd/e2-sat-editor/issues) to report bug, request features and send [Pull Request](https://github.com/ctlcltd/e2-sat-editor/pulls)

[Contribution guidelines](https://github.com/ctlcltd/e2-sat-editor/blob/main/CONTRIBUTING.md)


## Roadmap

The Roadmap contains ± ambitious points.

Read the full [Roadmap](https://github.com/ctlcltd/e2-sat-editor/blob/main/ROADMAP.md)


## Translation

The software is available in 33 languages: Arabic, Bulgarian, Catalan, Czech, Danish, German, Spanish, Persian, Finnish, French, Gaelic, Galician, Hebrew, Croatian, Hungarian, Italian, Japanese, Korean, Lithuanian, Latvian, Dutch, Norwegian Nynorsk, Polish, Portuguese (Brazil), Portuguese (Portugal), Russian, Slovak, Slovenian, Swedish, Turkish, Ukrainian, Chinese (China), Chinese (Taiwan).

New languages can be added.

Contribute to translations on the website: https://e2sateditor.com/translate/


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


## Contents

|Folder|Description|
|-|-|
|**dist**|Target files for platforms|
|**res**|Resource files and screenshots|
|**scripts**|Utils bash scripts|
|**src**|The source of this software|
|**translations**|Translation source files|
|**workspace**|Utils python scripts, emulation: ftp, telnet, webif|


## License

Source code licensed under the terms of the [MIT License](https://github.com/ctlcltd/e2-sat-editor/blob/main/LICENSE-MIT)

It is also licensed under the terms of the [GNU GPLv3 License](https://github.com/ctlcltd/e2-sat-editor/blob/main/LICENSE-GPL-3.0-or-later)

This software will be distribuited under the terms of the GNU GPLv3 License.

