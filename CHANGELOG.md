# Changelog

All notable changes to this project will be documented in this file.


## [v1.7.0]

**Version**: 1.7  
**Release Date**: *unreleased*  
**Full Changelog**: [v1.6.0...HEAD](https://github.com/ctlcltd/e2-sat-editor/compare/v1.6.0...HEAD)

- Added Neutrino terrestrial services support


## [v1.6.0]

**Version**: 1.6  
**Release Date**: 2024-07-25  
**Full Changelog**: [v1.5.0...v1.6.0](https://github.com/ctlcltd/e2-sat-editor/compare/v1.5.0...v.1.6.0)

- Fix crash with empty service parameters on save
- Fix parental lock on write
- Fix a buffer overflow when parsing userbouquet service flag
- Added support for 519 bouquet hidden
- Fix soft errors when listing with FTP command NLST
- Added cross-platform support for CRLF end line on read
- Fix end line from CRLF to LF on write \[Windows\]
- Added button to calculate DVBNS transponder
- Improved merge
- Fix tree highlight color theme bugs \[Windows\]
- Updated translations


## [v1.5.0]

**Version**: 1.5  
**Release Date**: 2024-05-20  
**Full Changelog**: [v1.4.0...v1.5.0](https://github.com/ctlcltd/e2-sat-editor/compare/v1.4.0...v1.5.0)

- Improved Edit menu and context menu
- Fix orbital position with West values
- Added new favourite types, ie. 5001 gstplayer, 5002 exteplayer3
- Fix context menu interfering with Drag and Drop \[macOS\]


## [v1.4.0]

**Version**: 1.4  
**Release Date**: 2024-04-26  
**Full Changelog**: [v1.3.0...v1.4.0](https://github.com/ctlcltd/e2-sat-editor/compare/v1.3.0...v1.4.0)

- Fix FTP issues \[Windows\]
- Fix reading files with end of line CRLF \[Windows\]
- Fix add favourite, stream type
- Fix edit transponder
- Fix merge, clear XML comments
- Fix reference ID and marker number
- Revised transponder feparams
- Win11 initial support \[Windows\]
- Prevent crashes


## [v1.3.0]

**Version**: 1.3  
**Release Date**: 2024-03-30  
**Full Changelog**: [v1.2.0...v1.3.0](https://github.com/ctlcltd/e2-sat-editor/compare/v1.2.0...v1.3.0)

- Type stream reference support
- Calculate \[dvbns\] namespace
- M3U import and export
- Automatic and manual checking for updates
- Bugfix


## [v1.2.0]

**Version**: 1.2  
**Release Date**: 2024-02-22  
**Full Changelog**: [v1.1.1...v1.2.0](https://github.com/ctlcltd/e2-sat-editor/compare/v1.1.1...v1.2.0)

- Connection presets with most common Set-Top-Box settings
- Connection profile import and export
- FTP improvements
- Heavy bugfix
- Introducing stream type support (only parse and save)


## [v1.1.1]

**Version**: 1.1.1 
**Release Date**: 2024-01-20  
**Full Changelog**: [v1.1.0...v.1.1.1](https://github.com/ctlcltd/e2-sat-editor/compare/v1.1.0...v.1.1.1)

- Fix FTP issue, file listing and directory


## [v1.1.0]

**Version**: 1.1  
**Release Date**: 2024-01-15  
**Full Changelog**: [v1.0.0...v1.1.0](https://github.com/ctlcltd/e2-sat-editor/compare/v1.0.0...v1.1.0)

- Important fixes: remove channel reference, remove service, Drag and Drop logic, tree column widths
- Prevent crash and strange behaviour


## [v1.0.0]

**Version**: 1.0  
**Release Date**: 2024-01-04  
**Full Changelog**: [v0.9.0...v1.0.0](https://github.com/ctlcltd/e2-sat-editor/compare/v0.9.0...v1.0.0)

- File Information dialog
- Online Help
- Bugfix


## [v0.9.0]

**Version**: 0.9  
**Release Date**: 2023-09-26  
**Full Changelog**: [v0.8.0...v0.9.0](https://github.com/ctlcltd/e2-sat-editor/compare/v0.8.0...v0.9.0)

- Translations, 33 languages (same as qt-base): Arabic, Bulgarian, Catalan, Czech, Danish, German, Spanish, Persian, Finnish, French, Gaelic, Galician, Hebrew, Croatian, Hungarian, Italian, Japanese, Korean, Lithuanian, Latvian, Dutch, Norwegian Nynorsk, Polish, Portuguese (Brazil), Portuguese (Portugal), Russian, Slovak, Slovenian, Swedish, Turkish, Ukrainian, Chinese (China), Chinese (Taiwan)
- Bugfix
- Win10 dark mode \[Windows\]


## [v0.8.0]

**Version**: 0.8  
**Release Date**: 2023-06-13  
**Full Changelog**: [v0.7.0...v0.8.0](https://github.com/ctlcltd/e2-sat-editor/compare/v0.7.0...v0.8.0)

- Bugfix, stability, Qt compatibility
- Added Picons editor
- Threaded FTP
- Fix file operations, edit service fields, XML encoding
- Improved CLI


## [v0.7.0]

**Version**: 0.7  
**Release Date**: 2023-05-21  
**Full Changelog**: [v0.6.0-pre...v0.7.0](https://github.com/ctlcltd/e2-sat-editor/compare/v0.6.0-pre...v0.7.0)

- Improvements: tree views, log inspector, logger, icon resources
- Testing and fixing for platforms
- Added initial CLI
- Added WASM support Qt for WebAssembly \[Wasm\]


## [v0.6.0-pre]

**Version**: 0.6 pre-release  
**Release Date**: 2023-04-07  
**Full Changelog**: [v0.5.0-pre...v0.6.0-pre](https://github.com/ctlcltd/e2-sat-editor/compare/v0.5.0-pre...v0.6.0-pre)

- Fix clipboard releated issues
- Improved Drag and Drop and copy/paste operations
- Improved merge
- Improved RTL support
- Revised source code and translation support


## [v0.5.0-pre]

**Version**: 0.5 pre-release  
**Release Date**: 2023-03-18  
**Full Changelog**: [v0.4.0-pre...v0.5.0-pre](https://github.com/ctlcltd/e2-sat-editor/compare/v0.4.0-pre...v0.5.0-pre)

- Added support for Neutrino Zapit XML channel lists
- Fix XML parser bug under MinGW
- Revised file operations
- Improved Drag and Drop
- Parser performance improvements
- Fix Qt5 compatibility issues

## [v0.4.0-pre]

**Version**: 0.4 pre-release  
**Release Date**: 2023-02-25  
**Full Changelog**: [v0.3.0-pre...v0.4.0-pre](https://github.com/ctlcltd/e2-sat-editor/compare/v0.3.0-pre...v0.4.0-pre)

- Editor edit views refactoring
- Added support for Lamedb 2.3 2.2 channel lists
- Parental lock feature
- Fix sprintf deprecation, integer width, leak analysis
- Fix dependency update libcurl-4 releated issues

## [v0.3.0-pre]

**Version**: 0.3 pre-release  
**Release date**: 2023-01-24  
**Full Changelog**: [v0.3.0-pre](https://github.com/ctlcltd/e2-sat-editor/commits/v0.3.0-pre)

- Fix Qt6 and Qt5 compatibility
- Revised layout and Qt stylesheet
- Macx style improvements \[macOS\]
- First pre-release

