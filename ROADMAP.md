# Roadmap

The goals to be achieved in the medium and long term.

One of the goals is automated build, but without continuous integration, to create builds with systems like *GitHub Actions*, etc.

Another main goal, extensions and scripting, import and export other channel list formats, add tools, remote control and other utilities.

To discuss the Roadmap, use [Discussions](https://github.com/ctlcltd/e2-sat-editor/discussions/13).


## Enhancements

Less ambitious features:
- import and export connection profiles
- mirror transponders, from Lamedb transponder to XML transponder and vice-versa
- improve management of duplicate channels
- add options to merge on import
- calculate *dvbns* namespace in `e2db`
- assign channel number such as LCN (Logical Channel Number), custom number is partially supported
- provide custom data for each reference
- picon image transformation, resize, apply image masks

Improvements:
- [automated build](https://github.com/ctlcltd/e2-sat-editor/discussions/17)
- less memory usage
- file handling


## Scripting support

A command line **GUI** side to perform actions on running software, **Console**. The language used must be JavaScript alike and the Console will response in JSON format. Using JSON included in *Qt* there are no additional dependencies. The idea is to partially exposes the written `e2db_cli` API and maybe add a **GUI** dependent API part, which will also be used for extensions.

**Console** will look similar to a browser inspector, although with fewer features as it is not a browser.


## Extensions support

Add extension support and use interpreted languages. *Qt* has plugin support, but they should be build. It would be very nice to create add-ons in multiple languages. I experimented with *Swig* and *shared memory* but it adds too much complexity, and to build anyway. Build makes extensibility and deployment complicated.

Keep in mind, the software is cross-platform, build time, limited number of libraries, *socket* require network and permissions, *web services* require network and permissions, *shared memory* requires permissions, sandbox limitations.

Will need an API used by add-ons. The API to operate on channels, import and export channel lists, as well as advanced operations on the **GUI**, for example to add columns and items to tree views, add tabs with a custom view.

Add-ons will come from an official repository, will distributed with tarballs and have a manifest file describing them. It will also be possible to load an add-on from a local environment by granting the authorization.

Extension support might have two interface types: 1) JavaScript built-in, 2) Standard streams.


### 1) Add-on: JavaScript built-in

At the basis of the JavaScript built-in support will be Qt `QJSEngine`. `QJSEngine` is part of `QML` module in Qt. `QML` can be used to extend the **GUI**.

This will require more attention to deployment. `QJSEngine` and `QML` require additional libraries to append to the Qt bundled.

**Pros**
- Built-in JavaScript support
- GUI extensibility

**Cons**
- More Qt dependencies at deploy
- QML is niche


### 2) Add-on: Standard streams

Support with standard streams *stdio* will be achieved with the communication between the running software and an intermediary `host`, with extension running on a language interpreter binary, through *stdin* and *stdout*.

To achieve this support a download manager will be developed, it will download official binaries of language interpreters, for example: *Python3*, *NodeJS*, or: *MicroPython*, *Ducktape*. A download manager will have to check integrity, decompress tarballs, maybe download the *p7zip* binary too. An intermediary `host` will check for integrity anytime, launches sub-processes, handles errors.

Legacy but universal approach. Standard streams do not require network and are available almost everywhere. Confining the binaries of language interpreters is hard. Maybe make this support optional, enabled and disabled.

**Pros**
- Universal add-on support
- Fewer dependencies

**Cons**
- Requires a download manager, decompress tarball, integrity check
- GUI extensibility is difficult
- Single execution
- Latency
- Presence of language interpreter binaries


## Smart Userbouquet

Smart Userbouquets are userbouquets created through filtering. The idea behind Smart Userbouquets is font collections created with metadata.

It will be possible to create Userbouquet from provider name or frequencies, or both.

Distinguish concrete userbouquets from userbouquets to be reviewed and confirmed. Not-to-be-saved userbouquets is partially supported. Provide visual feedback, for example: "Concrete Userbouquet", *"Smart Userbouquet \*"*.


## Multiple files: Enigma2 directory format

The **Enigma2** file format is a directory with multiple files.

It is not possible to create or register a **MIME type** for the **Enigma2** directory file format.

Several environments have limitations in opening and/or saving multiple files and directories: 1) Flatpak sandbox, 2) WebAssembly.


### 1) Flatpak sandbox (save multiple files)

The software is currently published and distributed on Flathub, it is the main *Flatpak* package distribution portal. At the time of the software release it was not possible to save multiple files. Multiple files is the nature of the Enigma2 directory file format. This is the motivation because the software has read and write access on the full filesystem. On the software page on Flathub appears a notice *"Potential unsafe: Full file system read/write access"*. This notice is also on the majority of softwares available through Flathub. Improvements have been made to `xdg-desktop-portal`, need to investigate further.

Need to test again the software to open and save with Flatpak sandbox and restricted permissions.

> [!NOTE]
> Note: *Qt* internally uses *GTK+* API to a limited extent to open and write files and directories.

> [!NOTE]
> Note: *Ubuntu 22.04 LTS* (EOSS: April 2027, EOL: April 2032), contains `xdg-desktop-portal` version 1.14.3, from 2022; *Ubuntu 20.04 LTS* (ESM, EOSS: April 2025, EOL: April 2030), contains `xdg-desktop-portal` version 1.6.0, from 2019; *Ubuntu 18.04 LTS* (ESM, EOSS: June 2023, EOL: April 2028), still mentioned on Flathub, contains `xdg-desktop-portal` version 1.0.3, from 2019.


### 2) WebAssembly (save and open multiple files)

At present the online **demo** has limitations. *Qt for WebAssembly* is used.

On *Qt for WASM* is not possible to open and save multiple files from file dialog. Multiple files is the nature of the Enigma2 directory format. *Qt for WASM* has support to open and save directory, but is currently not implemented in the public API, maybe unstable.

> [!NOTE]
> Note: Qt Wasm versions upper then 6.4.3 seems to have issues with `QDialog`, showing accessibility warns instead of window content.

Need to test again the software for compatibility with Qt version upper than 6.4.3.

Evaluate whether the **demo** with almost full functionality can be useful or not for spreading the software.


## Multi-windowed GUI

Multi-windowed GUI allow multiple software session and multiple window and drag tab from one session to another.

Need to extend: `QTabWidget`, `QTabBar`, `QStackedWidget` to make Drag and Drop of tabs and widgets possible.

Need to extend also: `e2se::gui`, `e2se::tab`, may also be involved *pointers* and *threads*.

This could increase the software complexity.

&nbsp;

Suggestions of any kind, open a [Feature request](https://github.com/ctlcltd/e2-sat-editor/issues/new?template=feature_request.yml).
