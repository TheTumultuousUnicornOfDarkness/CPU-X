# How to contribute to CPU-X

CPU-X is mainly developed by [TheTumultuousUnicornOfDarkness](https://github.com/TheTumultuousUnicornOfDarkness), but contributors are welcome!

Here is some things to be involved.

## Translate

If you want to translate CPU-X in your native tongue, please visit the [Weblate page](https://hosted.weblate.org/engage/cpu-x/?utm_source=widget).

## Update databases

When new CPUs are out, we need to add them in databases. You can find databases [here](src/core/databases.h). Also, please note CPU codenames are defined in [libcpuid](https://github.com/anrieff/libcpuid).  
Dumps can be found on [InstLatx64 website](http://users.atw.hu/instlatx64/). Latest dumps can be found on [this Git mirror](https://github.com/InstLatx64/InstLatx64/commits/master).

## Develop

CPU-X is written in C++ and uses the CMake build system.

First of all, if you are interested to modify CPU-X source code, take a look at [how to manually build CPU-X](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/wiki/manual-build).

### Source files

The file structure in the `src` directory is the following:
1. CPU-X core, where data are gathered:
```
src
└── core
    ├── benchmarks.cpp
    ├── core.cpp
    ├── core.hpp
    ├── databases.h
    ├── internal.hpp
    ├── libcpuid.cpp
    ├── libopencl.cpp
    ├── libopengl.cpp
    ├── libpci.cpp
    ├── libsystem.cpp
    ├── libvulkan.cpp
    └── opencl_ext.h
```
2. External projects, modified to be integrated within CPU-X (optional):
```
src
└── core
    ├── bandwidth
    └── dmidecode
```
3. CPU-X daemon, doing operations that require privileges:
```
src
└── daemon
    ├── client.cpp
    ├── client.hpp
    ├── daemon.h
    ├── server.cpp
    └── server.hpp
```
4. User interfaces (optional):
```
src
└── ui
    ├── gtk.cpp
    ├── gtk.hpp
    ├── ncurses.cpp
    └── ncurses.hpp
```

### Add new labels

Adding new labels to CPU-X is a little bit complex, but don't worry: this sub-section explains how to achieve that.  
> [!NOTE]
> This guide does not explain how to add a new tab.

**1. In core**

- First of all, you need to add a new `Label` object in `struct Data` in `data.hpp` file. Example to add `kernel` in `Data::System::OperatingSystem`:
  ```cpp
  Label kernel {_("Kernel")};
  ```
  `{_("Kernel")}` is the name of the label (which may be translated).
- Set the label value in `core.cpp`, in the appropriate function. You need to store label value in `value` field, like this:
  ```c
  data.system.os.kernel.value = "XXX";
  ```
> [!TIP]
> `fill_labels()` and `do_refresh()` are the calling functions.
- Finally, in `data.cpp` file, print the label in the appropriate `operator<<` overload.

**2. In NCurses TUI**

You may need to adapt `tui_ncurses.cpp`. To print text on screen, `mvwprintwc()` and `mvwprintw2c()` functions are used.
```c
mvwprintw2c(win, LINE_1, SizeInfo::tb, "%13s", "%s", data.system.os.kernel);
```
In this example, it will display content of `data.system.os.kernel` at `LINE_1`, with label name on the left side.

**3. In GTK+ 3 GUI**

You need to use [Glade](https://glade.gnome.org/) to edit UI. The UI file in under `data/` (e.g. `cpu-x-gtk-3.12.ui`).

Two labels must be created at least:
- one label to display the label name, with an ID like `TAB_labNEWLABEL` ;
- a second label to display the label value, with an ID like `TAB_valNEWLABEL`.

Then labels are filled from `gui_gtk.cpp` file:
- bind widget (from UI) to an object in `get_widgets()` ;
- set label by using `set_label_name_and_value()` in the appropriate `GtkData::gtab_XXX()` method.

**4. Verify**

Build and run CPU-X. The software must not crash or freeze after these changes.

Please check all modes to avoid potential regressions:
```shell
$ cpu-x -Dv
$ cpu-x -Nv
$ cpu-x -Gv
```

If nothing is broken, congratulations! :tada: You can open a new [pull request](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/compare).

### Add new options

This section describe how to add a new option. You can take a look on [this commit](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/commit/008e03cf95653e964a9e334347c297fef57ce82a).

1. `src/options.hpp`: add new option in `class Options`. Values are private: you need to define a public setter and getter.
2. `src/main.cpp`:
   * in `cpux_options` list: add a new line entry
   * in `parse_arguments()`: retrieve value for option
3. `data/io.github.thetumultuousunicornofdarkness.cpu-x.gschema.xml`: add a new key
4. `data/cpu-x-gtk-3.12.ui`: open UI file in Glade and change Settings window
5. `src/ui/gtk.hpp`: add a new widget in `struct GtkData`
6. `src/ui/gtk.cpp`:
   * `get_widgets()`: map UI widget to a C++ object
   * `load_settings()`: map setting to an option
   * `bind_settings()`: bind schema value to a C++ object
   * `set_signals()`: handle signal when value is changed by user (note: the "OK" button in Settings window is handled by `this->validatebutton->signal_clicked().connect()`)
7. Do something with your new option!

### Things to do

- A good starting point if you want to do stuff is to fix [issues](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/issues). One example: add proper CPU logos (as requested in #144).

- Another point, if you have a lot of time to spare, is to create new user interfaces:
    - Qt5/Qt6 GUI can be appreciated by users using Qt-based desktop environnement (like KDE);
    - Cocoa GUI to port CPU-X on macOS (read #147, #56, #55 #35 and #33 for more details).

- Also, feel free to improve existing stuff. For instance, databases are defined in a C header, but maybe there are better ways to store data.
