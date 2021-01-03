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

### Add new labels

Adding new labels to CPU-X is a little bit complex, but don't worry: this sub-section explains how to achieve that.  
This text is based on [this](https://github.com/X0rg/CPU-X/commit/ba60cbfc18952fc52b16a4c34a33f565493fb125#diff-8e11e336b0bbcfe85860ec612c712da5e03aebc0a755e6fcea1efa867c72b2f1) commit.  
:warning: Note: this guide does not explain how to add a new tab.

**1. In core**

- First of all, you need to add a new constant value in the appropriate enumerated type in `cpu-x.h` file; look for something starting with `enum EnTabTAB`. **Keep the `LASTTAB` value add the end.** In this example, `TAB` is the tab name and `NEWLABEL` is the constant value for the new label.
- Set the label name in `main.c`, inside the `labels_setname()` function:
  ```c
  asprintf(&data->tab_TAB[NAME][NEWLABEL], _("Label name"));
  ```
- Set the label value in `core.c`, in the appropriate function:
  ```c
  casprintf(&data->tab_TAB[VALUE][NEWLABEL], true or false, "%s", XXX);
  ```
  :bulb: `fill_labels()` and `do_refresh()` are the calling functions.

**2. In NCurses TUI**

You may need to adapt `tui_ncurses.c`. To print text on screen, `mvwprintwc()` and `mvwprintw2c()` functions are used.

If your label has a dynamic value (e.g. which change over time), also you need to adapt `nrefresh()` function.

**3. In GTK+ 3 GUI**

You need to use [Glade](https://glade.gnome.org/) to edit UI. The UI file in under `data/` (e.g. `cpu-x-gtk-3.12.ui`).

Two labels must be created at least:
- one label to display the label name, with an ID like `TAB_labNEWLABEL` ;
- a second label to display the label value, with an ID like `TAB_valNEWLABEL`.

On top of that, you need to declare these ID inside `gui_gtk_id.h`, in the appropriate array.  
:warning: Please respect the same order as the corresponding enumerated type in `cpu-x.h`.

Then labels are filled from `gui_gtk.c` file, by using `gtk_label_set_text()` function.  
You may need to adapt `get_widgets()`, `set_labels()` and `grefresh()` functions.

**4. Verify**

Build and run CPU-X. The software must not crash or freeze after these changes.

Please check all modes to avoid potential regressions:
```shell
$ cpu-x -Dv
$ cpu-x -Nv
$ cpu-x -Gv
```

If nothing is broken, congratulations! :tada: You can open a new [pull request](https://github.com/X0rg/CPU-X/compare).

### Things to do

- A good starting point if you want to do stuff is to fix [issues](https://github.com/X0rg/CPU-X/issues). One example: add proper CPU logos (as requested in #144).

- Another point, if you have a lot of time to spare, is to create new user interfaces:
    - Qt5 GUI can be appreciated by users using Qt-based desktop environnement (like KDE);
    - Cocoa GUI to port CPU-X on macOS (read #147, #56, #55 #35 and #33 for more details).

- Also, feel free to improve existing stuff. For instance, databases are defined in a C header, but maybe there are better ways to store data.
