
# BUGS

The following lists of known bugs are categorized by: Fix, Improve, Evaluate, Check; and ordered by priority.

Information on contributions, to contribute and send Pull Request, please see [CONTRIBUTING.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/CONTRIBUTING.md) file.


**To Fix:**

- [x] dialog message displayed without rich-text feature [gui] [checkUpdate.cpp] [piconsView.cpp]
- [x] ftp reconnect, after FTP upload timeout [ftp] [ftpcom.cpp]
- [x] ftp error messages, better message on errors [gui] [tab.cpp]
- [x] tunersets table index, doubled entries after: list item edit, tree drag and drop, tab switch [gui] [tunersetsView.cpp]
- [x] tunersets transponder, wrong tree list item after add and edit [gui] [tunersetsView.cpp]
- [x] parser, transponder frequency and symbol-rate precision (#37) [gui] [e2db.cpp]
- [x] parser, full XML multiline comment parsing (#37) [e2db.cpp]
- [x] parser, wrong transponder internal ID, discarded entries (#37) [e2db.cpp]
- [x] status bar message, message freezed on tab deleting, `QTimer` and timeout call [gui] [tab.cpp]
- [x] drag and drop file, `SEGFAULT` on drop file [gui] [gui.cpp]
- [x] restore window geometry [gui] [gui.cpp]
- [x] preferences, corner menus and `QSettings` syncronization [gui] [settings.cpp]
- [x] AppImage deploy using conda-forge, broken bundle, Qt `QCoreApplication::applicationFilePath` logic and AppRun [qt] [linux]
- [x] AppImage deploy, compatibility, missing Qt QPA wayland plugin [qt] [linux]
- [x] favourite reference, on copy-paste, `#DESCRIPTION` is placed everywhere on userbouquet file [mainView.cpp]
- [x] ftp upload, the current tree list is not stored before upload [tab.cpp] [viewAbstract.cpp] [mainView.cpp]
- [x] edit transponders, sort ordering is not stored [transpondersView.cpp] [tunersetsView.cpp]
- [x] tools popover defects, native `QMenu` `QComboBox` [macos] [gui] [tools.cpp]
- [x] find match cache, is cached after channel deletetion [gui] [viewAbstract.cpp]
- [x] check update, when disconnected, there is no connection error dialog [gui] [checkUpdate.cpp]
- [x] check update, when disconnected, version string `%1` replacement error [gui] [checkUpdate.cpp]
- [x] html markup errors `<tr>` `<thead>` [printable.cpp] [e2db_converter.cpp]
- [x] telnet reload, when connected is always True also when not reloading [ftp] [ftpcom.cpp]
- [x] cli history, PgUp history back [linux] [cli] [e2db_termctl.cpp]


**To Improve:**

- [x] workers refactoring, actions and QThread concurrency [gui] [ftpcom.cpp] [checkUpdate.cpp] [piconsView.cpp]
- [x] ftp connection indicator, better visual feedback [gui] [tab.cpp]
- [x] service and reference flags, show flags in the UI (#33) [gui]
- [x] ftp connection messages, eg. "Connected", better visual feedback, status bar is not enough (#35) [gui] [tab.cpp]
- [x] ftp error messages, from "Access denied to remote resource." to file error. [ftp] [fptcom.cpp]
- [x] convert lamedbx from and to zapitx, better visual feedback [gui] [tab.cpp]
- [x] cli history, load and save [cli] [e2db_termctl.cpp]
- [ ] service flags and compatibility, `dvbfrontend` [gui] [e2db.cpp]
- [ ] reference flags and compatibility, `dvbfrontend` [gui] [e2db.cpp]
- [ ] channel book dialog, service type issue, ex. open from TV, move to Radio [gui] [channelBookView.cpp] [dialChannelBook.cpp]
- [ ] neutrino feparams and versions, unknown compatibility [gui] [e2db.cpp]
- [ ] counters accuracy for items, Total, TV, Radio, Data [gui] [tab.cpp]
- [ ] transponder orbital position column, wrong sorting order A-Z [gui] [viewAbstract.cpp]
- [ ] "userbouquet" translated as "floral bouquets" [l10n]
- [ ] libcurl cabundle dependency [networking]
- [ ] snapcraft and xdg icons [snapcraft]


**To Evaluate:**

- [ ] AppImage deploy, GLIBC/GCC/conda-forge and compatibility [qt] [linux]
- [x] find match cache, is cached after reset, on debug [gui] [viewAbstract.cpp]
- [x] flatpak sandbox, capability to save directory with multiple files, `xdg-desktop-portal` [gui] [gui.cpp] [flatpak]
- [ ] reference, merge markers with same name [gui] [mainView.cpp]
- [ ] force updates from TSV, CSV, eg. rename channels, to change channels data [gui] [mainView.cpp]
- [ ] status bar closed, visual feedback `InfoMessage` downloaded files, no double dialog errors + infos [gui] [tab.cpp]
- [ ] reload option, webif and|or telnet [gui] [tab.cpp] [settings.cpp] [ftpcom.cpp]
- [ ] reload option, deactivate reload, manually [gui] [tab.cpp] [settings.cpp] [ftpcom.cpp]
- [ ] service and reference flags, better visual feedback (#33) [gui]
- [ ] edit service from Channel Book [gui] [channelBookView.cpp]
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
- [ ] import from ftp, tools and demo, expending `import_blob` + `parse_e2db` [e2db.cpp]
- [ ] preferences, settings dialog, on Enter keybind, is not saving settings [gui] [settings.cpp]
- [ ] runtime exceptions, eg. use of `std::runtime_error` causes segfault on mingw32 [code]

