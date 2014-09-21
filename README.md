# CPU-X

CPU-X is a Free software that gathers information on CPU & motherboard.
CPU-X is similar to CPU-Z (only available for Microsoft Windows, see http://www.cpuid.com/softwares/cpu-z.html for mor informations), but CPU-X is designed for GNU/Linux.
It is written in C, and GUI uses GTK3+ library, so it's working out-of-box on X11 or Wayland.


## Dependencies

* GTK3+ (version 3.10 or uppers recommanded)
    * GUI
    * http://www.gtk.org/

* Libcpuid
    * Extract CPU features
    * http://libcpuid.sourceforge.net/

* Util-Linux
    * Provide command 'lscpu'
    * http://ftp.kernel.org/pub/linux/utils/util-linux/

* Dmidecode
    * Usefull informations on hardware
    * http://www.nongnu.org/dmidecode/


## Build

GTK headers are needed to compile.

* If you want to install CPU-X on your system, do:
```
make
make install
```

* If you want a portable binary, do:
```
make embed
```

You must do a `make mrproper` between if you want to use both.


## Usage

Start program with root privileges allow to use Dmidecode.
Simply run command `cpu-x` if it is installed on you system, or double-click on `cpu-x_portable` is also possible.

## Screenshots

You can see CPU-X here:
https://github.com/X0rg/CPU-X/wiki/Screenshots
