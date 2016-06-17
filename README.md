
[![GitHub release](https://img.shields.io/github/release/X0rg/CPU-X.svg)](https://github.com/X0rg/CPU-X)
[![GitHub downloads](https://img.shields.io/github/downloads/X0rg/CPU-X/latest/total.svg)](https://github.com/X0rg/CPU-X)
[![GitHub issues](https://img.shields.io/github/issues/X0rg/CPU-X.svg)](https://github.com/X0rg/CPU-X/issues)

CPU-X is a Free software that gathers information on CPU, motherboard and more.  
CPU-X is similar to [CPU-Z (Windows)](http://www.cpuid.com/softwares/cpu-z.html), but CPU-X is a Free and Open Source software designed for GNU/Linux; also, it works on *BSD.  
This software is written in C and built with [CMake](http://www.cmake.org/) tool.  
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

These dependencies are needed to **manually build** CPU-X (e.g you can safely remove them after build):
* A C compiler, like [GCC](https://gcc.gnu.org/) or [Clang](http://clang.llvm.org/)
* [CMake](http://www.cmake.org/)
* [Pkg-Config](http://www.freedesktop.org/wiki/Software/pkg-config/) / [Pkgconf](https://github.com/pkgconf/pkgconf)
* [NASM](http://www.nasm.us/)


### Build and run dependencies

These dependencies are needed to **manually build** and **run** CPU-X (e.g you can't remove a dependency if CPU-X was built with):
* [GTK3+](http://www.gtk.org/) (minimum supported version is 3.8, version 3.16 or newer recommended)  
* [NCurses](http://www.gnu.org/software/ncurses/)  
* [Libcpuid](http://libcpuid.sourceforge.net/) (version 0.2.2 or newer is recommended)  
* [Libpci](http://mj.ucw.cz/sw/pciutils/)  
* [Procps-ng](http://sourceforge.net/projects/procps-ng/) (Linux) / [Libstatgrab](http://www.i-scream.org/libstatgrab/) (*BSD)  
* [Curl](http://curl.haxx.se/) / [Wget](https://www.gnu.org/software/wget/) (optionnal)


## Download/Install

### Download packages

You can download binary packages to easily install CPU-X on your system. A lot of distributions are supported, click on your distro logo:
[![Arch Linux](http://i.stack.imgur.com/ymaLV.png)](https://github.com/X0rg/CPU-X/releases/download/v3.0.0/CPU-X_v3.0.0_ArchLinux.tar.gz)
[![Debian](https://phrozensoft.com/uploads/2014/11/debian_logo.png)](https://github.com/X0rg/CPU-X/releases/download/v3.0.0/CPU-X_v3.0.0_Debian.tar.gz)
[![Fedora](http://kivy.org/images/os_fedora.png)](https://github.com/X0rg/CPU-X/releases/download/v3.0.0/CPU-X_v3.0.0_Fedora.tar.gz)
[![Gentoo](http://www.ltsp.org/images/distro/gentoo.png)](https://github.com/X0rg/CPU-X/wiki/%5BPackage%5D-Gentoo)
[![OpenSUSE](https://harmonyseq.files.wordpress.com/2010/10/opensuse-logo_design_mini.png)](https://github.com/X0rg/CPU-X/releases/download/v3.0.0/CPU-X_v3.0.0_OpenSUSE.tar.gz)
[![SUSE Linux Enterprise](http://i.i.cbsi.com/cnwk.1d/i/tim//2010/06/03/Foreman_11426063_9959_100px-Suse_logo.svg_32x20.png)](https://github.com/X0rg/CPU-X/releases/download/v3.0.0/CPU-X_v3.0.0_SLE.tar.gz)
[![Ubuntu](http://computriks.com/img/icon/ubuntu24.png)](https://github.com/X0rg/CPU-X/releases/download/v3.0.0/CPU-X_v3.0.0_Ubuntu.tar.gz)


### Manual build

For step-by-step guide, you can see the [wiki page](https://github.com/X0rg/CPU-X/wiki) (GNU/Linux or *BSD).  
On some GNU/Linux distributions, you need to install the appropriate **-dev** package.  
You can disable components in CPU-X before build by passing argument `-D<var>=0` when running CMake:  
`-DWITH_GTK=0` will disable support of GUI in GTK3+  
`-DWITH_NCURSES=0` will disable support of TUI in NCurses  
`-DWITH_LIBCPUID=0` will avoid calls to Libcpuid (not recommended)  
`-DWITH_LIBPCI=0` will avoid calls to Libpci (not recommended)  
`-DWITH_LIBSYSTEM=0` will avoid calls to Libprocps/Libstatgrab (not recommended)  
`-DWITH_DMIDECODE=0` will not compile built-in [Dmidecode](http://www.nongnu.org/dmidecode/) (not recommended)  
`-DWITH_BANDWIDTH=0` will not compile built-in [Bandwidth](https://zsmith.co/bandwidth.html) (not recommended)  



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
mkdir pbuild && cd pbuild
cmake -DPORTABLE_BINARY=1 ..
make
```

Note: portable binary is *accomplished/bin/cpu-x*, in directory *pbuild*. Don't do `make install` after.You can move this runnable file where you want.


### Portable version

CPU-X is available in a portable version (Linux 32/64-bit, FreeBSD 32/64-bit), like CPU-Z.  
You can find the last release which depends on GTK [**here**](https://github.com/X0rg/CPU-X/releases/download/v3.0.0/CPU-X_v3.0.0_portable.tar.gz). All others needed librairies are included in the binary.    
Also, if GTK librairies are not present on your system, you can use [**this**](https://github.com/X0rg/CPU-X/releases/download/v3.0.0/CPU-X_v3.0.0_portable_noGTK.tar.gz) instead.  
You can find all downloads on [this page](https://github.com/X0rg/CPU-X/releases).  

After downloading tarball, you need to extract his content to be able to run CPU-X portable.  
You can use this portable version on a lot of system, so you can leave a binary on a USB stick for instance.


## Usage

Start program with **root privileges** allows CPU-X to access some special devices, minimizing empty labels count.  
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

* **CPU-X won't start**: check binary permissions (do a `chmod 755` if they are wrong).
* **CPU-X still won't start**: run it from a shell with `--verbose` argument, and look output.
* **Some labels are empty**: CPU-X needs root privileges to run fine. If you manually build CPU-X, check dependencies. Or else, if a label is still empty, your hardware isn't recognized by a library.


## Bugs/Improvements/Request features

Please [open a new issue](https://github.com/X0rg/CPU-X/issues/new).  
For **bugs**, you can attach `cpu-x -V` and `# cpu-x -ovd` outputs to issue.


## Links
Official [webpage](http://x0rg.github.io/CPU-X/) made by GitHub Pages.  
Official [wiki](https://github.com/X0rg/CPU-X/wiki), still on GitHub.
