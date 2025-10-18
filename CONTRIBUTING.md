# Contribution guidelines

Your contribution could be very useful to this project. There are a few things you need to know to contribute.

For a list of known Bugs and the Roadmap, please refer to [BUGS.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/BUGS.md) and [ROADMAP.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/ROADMAP.md), see also the topics on [Discussions](https://github.com/ctlcltd/e2-sat-editor/discussions).

> [!NOTE]
> *The software is free software*
> Each contribution must comply with the licensing conditions
> All informations on where to find the license terms in the [README.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/README.md#license) file


## Issues and Feature requests

When you find an issue, fill a report from [Issues](https://github.com/ctlcltd/e2-sat-editor/issues) providing many details as possible.

To ask for new features, use [Feature request](https://github.com/ctlcltd/e2-sat-editor/issues/new?template=feature_request.yml).


## Pull Request

You can send your contibution via [Pull Request](https://github.com/ctlcltd/e2-sat-editor/pulls).

If your code does not follow the Contribution guidelines, your PR could be rejected or revised. Submitted code will be modified to follow the Contribution guidelines.

Informations on How To do Pull Request on the page: [Creating a Pull Request](https://docs.github.com/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request) from GitHub Documentation.


## Development

Before start, you need to install build tools and Qt: https://www.qt.io/download

You might need Curl installed with headers, details here: https://curl.se/download.html

> [!NOTE]
> Depending on your OS environment, you should install dependencies

Clone the repository:
```
git clone https://github.com/ctlcltd/e2-sat-editor.git
```

> [!TIP]
> Instructions on how to build in [BUILD.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/BUILD.md) file


### To Do

There is a list of known [BUGS.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/BUGS.md) and the [ROADMAP.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/ROADMAP.md). Other things are commented `//TODO` in the code.

Any improvements to the already written code will be appreciated.

> [!TIP]
> Keep in mind intuitiveness, strenghtness and performance


### Coding Conventions

Look the written code and use the same conventions that are already in use, for example: property names, statement names, ruler, indentation.

Use *standard C++17* functions. See informations about [Compatibility](https://github.com/ctlcltd/e2-sat-editor/blob/main/CONTRIBUTING.md#compatibility).

Main components (*namespace*) are isolated from the `gui`, they have counterparts. For example, `e2se::e2db` and `e2se::ftpccom` have their counterparts `e2se_gui::e2db` and `e2se_gui::ftpcom` respectively. Main components will be usable as dependency and from `cli`, they will not depends on the `gui`.


### Compatibility

Cross-platform compatibility: `Windows`, `macOS`, `Linux`, `Unix-like BSD`, `WebAssembly` (Emscripten).

Compilers compatibility: `GNU GCC`, `Clang`, `MinGW-w64`.

Software Demo mode compatibility: `Qt for WebAssembly`.

Software backward compatibility: `Qt 5`.


### Do Not

Do not create new files, new directories, new classes, if not necessary.

Do not introduce in the code: undefined behaviors, useless complexity, return values or results that differs between compilers and platforms.

Do not use other dependencies if not strictly necessary. The main dependencies are: `Qt 6`, `libcurl4`.

Do not use C++ dialects or preprocessors, do not extend classes from `QObject` or `QWidget`, if not strictly necessary.

Do not steal code from other projects or take code snippets, submitted code must be your own.

Do not use product names or registered trademarks in property names, statement names, file names. This is strongly discouraged.


### Attribution

If you want be added to the contributors list, in the *e2 SAT Editor team*, let us know at the time of the Pull Request or at a later time from comments.

