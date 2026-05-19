
# BUGS

The following lists of known bugs are categorized by: Fix, Improve, Evaluate, Check; and ordered by priority.

Information on contributions, to contribute and send Pull Request, please see [CONTRIBUTING.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/CONTRIBUTING.md) file.


**To Fix:**

- [ ] preferences, freeze on settings save, `tab::ftpComboItems`, no debug trace [gui] [settings.cpp] [tab.cpp]
- [ ] cli history, no stepping, history loads, `std::iostream` `EOF` [cli] [e2db_termctl.cpp] [windows]
- [x] preferences, segfault on settings save, `qt-6.4.2`, `tab::ftpComboChanged` `QComboBox::setCurrentIndex` [gui] [settings.cpp] [tab.cpp]
- [x] find match cache, cached after drop, drag and drop [gui] [mainView.cpp] [viewAbstract.cpp]
- [x] tab name, count, file name, file dialog filepath, trailing slash [gui] [gui.cpp] [tab.cpp]
- [x] cli history, load, fix CRLF [cli] [e2db_termctl.cpp]
- [x] channel book dialog, filter service type toggler [gui] [dialChannelBook.cpp] [channelBookView.cpp]
- [x] dialog message displayed without rich-text feature [gui] [checkUpdate.cpp] [piconsView.cpp]
- [x] ftp reconnect, after FTP upload timeout [ftpcom]
- [x] ftp error messages, better message on errors [gui] [tab.cpp]
- [x] tunersets table index, doubled entries after: list item edit, tree drag and drop, tab switch [gui] [tunersetsView.cpp]
- [x] tunersets transponder, wrong tree list item after add and edit [gui] [tunersetsView.cpp]
- [x] parser, transponder frequency and symbol-rate precision (issues:#37) [gui] [e2db]
- [x] parser, full XML multiline comment parsing (issues:#37) [e2db]
- [x] parser, wrong transponder internal ID, discarded entries (issues:#37) [e2db]
- [x] status bar message, message freezed on tab deleting, `QTimer` and timeout call [gui] [tab.cpp]
- [x] drag and drop file, segfault on drop file [gui] [gui.cpp]
- [x] restore window geometry [gui] [gui.cpp]
- [x] preferences, corner menus and `QSettings` syncronization [gui] [settings.cpp]
- [x] AppImage deploy using conda-forge, broken bundle, Qt `QCoreApplication::applicationFilePath` logic and `AppRun` [qt] [linux]
- [x] AppImage deploy, compatibility, missing Qt QPA wayland plugin [qt] [linux]
- [x] ftp upload, the current tree list is not stored before upload [gui] [tab.cpp] [viewAbstract.cpp] [mainView.cpp]
- [x] edit transponders, sort ordering is not stored [gui] [transpondersView.cpp] [tunersetsView.cpp]
- [x] find match cache, is cached after channel deletetion [gui] [viewAbstract.cpp]
- [x] telnet reload, when connected is always True also when not reloading [ftpcom]
- [x] cli history, `PgUp` history back [cli] [e2db_termctl.cpp] [linux]


**To Improve:**

- [ ] service flags and compatibility, `dvbfrontend` [gui] [e2db]
- [ ] reference flags and compatibility, `dvbfrontend` [gui] [e2db]
- [ ] counters accuracy for items, Total, TV, Radio, Data [gui] [tab.cpp]
- [ ] delete bouquet, file is not deleted, permanent delete method [gui] [mainView.cpp]
- [ ] lamedb 2.5 2.4 differs, cached pid, caid [e2db]
- [ ] transponder orbital position column, wrong sorting order A-Z [gui] [viewAbstract.cpp]
- [ ] "userbouquet" translated as "floral bouquets" [l10n]
- [ ] libcurl cabundle dependency [networking]
- [ ] snapcraft and xdg icons [xdg] [snapcraft]
- [x] tree column width, font size metrics [gui] [viewAbstract.cpp]
- [x] reference panel, PIDs top alignment [gui] [mainView.cpp]
- [x] filepath directory separator [gui] [windows]
- [x] neutrino feparams and versions, unknown compatibility, `dvbfrontend` [gui] [e2db]
- [x] workers refactoring, actions and `QThread` concurrency [gui] [ftpcom] [checkUpdate.cpp] [piconsView.cpp]
- [x] ftp connection indicator, better visual feedback [gui] [tab.cpp]
- [x] service and reference flags, show flags in the UI (issues:#33) [gui]
- [x] ftp connection messages, eg. "Connected", better visual feedback, status bar is not enough (issues:#35) [gui] [tab.cpp]
- [x] ftp error messages, from "Access denied to remote resource." to file error. [fptcom]
- [x] convert lamedbx from and to zapitx, better visual feedback [gui] [tab.cpp]
- [x] cli history, load and save [cli] [e2db_termctl.cpp]


**To Evaluate:**

- [ ] service and reference flags, better visual feedback (issues:#33) [gui]
- [ ] neutrino feparams: frequency, symbol rate, `dvbfrontend` [e2db]
- [ ] scale factor, dialog zorder, Qt QPA wayland, eg. `ubuntu` `kde-neon-6` [qt] [linux]
- [ ] AppImage deploy, GLIBC/GCC/conda-forge and compatibility [qt] [linux]
- [ ] stdout notices, `QObject::killTimer`, QTimer id -1 not valid, maybe `tab::bubbleMessage` [gui] [tab.cpp]
- [ ] ftp upload resume [ftpcom]
- [ ] edit service from Channel Book [gui] [channelBookView.cpp]
- [ ] status bar closed, visual feedback `tab::InfoMessage` downloaded files, no double dialog errors + infos [gui] [tab.cpp]
- [ ] reload option, webif and|or telnet [gui] [tab.cpp] [settings.cpp] [ftpcom]
- [ ] reload option, deactivate reload, manually [gui] [tab.cpp] [settings.cpp] [ftpcom]
- [ ] desktop entry xdg, translated values [l10n] [xdg]
- [ ] FTPS, HTTPS support [networking] [gui] [settings.cpp] [fptcom]
- [ ] lamedb 2.5, comments support [e2db]
- [ ] neutrino webtv support (issues:#35) [e2db]
- [ ] reference, merge markers with same name [gui] [mainView.cpp]
- [ ] force updates from TSV, CSV, eg. rename channels, to change channels data [gui] [mainView.cpp]
- [ ] tab default focus, tab profile `QComboBox`, eg. floating dock widget [gui] [tab.cpp] [viewAbstract.cpp]
- [ ] find toolbar, look and feel [gui] [windows] [linux]
- [ ] items move and copy, from and to, drag and drop is not enough [gui] [cli]
- [ ] parental userbouquet `FROM BOUQUET`, unknown compatibility [e2db]
- [ ] parental userbouquet `FROM BOUQUET`, visual feedback icon [gui] [e2db]
- [ ] ftp callbacks, `tab::resetStatusBar` is called twice [gui] [tab.cpp]
- [ ] usability, import multiple selection [gui] [tab.cpp]
- [ ] usability, import directory selection [gui] [tab.cpp]
- [ ] A11y Accessibility status [a11y] [gui]
- [ ] HiDPI, font sizes [gui]
- [ ] RTL support, `LRM` text is not enough [rtl] [gui]
- [ ] Picons Editor, `QListWidget::IconMode` lazy load [gui] [piconsView.cpp]
- [ ] debug to stdout is not working on silicon [arm64] [macos]
- [x] neutrino cable, atsc, unknown support [e2db]
- [x] find match cache, is cached after reset, on debug [gui] [viewAbstract.cpp]
- [x] flatpak sandbox, capability to save directory with multiple files, `xdg-desktop-portal` [gui] [gui.cpp] [xdg] [flatpak]


**To Check:**

- [ ] console, one segfault, maybe `QWidget::deleteLater` [gui] [console_gui.cpp] [tools.cpp] [tab.cpp] 
- [ ] XML entities encoding, eg. satellite name [e2db]
- [ ] singular userbouquet import, in original sorting order, `parse_e2db_bouquets` [e2db]
- [ ] preferences, settings dialog, on `Enter` keybind, is not saving settings [gui] [settings.cpp]
- [ ] `onid == 0x0001` is `valid = (tsid > 0x0001)`, `valid = true` is False on STB [e2db]
- [ ] runtime exceptions, eg. use of `std::runtime_error` causes segfault on mingw32 [build]
- [ ] import from ftp, tools and demo, expending `import_blob` + `parse_e2db` [e2db]
- [x] neutrino terrestrial, wrong syntax, eg. `name="NaN"` `name="0.0E"` [e2db]
- [x] telnet reload, it connects but seems to not working [ftpcom]
- [x] telnet emulation py script, seems to not working [workspace] [start-telnet.py]
- [x] combobox defects, `QComboBox`, eg. sort menu, edit service [qt-wasm] [gui]
- [x] edit service on demo, collision dialog is not displayed, dialogs limitation [qt-wasm] [gui] [editService.cpp]
- [x] snapcraft cmake log, `libcurl4t64` conflict, `kde-neon-6` [build] [snapcraft]

