# Roadmap

The current project Roadmap is concerned in enhancements. To discuss the Roadmap, see [Roadmap](https://github.com/ctlcltd/e2-sat-editor/discussions/13) discussion.

For suggestions of any kind, please open a [Feature request](https://github.com/ctlcltd/e2-sat-editor/issues/new?template=feature_request.yml).

There are several points To Do and To Evaluate.

**To Do:**

- [ ] userbouquet replacement, import file [e2db] [mainView.cpp]
- [ ] service from reference replacement [e2db] [mainView.cpp]
- [ ] tiny UI, item sizes contracted, icons have priority [gui]
- [ ] get transponders from list transform [tools.cpp] [e2db]
- [ ] export userbouquets selection [e2db] [mainView.cpp]
- [ ] tree view mode, classic view with three tree side by side [gui]
- [ ] tools duplicates, check for duplicate tunersets [tools.cpp] [e2db]
- [ ] log inspector text filter, with regular expression, eg. `/e2db/` [gui]
- [ ] multi-window GUI [gui] [gui.cpp] [tab.cpp]
- [x] console, similar to a browser inspector [gui] [e2db_console.cpp] [termctl_gui.cpp] [console_gui.cpp]
- [x] improve management of duplicate channels [gui] [mainView.cpp] [editService.cpp]
- [x] mirror transponders lamedb from|to xml [gui] [e2db] [tools.cpp]
- [x] calculate dvbns namespace [gui] [e2db] [editService.cpp] [tools.cpp]
- [x] favourite support, `stream` reference type [gui] [e2db]
- [x] connection profiles import and export [gui] [settings.cpp]

**To Evaluate:**

- [ ] merge options on import, auto-merge tristate button [gui] [tab.cpp] [e2db]
- [ ] custom channel number as LCN Logical Channel Number [gui] [e2db]
- [ ] provide reference custom data [e2db]
- [ ] improve m3u support, import and export [gui] [e2db]
- [ ] wasm, capability to open and save the directory with multiple files [qt-wasm] [gui] [gui.cpp]
- [x] (external batch command) picon image transformation, eg. resize, mask [gui] [piconsView.cpp]


## Smart Userbouquet

Smart Userbouquets are userbouquets created on the fly, through filtering. Smart Userbouquets are like the font collections created with metadata.

It will be possible to create many smart userbouquets: Userbouquet from provider name or frequencies, or both.

Provide a visual feedback, distinguishing concrete userbouquet from smart userbouquet, for example: "Concrete Userbouquet" *"Smart Userbouquet \*"*


## Automated Build

The software is built at [GitHub Actions](https://github.com/ctlcltd/e2-sat-editor/actions).

See [Automated Build](https://github.com/ctlcltd/e2-sat-editor/discussions/17) discussion.


## Console

✅ Done, in version 2.0.0

Console will look similar to a browser inspector. Scripting will be achivied exposing the `e2db_cli` API with a `gui` counterpart.


## Add-ons

Add extension support using interpreted languages. It would be very nice to create add-ons in multiple languages: JavaScript, Python.

Keep focused the impact on weight and dependencies.

