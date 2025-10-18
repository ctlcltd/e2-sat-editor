# Roadmap

The current project Roadmap is concerned in enhancements. To discuss the Roadmap, see [Roadmap](https://github.com/ctlcltd/e2-sat-editor/discussions/13) discussion.

For suggestions of any kind, please open a [Feature request](https://github.com/ctlcltd/e2-sat-editor/issues/new?template=feature_request.yml).

There are several points To Do and To Evaluate.

**To Do:**

- [ ] userbouquet replacement, import file [e2db.cpp] [mainView.cpp]
- [ ] service from reference replacement [e2db.cpp] [mainView.cpp]
- [ ] tiny UI, item sizes contracted, icons have priority [gui]
- [ ] get transponders from list transform [tools.cpp] [e2db.cpp]
- [ ] export userbouquets selection [e2db.cpp] [mainView.cpp]
- [ ] tree view mode, classic view with three tree side by side [gui]
- [ ] tools duplicates, check for duplicate tunersets [tools.cpp] [e2db.cpp]
- [ ] log inspector text filter, with regular expression, eg. `/e2db/` [gui]
- [ ] multi-windowed GUI [gui] [gui.cpp] [tab.cpp]
- [x] improve management of duplicate channels [gui] [mainView.cpp] [editService.cpp]
- [x] mirror transponders Lamedb|XML [gui] [e2db.cpp] [tools.cpp]
- [x] calculate dvbns namespace [gui] [e2db.cpp] [editService.cpp] [tools.cpp]
- [x] favourite support, `stream` reference type [gui] [e2db.cpp]
- [x] connection profiles import and export [gui] [settings.cpp]

**To Evaluate:**

- [ ] merge options on import [gui] [tab.cpp] [e2db.cpp]
- [ ] custom channel number as LCN (Logical Channel Number) [gui] [e2db.cpp]
- [ ] provide reference custom data [e2db.cpp]
- [ ] picon image transformation, eg. resize, mask [gui] [piconsView.cpp]
- [ ] improve m3u support, import and export [gui] [e2db.cpp]
- [ ] wasm, capability to open and save the directory with multiple files [qt-wasm] [gui] [gui.cpp]


## Smart Userbouquet

Smart Userbouquets are userbouquets created on the fly, through filtering. Smart Userbouquets are like the font collections created with metadata.

It will be possible to create many smart userbouquets: Userbouquet from provider name or frequencies, or both.

Provide a visual feedback, distinguishing concrete userbouquet from smart userbouquet, for example: "Concrete Userbouquet" *"Smart Userbouquet \*"*


## Automated Build

The software is built at [GitHub Actions](https://github.com/ctlcltd/e2-sat-editor/actions).

See [Automated Build](https://github.com/ctlcltd/e2-sat-editor/discussions/17) discussion.


## Console

Console will look similar to a browser inspector. Scripting will be achivied exposing the `e2db_cli` API with a `gui` counterpart.


## Add-ons

Add extension support using interpreted languages. It would be very nice to create add-ons in multiple languages: JavaScript, Python.

Keep focused the impact on weight and dependencies.

