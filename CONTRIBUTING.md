# How to contribute to CPU-X

CPU-X is mainly developed by @X0rg, but contributors are welcome!

Here is some things to be involved.

## Translate

If you want to translate CPU-X in your native tongue, please visit the [Weblate page](https://hosted.weblate.org/engage/cpu-x/?utm_source=widget).

## Update databases

When new CPUs are out, we need to add them in databases. You can find databases [here](src/databases.h). Also, please note CPU codenames are defined in [libcpuid](https://github.com/anrieff/libcpuid).

## Develop

CPU-X is written in C and uses the CMake build system.

First of all, if you are interested to modify CPU-X source code, take a look at [how to manually build CPU-X](https://github.com/X0rg/CPU-X/wiki/manual-build).

### Source files

The file structure in the `src` directory is the following:
1. CPU-X core, where data are gathered:
    - core.c
    - core.h
    - cpu-x.h
    - daemon.c
    - daemon.h
    - databases.h
    - ipc.h
    - main.c
    - util.c
2. External projects, modified to be integrated within CPU-X (optional):
    - bandwidth
    - dmidecode
3. User interfaces (optional):
     - gui_gtk.c
     - gui_gtk.h
    - gui_gtk_id.h
    - tui_ncurses.c
    - tui_ncurses.h

### Things to do

- A good starting point if you want to do stuff is to fix [issues](https://github.com/X0rg/CPU-X/issues).

- Another point, if you have a lot of time to spare, is to create new user interfaces:
    - Qt5 GUI can be appreciated by users using Qt-based desktop environnement (like KDE);
    - Cocoa GUI to port CPU-X on macOS (read #147, #56, #55 #35 and #33 for more details).

- Also, feel free to improve existing stuff. For instance, databases are defined in a C header, but maybe there are better ways to store data.
