# Contribution guidelines

Your contribution is very useful for this project, but some rules are needed to keep it well.

Info on how to do Pull Request are available on the page [Creating a pull request](https://docs.github.com/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request) from the GitHub help.

When the code you submit via Pull Request does not follow these contribution guidelines, your PR will be rejected and you will be required to follow these guidelines, or your PR will be accepted and then the submitted code will be modified to follow these.


&nbsp;
## Before contributing

There are a few things you need to know in order to contribute in the right way.

This project is free software. Each contribution must comply with the licensing conditions. All information on where to find the license terms in the [README.md](README.md).

- Any improvements to the already written code are appreciated. Things to do and improve are marked in the code with **//TODO**, **//TODO FIX**, **//TODO improve**.

- You can follow the "unwritten rules", look at the already written code. For example, consecutive conditional statements must have brackets, within the statement unbracketed conditional statements are allowed, in a single line, but only if they are consecutive.

- Cross-compatibility for these platforms: Linux, Macos, Windows; and these compilers: **gnu-g++**, **macos-clang**, **mingw**.

- Use only **standard C++** functions with well-defined behaviors, and maintain some compatibility with C++11. This is to maintain some backwards compatibility with other devices. Using C++17 is allowed, but do not abuse it. Keep in mind the cross-platform compatibility.

- The only dependencies are **Qt** and **libcurl**. Do not use dependencies when not strictly necessary, use only standard C++ functions. For example, libboost will not become a dependency of this software.

- Do not consider Qt strictly necessary but a useful thing, to not strictly bind to Qt and to make porting easier. Do not extend *gui* classes from classes derived from *QObject* and *QWidget*.

- The C++ code in this project is written with indentation and **4-width tabs**. For output to other formats within the software, look at the already written code.

- Do not introduce in the code: unwanted behaviors, useless complexity, undefined behavior, return values or behaviors of functions that are different between platforms, memory leaks, conflicts, malfunctions.

- Use the principle of intuitiveness, it is not necessary to comment on every single thing if its behavior is easily understood.

- You can reuse "unwritten conventions" as much as possible, look at already written code. Use the same names that are already used for properties, function names, class names, attribute names. This depends on the context. Even the same case style used for the names. The use of product names or registered trademarks for property names, functions, classes, attributes, is strongly discouraged.

- Remember to keep core components isolated from *gui*. These components have their counterpart for *gui* integration. For example, *e2se::e2db* and *e2se::ftpccom* have their counterparts *e2se_gui::e2db* and *e2se_gui::ftpcom* inside *gui*. These components will be usable as dependencies and also from the command line, they will not use the toolkit. Qt should absolutely not be used within core components.

- Do not force things from the beginning into the basement. For example, it is a stretch to create a foundation class for everything when each derived class will have different behavior. Another example, *e2se_gui::printable* generates HTML 4.0 Strict for printing with Qt functions, HTML and CSS support is limited. However *e2se_e2db::e2db_converter* does not use Qt, it generates more current, generic and non-printable HTML and CSS.

- Do not create new files, new classes, new folders, when it is not strictly necessary. Keep the pre-existing folder structure intact.

- It may seem superfluous. Submitted code must be your own, you must not steal code from other projects or snippets. And keep in mind performance.

Your contributions to this project will be signed, your GitHub username will be added to the contributors list, in the *e2 SAT Editor team*. If you prefer otherwise, let us know at the time of the Pull Request or at a later time.


### Roadmap

This project has not reached the stable stage. For a roadmap refer to the TODO list in the [README.md](README.md). Things like: cli, translations, plug-in system, etc., are not defined yet. If you want to collaborate on these not yet defined things, we can discuss them in [Discussions](https://github.com/ctlcltd/e2-sat-editor/discussions).


### Code of Conduct

For the code of conduct, you can refer to the [Code of Conduct](https://docs.github.com/site-policy/github-terms/github-community-code-of-conduct) of the GitHub community.

Use common sense.
