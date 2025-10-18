
# BUGS

The following lists of known bugs are categorized by: Fix, Improve, Evaluate, Check; and ordered by priority.

Information on contributions, to contribute and send Pull Request, please see [CONTRIBUTING.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/CONTRIBUTING.md) file.


**To Fix:**

- [ ] ftp upload, the current tree list is not stored before upload [tab.cpp] [viewAbstract.cpp] [mainView.cpp]
- [ ] favourite reference, on copy-paste `#DESCRIPTION` is everywhere [mainView.cpp]
- [ ] edit transponders, sort is not stored after sort [transpondersView.cpp]
- [ ] telnet reload, when connected is always True also when not reloading [ftp] [ftpcom.cpp]
- [ ] telnet reload, it connects but seems to not working [ftp] [ftpcom.cpp]
- [ ] tools popover defects, native `QMenu` `QComboBox` [macos] [gui] [tools.cpp]
- [ ] find match cache, cached after channel deletetion [gui] [viewAbstract.cpp]
- [ ] telnet emulation py script, seems to not working [workspace] [start-telnet.py]
- [ ] check update, when disconnected, there is no connection error dialog [checkUpdate.cpp]
- [ ] check update, when disconnected, version string `%1` is omitted [checkUpdate.cpp]
- [ ] html bouquets, markup errors `<tr>` [printable.cpp]
- [ ] cli on terminal emulation, PgUp and PgDown inversion [linux] [cli] [e2db_termctl.cpp]


**To Improve:**

- [ ] service flags and compatibility, `dvbfrontend` [gui] [e2db.cpp]
- [ ] reference flags and compatibility, `dvbfrontend` [gui] [e2db.cpp]
- [ ] ftp error messages, from "Access denied to remote resource." to "No such file or directory." [ftp] [fptcom.cpp]
- [ ] counters accuracy for items, Total, TV, Radio, Data [gui] [tab.cpp]
- [ ] service and reference flags in the GUI, eg. Issues (#33) [gui]
- [ ] items move and copy, from and to, drag and drop not enough [gui] [cli]
- [ ] convert lamedbx from and to zapitx, better visual feedback [gui] [tab.cpp]
- [ ] transponder orbital position column, sorting order A-Z [gui] [viewAbstract.cpp]
- [ ] "userbouquet" translated as "floral bouquets" [l10n]
- [ ] neutrino feparams and versions, unknown compatibility [gui] [e2db.cpp]
- [ ] libcurl cabundle dependency [networking]
- [ ] snapcraft and xdg icons [snapcraft]


**To Evaluate:**

- [ ] find match cache, cached after reset in debug [gui] [viewAbstract.cpp]
- [ ] ftp, `resetStatusBar` is called twice [gui] [tab.cpp]
- [ ] reference, merge markers with same name [gui] [mainView.cpp]
- [ ] usability, import multiple selection [gui] [tab.cpp]
- [ ] usability, import directory selection [gui] [tab.cpp]
- [ ] runtime exceptions, eg. use of `std::runtime_error` cause segfault on mingw32 [code]
- [ ] force updates from TSV, CSV, eg. rename channels, change parameters [gui] [mainView.cpp]
- [ ] status bar closed, visual feedback `InfoMessage` downloaded files, no double dialog, errors and infos [gui] [tab.cpp]
- [ ] reload option, webif and|or telnet [gui] [tab.cpp] [settings.cpp] [ftpcom.cpp]
- [ ] reload option, deactivate reload, manually [gui] [tab.cpp] [settings.cpp] [ftpcom.cpp]
- [ ] parental userbouquet `FROM BOUQUET`, unknown compatibility, visual feedback icon [gui] [e2db.cpp]
- [ ] lamedb5 comments [e2db.cpp]
- [ ] neutrino cable, atsc [e2db.cpp]
- [ ] FTPS, HTTPS support [networking] [gui] [settings.cpp] [fptcom.cpp]
- [ ] find toolbar, look and feel [windows] [linux] [gui]
- [ ] picons editor, `QListWidget::IconMode` lazy load [gui] [piconsView.cpp]
- [ ] HiDPI, font sizes [gui]
- [ ] RTL support, `LRM` text not enough [rtl]
- [ ] accessibility status [gui]
- [ ] edit service on demo, collision dialog is not displayed, dialog cohexist limit [qt-wasm] [gui] [editService.cpp]
- [ ] debug to stdout is not working on silicon [arm64] [macos]
- [ ] ftp debug, bad stdout output [ftp]
- [ ] snapcraft cmake log, `libcurl4t64` conflict `kde-neon-6` [build] [snapcraft]


**To Check:**

- [ ] flatpak sandbox, capability to save the directory with multiple files, `xdg-desktop-portal` [gui] [gui.cpp] [flatpak]
- [ ] select defects, `QComboBox`, eg. sort menu, edit service [qt-wasm] [gui]
- [ ] settings dialog, save keybind Enter is not saving [gui] [settings.cpp]
- [ ] ftp download, counters issue [gui] [tab.cpp]
- [ ] singular userbouquet import, original sorting order, `parse_e2db_bouquets` [e2db.cpp]
- [ ] neutrino terrestrial, eg. `name="NaN"` `name="0.0E"` [e2db.cpp]
- [ ] `onid == 0x0001` is `valid = (tsid > 0x0001)`, `valid = true` is False on STB [e2db.cpp]
- [ ] edit service from Channel Book [gui] [channelBookView.cpp]
- [ ] import from ftp, tools and demo, expending `import_blob` + `parse_e2db` [e2db.cpp]

