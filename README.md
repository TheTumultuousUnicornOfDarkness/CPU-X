->![CPU-X](https://raw.githubusercontent.com/X0rg/CPU-X/master/data/pictures/CPU-X.png)

[![GitHub release](https://img.shields.io/github/release/X0rg/CPU-X.svg)](https://github.com/X0rg/CPU-X)
[![GitHub downloads](https://img.shields.io/github/downloads/X0rg/CPU-X/latest/total.svg)](https://github.com/X0rg/CPU-X)
[![GitHub issues](https://img.shields.io/github/issues/X0rg/CPU-X.svg)](https://github.com/X0rg/CPU-X/issues)<-

CPU-X is a Free software that gathers information on CPU, motherboard and more.  
CPU-X is similar to [CPU-Z (Windows)](http://www.cpuid.com/softwares/cpu-z.html), but CPU-X is designed for GNU/Linux; it can also works on *BSD and on OS X (some features are missing).  
This software is written in C programming language, and built with [CMake](http://www.cmake.org/).  
This is a graphical software (GTK is used), but it can be used from a shell by using Ncurses or by dumping data.

->![](http://i.imgur.com/oCqilgK.png)<-

***

# Table of contents
* [Dependencies](#dependencies)
* [Download/Install](#downloadinstall)
  * [Download packages](#download-packages)
  * [Manual build](#manual-build)
  * [Portable version](#portable-version)
* [Usage](#usage)
* [Screenshots](#screenshots)
* [Translate/Contributions](#translatecontributions)
* [Troubleshooting](#troubleshooting)
* [Bugs/Improvements/Request features](#bugsimprovementsrequest-features)

***

## Dependencies

* GTK3+ (version 3.8 or newer, latest is recommended)
    * Graphical User Interface (GUI)
    * http://www.gtk.org/
    * Required to run CPU-X in GTK mode; **not** embedded in portable version (use system shared library)

* Ncurses
    * Text-based User Interface (TUI)
    * http://www.gnu.org/software/ncurses/
    * Required to run CPU-X in NCurses mode; embedded in portable version

* Libcpuid
    * Extract CPU features
    * http://libcpuid.sourceforge.net/
    * Required to run CPU-X correctly; embedded in portable version

* Libpci
    * Extract others hardware features
    * http://mj.ucw.cz/sw/pciutils/
    * Required to run CPU-X correctly; **not** embedded in portable version (use system shared library)

* Procps-ng, provides *libsystem* (**Linux only**)
    * Useful informations on system
    * http://sourceforge.net/projects/procps-ng/
    * Required to run CPU-X correctly; embedded in portable version

* Libstatgrab, provides *libsystem* (replaces **Procps-ng**)
    * Useful informations on system
    * http://www.i-scream.org/libstatgrab/
    * Required to run CPU-X correctly; embedded in portable version


## Download/Install

### Download packages

You can download binary packages to easily install CPU-X on your system. A lot of distributions are supported, click on your distro logo and follow instruction:  
[![Arch Linux](http://i.stack.imgur.com/ymaLV.png)](https://github.com/X0rg/CPU-X/wiki/%5BPackage%5D-Arch-Linux)
[![Debian](https://phrozensoft.com/uploads/2014/11/debian_logo.png)](https://github.com/X0rg/CPU-X/wiki/%5BPackage%5D-Debian)
[![Fedora](http://kivy.org/images/os_fedora.png)](https://github.com/X0rg/CPU-X/wiki/%5BPackage%5D-Fedora)
[![Gentoo](http://www.ltsp.org/images/distro/gentoo.png)](http://gpo.zugaina.org/sys-apps/cpu-x)
[![Ubuntu](http://computriks.com/img/icon/ubuntu24.png)](https://github.com/X0rg/CPU-X/wiki/%5BPackage%5D-Ubuntu)



### Manual build

For step-by-step guide, you can see the [wiki page](https://github.com/X0rg/CPU-X/wiki) (GNU/Linux, *BSD and OS X).  
First of all, you need to install [CMake](http://www.cmake.org/) (this is only needed for build).
GTK headers are needed to compile with GTK support.  
You can disable components in CPU-X before build by passing argument `-D<var>=0` when running CMake:  
`-DWITH_GTK=0` will disable support of GUI in GTK3+  
`-DWITH_NCURSES=0` will disable support of NCurses mode  
`-DWITH_LIBCPUID=0` will avoid calls to Libcpuid (not recommended)  
`-DWITH_LIBDMI=0` will not compile Libdmi and will avoid calls to Libdmi (not recommended)  
`-DWITH_LIBPCI=0` will avoid calls to Libpci (not recommended)  
`-DWITH_LIBSYSTEM=0` will avoid calls to Libprocps/Libstatgrab (not recommended)  

* If you want to install CPU-X on your system, do:
```
mkdir build && cd build
cmake ..
make
make install
```
By default, CPU-X will be installed in */usr/local*. If you want to change it, add option `cmake -DCMAKE_INSTALL_PREFIX=<absolute_path> ..` on CMake invocation.

* If you want to build a portable binary, do:
```
mkdir ebuild && cd ebuild
cmake -DEMBED=1 ..
make
```

Note: portable binary is *accomplished/bin/cpu-x*, in directory *ebuild*. Don't do `make install` after.You can move this runnable file where you want.


### Portable version

CPU-X is available in a portable version (Linux 32/64-bit, FreeBSD 32-bit), like CPU-Z.  
Latest release is [HERE](https://github.com/X0rg/CPU-X/releases/latest), all releases are [here](https://github.com/X0rg/CPU-X/releases).  
Download file *CPU-X_vX.X.X_portable.tar.gz* or file *CPU-X_vX.X.X_portable_noGTK.tar.gz*, extract archive, and you can now run CPU-X portable.  
You can put these files on a USB stick for example.


## Usage

Start program with **root privileges** allow to use **Dmidecode** (Libdmi) and avoid empty labels.
Application is put in the desktop menus, in **System Tools** category: entry *CPU-X* run CPU-X as regular user, and entry *CPU-X (Root)* grant root privileges.  
Else, you can use command `cpu-x`, or double-click on `cpu-x` binary is also possible (if program won't start, check if file has executable bit set).  
If GTK and NCurses are supported, you can start CPU-X in NCurses mode by taping in a shell (as root) `cpu-x --ncurses`.  
Use `cpu-x --help` for other commands and help.


## Screenshots

You can see how CPU-X looks here:
https://github.com/X0rg/CPU-X/wiki/Screenshots


## Translate/Contributions

You want to have CPU-X in a foreign language but no translation exists? See the following wiki page:
https://github.com/X0rg/CPU-X/wiki/Translate  
You want to contribute to CPU-X? In the top-right corner of the page, click **Fork**.


## Troubleshooting

* **CPU-X won't start**: try to do a `chmod +x` on binary.
* **CPU-X still won't start**: run it from a shell, and look output.
* **Some labels are empty**: verify if CPU-X is run with root privileges. If it is the case, your hardware is not recognised by a library.


## Bugs/Improvements/Request features

Please [open a new issue](https://github.com/X0rg/CPU-X/issues/new).
