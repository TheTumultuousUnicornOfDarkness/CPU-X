
[![](https://github.com/X0rg/CPU-X/blob/master/data/icons/CPU-X_22x22.png?raw=true)](https://x0rg.github.io/CPU-X/)
[![GitHub release](https://img.shields.io/github/release/X0rg/CPU-X.svg)](https://github.com/X0rg/CPU-X/tags)
[![GitHub downloads](https://img.shields.io/github/downloads/X0rg/CPU-X/latest/total.svg)](https://github.com/X0rg/CPU-X/releases/latest)
[![GitHub total downloads](https://img.shields.io/github/downloads/X0rg/CPU-X/total.svg)](https://github.com/X0rg/CPU-X/releases)
[![GitHub issues](https://img.shields.io/github/issues/X0rg/CPU-X.svg)](https://github.com/X0rg/CPU-X/issues)
[![Build Status](https://travis-ci.com/X0rg/CPU-X.svg?branch=master)](https://travis-ci.com/X0rg/CPU-X)
[![Translation status](https://hosted.weblate.org/widgets/cpu-x/-/svg-badge.svg)](https://hosted.weblate.org/engage/cpu-x/?utm_source=widget)

CPU-X is a Free software that gathers information on CPU, motherboard and more.  
CPU-X is similar to [CPU-Z (Windows)](https://www.cpuid.com/softwares/cpu-z.html), but CPU-X is a Free and Open Source software designed for GNU/Linux; also, it works on *BSD.  
This software is written in C and built with [CMake](https://www.cmake.org/) tool.  
It can be used in graphical mode by using GTK or in text-based mode by using NCurses. A dump mode is present from command line.  


***

# Table of contents
* [Dependencies](#dependencies)
  * [Build-only dependencies](#build-only-dependencies)
  * [Build and run dependencies](#build-and-run-dependencies)
* [Download/Install](#downloadinstall)
  * [Download packages](#download-packages)
  * [Manual build](#manual-build)
  * [Portable version](#portable-version)
* [Usage](#usage)
* [Screenshots](#screenshots)
* [Translate/Contributions](#translatecontributions)
* [Troubleshooting](#troubleshooting)
* [Bugs/Improvements/Request features](#bugsimprovementsrequest-features)
* [Links](#links)

***

## Dependencies

### Build-only dependencies

These dependencies are needed to **build** CPU-X:
* A C compiler ([GCC](https://gcc.gnu.org/) or [Clang](https://clang.llvm.org/))
* [CMake](https://www.cmake.org/)
* [Pkg-Config](https://www.freedesktop.org/wiki/Software/pkg-config/) / [Pkgconf](https://github.com/pkgconf/pkgconf)
* [NASM](http://www.nasm.us/)


### Build and run dependencies

These dependencies are needed to **build¹** and **run** CPU-X:
* [GTK3+](https://www.gtk.org/) (version 3.12 or newer is needed)  
* [NCurses](https://www.gnu.org/software/ncurses/)  
* [Libcpuid](http://libcpuid.sourceforge.net/) (version 0.3.0 or newer is needed)  
* [Pciutils](https://mj.ucw.cz/sw/pciutils/)  
* [Procps-ng](https://sourceforge.net/projects/procps-ng/) (Linux) / [Libstatgrab](https://www.i-scream.org/libstatgrab/) (*BSD)  
* [Curl](https://curl.haxx.se/)  
* [JSON-C](https://github.com/json-c/json-c)  
**¹**On some GNU/Linux distributions, the appropriate **-dev** or **-devel** package is needed.


## Download/Install

### Download packages

You can download binary packages to easily install CPU-X on your system. A lot of distributions are supported, see the [download section](https://github.com/X0rg/CPU-X/releases/latest) or the wiki page about [GNU/Linux packages](https://github.com/X0rg/CPU-X/wiki/GNU-Linux-Packages).  


### Manual build

For step-by-step guide, you can see this [wiki page](https://github.com/X0rg/CPU-X/wiki/Manual-build).  
If you need to disable some parts of CPU-X, you can read [this page](https://github.com/X0rg/CPU-X/wiki/Modular-components).

To build and install CPU-X on your system, do (in CPU-X directory) :
```
$ mkdir build && cd build
$ cmake ..
$ make
# make install
```
By default, CPU-X will be installed in */usr/local*. If you want to change it, add option `cmake -DCMAKE_INSTALL_PREFIX=<absolute_path> ..` on CMake invocation.


### Portable version

CPU-X is available in a portable version (Linux 32/64-bit, FreeBSD 32/64-bit), like CPU-Z.  
You can find the lastest release [**here**](https://github.com/X0rg/CPU-X/releases/latest).  
The CPU-X_vx.x.x_portable.tar.gz tarball requires GTK is installed on your system.  
The CPU-X_vx.x.x_portable_noGTK.tar.gz tarball requires to start CPU-X from a terminal.  

After downloading tarball, you need to extract his content to be able to run CPU-X portable. Check if binary has executable bit set.  
You can use this portable version on a lot of system, so you can leave a binary on a USB stick for instance.


## Usage

Start program with **root privileges** allows CPU-X to access some special devices, minimizing empty labels count.  
Application is put in the desktop menus, in **System Tools** category: entry *CPU-X* run CPU-X as regular user, and entry *CPU-X (Root)* grant root privileges.  
Else, you can use command `cpu-x`, or double-click on `cpu-x` binary is also possible (if program won't start, check if file has executable bit set).  
If GTK and NCurses are supported, you can start CPU-X in NCurses mode by taping in a shell (as root) `cpu-x --ncurses`.  
Use `cpu-x --help` for other commands and help.


## Screenshots

You can find screenshots in [gallery](https://github.com/X0rg/CPU-X/wiki/Screenshots).


## Translate/Contributions

Translation status:  
[![Translation status](https://hosted.weblate.org/widgets/cpu-x/-/multi-blue.svg)](https://hosted.weblate.org/engage/cpu-x/?utm_source=widget)  
Or you want to contribute to CPU-X? In the top-right corner of the page, click on the **Fork** button.


## Troubleshooting

Refer to the dedicated [FAQ](https://github.com/X0rg/CPU-X/wiki/FAQ) page.


## Bugs/Improvements/Request features

Please [open a new issue](https://github.com/X0rg/CPU-X/issues/new) and fill template. You can remove italic text.


## Links
Official [webpage](https://x0rg.github.io/CPU-X/) made by GitHub Pages.  
Official [wiki](https://github.com/X0rg/CPU-X/wiki), still on GitHub.
