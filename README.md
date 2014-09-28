# CPU-X

CPU-X is a Free software that gathers information on CPU & motherboard.
CPU-X is similar to CPU-Z (only available for Microsoft Windows, see http://www.cpuid.com/softwares/cpu-z.html for more informations), but CPU-X is designed for GNU/Linux.
It is written in C, and GUI uses GTK3+ library, so it's working out-of-box on X11 or Wayland.


## Dependencies

* GTK3+ (version 3.10 or uppers recommanded)
    * GUI
    * http://www.gtk.org/

* Libcpuid
    * Extract CPU features
    * http://libcpuid.sourceforge.net/

* Dmidecode
    * Usefull informations on hardware ; a library is provided
    * http://www.nongnu.org/dmidecode/


## Build

First, you must to have CMake installed to continue (only needed for building).
GTK headers are needed to compile.

* If you want to install CPU-X on your system, do:
```
mkdir build && cd build
cmake ..
make
make install
```
By default, CPU-X will be installed in */usr/local*. If you want to change it, change `cmake ..` by `cmake -DCMAKE_INSTALL_PREFIX=YOUR_ABSOLUTE_PATH ..`.

* If you want a portable binary, do:
```
mkdir ebuild && cd ebuild
cmake -DEMBED=1 ..
make
```

Note: portable binary will be in directory "ebuild/bin".

## Usage

Start program with root privileges allow to use Dmidecode.
Simply run command `cpu-x` if it is installed on you system, or double-click on `cpu-x` is also possible.


## Download binairies

You can also download an archive with two portable binaries (i686 & x86_64) at https://github.com/X0rg/CPU-X/releases (file "CPU-X_portable.tar.gz").


## Screenshots

You can see CPU-X here:
https://github.com/X0rg/CPU-X/wiki/Screenshots
