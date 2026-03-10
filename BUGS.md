
# BUGS

The following lists of known bugs are categorized by: Fix, Improve, Evaluate, Check; and ordered by priority.

Information on contributions, to contribute and send Pull Request, please see [CONTRIBUTING.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/CONTRIBUTING.md) file.


**To Fix:**

- [x] favourite reference, on copy-paste, `#DESCRIPTION` is placed everywhere on userbouquet file [mainView.cpp]
- [x] ftp upload, the current tree list is not stored before upload [tab.cpp] [viewAbstract.cpp] [mainView.cpp]
- [x] edit transponders, sort ordering is not stored [transpondersView.cpp] [tunersetsView.cpp]
- [x] tools popover defects, native `QMenu` `QComboBox` [macos] [gui] [tools.cpp]
- [x] find match cache, is cached after channel deletetion [gui] [viewAbstract.cpp]
- [x] check update, when disconnected, there is no connection error dialog [checkUpdate.cpp]
- [x] check update, when disconnected, version string `%1` replacement error [checkUpdate.cpp]
- [x] html markup errors `<tr>` `<thead>` [printable.cpp] [e2db_converter.cpp]
- [x] telnet reload, when connected is always True also when not reloading [ftp] [ftpcom.cpp]
- [x] cli history, PgUp history back [linux] [cli] [e2db_termctl.cpp]


**To Improve:**

- [x] service and reference flags, show flags in the UI, eg. Issues (#33) [gui]
- [x] ftp connection messages, eg. "Connected", better visual feedback, status bar is not enough, eg. Issues (#35) [gui] [tab.cpp]
- [x] ftp error messages, from "Access denied to remote resource." to file error. [ftp] [fptcom.cpp]
- [ ] convert lamedbx from and to zapitx, better visual feedback [gui] [tab.cpp]
- [ ] cli history, load and save [cli] [e2db_termctl.cpp]
- [ ] service flags and compatibility, `dvbfrontend` [gui] [e2db.cpp]
- [ ] reference flags and compatibility, `dvbfrontend` [gui] [e2db.cpp]
- [ ] neutrino feparams and versions, unknown compatibility [gui] [e2db.cpp]
- [ ] counters accuracy for items, Total, TV, Radio, Data [gui] [tab.cpp]
- [ ] transponder orbital position column, wrong sorting order A-Z [gui] [viewAbstract.cpp]
- [ ] "userbouquet" translated as "floral bouquets" [l10n]
- [ ] libcurl cabundle dependency [networking]
- [ ] snapcraft and xdg icons [snapcraft]


**To Evaluate:**

- [x] find match cache, is cached after reset, on debug [gui] [viewAbstract.cpp]
- [ ] flatpak sandbox, capability to save directory with multiple files, `xdg-desktop-portal` [gui] [gui.cpp] [flatpak]
- [ ] reference, merge markers with same name [gui] [mainView.cpp]
- [ ] force updates from TSV, CSV, eg. rename channels, to change channels data [gui] [mainView.cpp]
- [ ] status bar closed, visual feedback `InfoMessage` downloaded files, no double dialog errors + infos [gui] [tab.cpp]
- [ ] reload option, webif and|or telnet [gui] [tab.cpp] [settings.cpp] [ftpcom.cpp]
- [ ] reload option, deactivate reload, manually [gui] [tab.cpp] [settings.cpp] [ftpcom.cpp]
- [ ] service and reference flags, find and filtering, eg. Issues (#33) [gui]
- [ ] FTPS, HTTPS support [networking] [gui] [settings.cpp] [fptcom.cpp]
- [ ] find toolbar, look and feel [windows] [linux] [gui]
- [ ] items move and copy, from and to, drag and drop is not enough [gui] [cli]
- [ ] lamedb5 comments support [e2db.cpp]
- [ ] neutrino cable, atsc, unknown support [e2db.cpp]
- [ ] neutrino webtv support, Issues (#35) [e2db.cpp]
- [ ] parental userbouquet `FROM BOUQUET`, unknown compatibility [e2db.cpp]
- [ ] parental userbouquet `FROM BOUQUET`, visual feedback icon [gui] [e2db.cpp]
- [ ] ftp callbacks, `resetStatusBar` is called twice [gui] [tab.cpp]
- [ ] usability, import multiple selection [gui] [tab.cpp]
- [ ] usability, import directory selection [gui] [tab.cpp]
- [ ] A11y Accessibility status [a11y] [gui]
- [ ] HiDPI, font sizes [gui]
- [ ] RTL support, `LRM` text is not enough [rtl]
- [ ] picons editor, `QListWidget::IconMode` lazy load [gui] [piconsView.cpp]
- [ ] debug to stdout is not working on silicon [arm64] [macos]
- [ ] ftp debug, bad stdout output [ftp]


**To Check:**

- [x] telnet reload, it connects but seems to not working [ftp] [ftpcom.cpp]
- [x] telnet emulation py script, seems to not working [workspace] [start-telnet.py]
- [x] combobox defects, `QComboBox`, eg. sort menu, edit service [qt-wasm] [gui]
- [x] edit service on demo, collision dialog is not displayed, dialogs limitation [qt-wasm] [gui] [editService.cpp]
- [x] snapcraft cmake log, `libcurl4t64` conflict, `kde-neon-6` [build] [snapcraft]
- [ ] ftp download, counters issue [gui] [tab.cpp]
- [ ] singular userbouquet import, in original sorting order, `parse_e2db_bouquets` [e2db.cpp]
- [ ] neutrino terrestrial, wrong syntax, eg. `name="NaN"` `name="0.0E"` [e2db.cpp]
- [ ] `onid == 0x0001` is `valid = (tsid > 0x0001)`, `valid = true` is False on STB [e2db.cpp]
- [ ] edit service from Channel Book [gui] [channelBookView.cpp]
- [ ] import from ftp, tools and demo, expending `import_blob` + `parse_e2db` [e2db.cpp]
- [ ] settings dialog, on Enter keybind, is not saving settings [gui] [settings.cpp]
- [ ] runtime exceptions, eg. use of `std::runtime_error` causes segfault on mingw32 [code]

