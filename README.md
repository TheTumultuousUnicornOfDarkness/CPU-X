
[![Logo](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/blob/master/data/icons/CPU-X_22x22.png?raw=true)](https://thetumultuousunicornofdarkness.github.io/CPU-X/)
[![GitHub release](https://img.shields.io/github/release/TheTumultuousUnicornOfDarkness/CPU-X.svg)](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/tags)
[![GitHub commits](https://img.shields.io/github/commits-since/TheTumultuousUnicornOfDarkness/CPU-X/latest.svg)](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/commits/master)
[![GitHub downloads](https://img.shields.io/github/downloads/TheTumultuousUnicornOfDarkness/CPU-X/latest/total.svg)](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/releases/latest)
[![GitHub total downloads](https://img.shields.io/github/downloads/TheTumultuousUnicornOfDarkness/CPU-X/total.svg)](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/releases)
[![GitHub issues](https://img.shields.io/github/issues/TheTumultuousUnicornOfDarkness/CPU-X.svg)](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/issues)
[![GitHub pull-requests](https://img.shields.io/github/issues-pr/TheTumultuousUnicornOfDarkness/CPU-X.svg)](https://GitHub.com/TheTumultuousUnicornOfDarkness/CPU-X/pull)
[![Translation status](https://hosted.weblate.org/widgets/cpu-x/-/svg-badge.svg)](https://hosted.weblate.org/engage/cpu-x/?utm_source=widget)

| Linux | FreeBSD | AppImage |
| :---: | :---: | :---: |
| [![Build Status (Linux)](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/workflows/Linux%20build/badge.svg?branch=master)](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/actions?query=workflow%3A%22Linux+build%22) | [![Build Status (FreeBSD)](https://api.cirrus-ci.com/github/TheTumultuousUnicornOfDarkness/CPU-X.svg)](https://cirrus-ci.com/github/TheTumultuousUnicornOfDarkness/CPU-X) | [![AppImage Continuous](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/workflows/AppImage%20Continuous/badge.svg?branch=master)](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/actions?query=workflow%3A%22AppImage+Continuous%22) |

**CPU-X** is a Free software that gathers information on CPU, motherboard and more.  
CPU-X is a system profiling and monitoring application (similar to [CPU-Z for Windows](https://www.cpuid.com/softwares/cpu-z.html)), but CPU-X is a Free and Open Source software designed for GNU/Linux and FreeBSD.  
This software is written in C++ and built with [CMake](https://www.cmake.org/) tool.  
It can be used in graphical mode by using GTK or in text-based mode by using NCurses. A dump mode is present from command line.  

**:warning: There are no milestones for this project, read [this announcement](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/wiki/future-of-project) for more details.**

***

# Table of contents
- [Table of contents](#table-of-contents)
  - [Dependencies](#dependencies)
    - [Build-only dependencies](#build-only-dependencies)
    - [Test dependencies](#test-dependencies)
    - [Build and run dependencies](#build-and-run-dependencies)
  - [Download and install](#download-and-install)
    - [From repositories](#from-repositories)
    - [From GitHub releases](#from-github-releases)
    - [Manual build](#manual-build)
  - [Usage](#usage)
  - [Wiki](#wiki)
  - [Bugs/Improvements/Request features](#bugsimprovementsrequest-features)
  - [Translate CPU-X](#translate-cpu-x)

***

## Dependencies

### Build-only dependencies

These dependencies are needed to **build** CPU-X:
* A C++ compiler ([GCC](https://gcc.gnu.org/) or [Clang](https://clang.llvm.org/))
* [CMake](https://www.cmake.org/)
* [Pkg-Config](https://www.freedesktop.org/wiki/Software/pkg-config/) / [Pkgconf](https://github.com/pkgconf/pkgconf)
* [NASM](https://www.nasm.us/)

### Test dependencies

These dependencies are optional to **test** CPU-X:
* [Grep](https://www.gnu.org/software/grep/)
* [GNU Awk¹](https://www.gnu.org/software/gawk/)
* [Mawk¹](https://invisible-island.net/mawk/)
* [One true awk¹](https://github.com/onetrueawk/awk)

**¹**At least on Awk is required, e.g. the default Awk on the system.

### Build and run dependencies

These dependencies are needed to **build²** and **run** CPU-X:
* [Gtkmm3](https://www.gtkmm.org/en/index.html) (versions 3.12 to 3.24 are supported)  
* [NCurses](https://www.gnu.org/software/ncurses/)  
* [Libcpuid](http://libcpuid.sourceforge.net/) (version 0.6.0 or newer is needed)  
* [Pciutils](https://mj.ucw.cz/sw/pciutils/)  
* [GLFW](https://www.glfw.org/) (version 3.3 or newer is needed), with [OpenGL](https://www.opengl.org/)  
* [Vulkan](https://www.vulkan.org/)  
* [OpenCL](https://www.khronos.org/opencl/) (version 1.2 or newer is needed), **disabled by default** (enable with `-DWITH_OPENCL=1` during CMake invocation)  
* [Procps-ng](https://sourceforge.net/projects/procps-ng/) (Linux) / [Libstatgrab](https://www.i-scream.org/libstatgrab/) (*BSD)  

**²**On some GNU/Linux distributions, the appropriate **-dev** or **-devel** package is needed.

## Download and install

### From repositories

In alphabetical order:
- [Debian (since version 11 "Bullseye")](https://packages.debian.org/search?searchon=names&keywords=cpu-x): `apt install cpu-x`
- [Fedora (since version 30)](https://src.fedoraproject.org/rpms/cpu-x): `dnf install cpu-x`
- [FreeBSD (since version 11)](https://www.freshports.org/sysutils/cpu-x): `pkg install cpu-x`
- [OpenMandriva Lx (since version 4.0 "Nitrogen")](https://github.com/OpenMandrivaAssociation/cpu-x): `dnf install cpu-x`
- [openSUSE Leap (since version 15.2 from **utilities** project)](https://software.opensuse.org//download.html?project=utilities&package=cpu-x): `zypper install cpu-x`
- [Solus](https://packages.getsol.us/shannon/c/cpu-x/): `eopkg install cpu-x`
- [Ubuntu (since version 20.04 "Focal Fossa")](https://packages.ubuntu.com/search?suite=default&section=all&arch=any&keywords=cpu-x&searchon=names) : `apt install cpu-x`

All packages:  
[![Packaging status](https://repology.org/badge/vertical-allrepos/cpu-x.svg?exclude_unsupported=1)](https://repology.org/project/cpu-x/versions)

### From GitHub releases

Look for **Assets** [here ![GitHub release](https://img.shields.io/github/release/TheTumultuousUnicornOfDarkness/CPU-X.svg)](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/releases/latest) for portable binary (AppImage).

### Manual build

For step-by-step guide, please read [this page](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/wiki/manual-build) on the Wiki.

## Usage

Application is available in menu (**System Tools** category). If you start the daemon, it will allow CPU-X to access some special devices, which provides more information and avoid empty labels.  
The CPU-X daemon requires root privileges, and uses Polkit for privileges escalation.

You can use `cpu-x` from command line, some options are available, like:
- `--gtk`: to start the graphical user interface (default)
- `--ncurses`: to start the text-based user interface
- `--dump`: to get a summary of data

Use `--help` to see all arguments.

## Wiki

More informations are available on Wiki, like screenshots, troubleshooting and tips. You can access the Wiki by using [this link](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/wiki).

## Bugs/Improvements/Request features

Please [open a new issue](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/issues/new/choose) and fill the appropriate template. Note that text between `<!-- ... -->` is not displayed.  
:warning: **Incomplete reports may be closed without justification.**

## Translate CPU-X

If you want to translate CPU-X in your native tongue, please visit the [Weblate page](https://hosted.weblate.org/engage/cpu-x/?utm_source=widget).  
POT file is updated on each changes thanks to [Update POT file workflow](https://github.com/TheTumultuousUnicornOfDarkness/CPU-X/actions?query=workflow%3A%22Update+POT+file%22).
