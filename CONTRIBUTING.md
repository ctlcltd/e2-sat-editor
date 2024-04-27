# Contribution guidelines

Your contribution could be very useful to this project.

Info on how to do Pull Request are available on the page: [Creating a pull request](https://docs.github.com/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request) from GitHub Docs.

When the code you submit via Pull Request does not follow these contribution guidelines, your PR could be rejected or accepted. Submitted code will be modified to follow these guidelines.

&nbsp;

## Guidelines

There are a few things you need to know in order to contribute in the right way.

This project is free software. Each contribution must comply with the licensing conditions. All informations on where to find the license terms in the [README.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/README.md).

- Any improvements to the already written code are appreciated.

- Things to do and improve are marked in the code with **//TODO**, **//TODO FIX**, **//TODO improve**.

- Things to be tested are marked in the code with **//TODO TEST**.

- Keep in mind cross-platform compatibility: Windows, macOS, Linux, Unix-like BSD, Qt for WebAssembly.

- Keep in mind compilers compatibility: **GNU GCC**, **Clang**, **MinGW-w64**.

- Keep in mind CPU architecture compatibility: amd64 aarch64 arm64 64-bit, i686 arm32 32-bit.

- Keep in mind backward compatibility with Qt 5 and compatibility with Qt for WebAssembly (Emscripten), for Demo mode.

- Use **standard C++** functions with well-defined behaviors, maintain some backward compatibility.

- Dependencies are **Qt** and **libcurl4**. Do not use other dependencies when not strictly necessary.

- Do not extend classes from *QObject* or *QWidget* when not strictly necessary.

- You can reuse "unwritten conventions" as much as possible, look at already written code. Use the same conventions that are already in use for property names, function names, class names.

- Remember to keep key components isolated from *gui*. These components have their counterpart for *gui*. For example, *e2se::e2db* and *e2se::ftpccom* have their counterparts *e2se_gui::e2db* and *e2se_gui::ftpcom* inside *gui*. These components will be usable as dependency and also from the command line, they will not depends on the GUI toolkit.

- Do not use product names or registered trademarks for property names, function names, class names. This is strongly discouraged.

- Do not create new files, new directories, new classes, when it is not necessary.

- Do not introduce in the code: undefined behaviors, useless complexity, return values or behaviors that differs on compilers and platforms.

- Submitted code must be your own, please do not steal code from other projects or take code snippets.

- Keep in mind intuitiveness, strenghtness, performance.

If you want be added to the contributors list, in the *e2 SAT Editor team* list, let us know at the time of the Pull Request or at a later time from comments.


### Roadmap

For the roadmap, please refer to [ROADMAP.md](https://github.com/ctlcltd/e2-sat-editor/blob/main/ROADMAP.md) and topics on [Discussions](https://github.com/ctlcltd/e2-sat-editor/discussions).


### Code of Conduct

For the code of conduct, you can refer to the [Code of Conduct](https://docs.github.com/site-policy/github-terms/github-community-code-of-conduct) from the GitHub community.

