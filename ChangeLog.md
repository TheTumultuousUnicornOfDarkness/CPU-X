# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

- [Changelog](#changelog)
	- [[v4.2.0] - 2021-03-28](#v420---2021-03-28)
		- [Added](#added)
		- [Changed](#changed)
		- [Removed](#removed)
		- [Fixed](#fixed)
	- [[v4.1.0] - 2021-01-10](#v410---2021-01-10)
		- [Added](#added-1)
		- [Changed](#changed-1)
		- [Fixed](#fixed-1)
	- [[v4.0.1] - 2020-06-13](#v401---2020-06-13)
		- [Changed](#changed-2)
		- [Fixed](#fixed-2)
	- [[v4.0.0] - 2020-05-17](#v400---2020-05-17)
		- [Added](#added-2)
		- [Changed](#changed-3)
		- [Removed](#removed-1)
		- [Fixed](#fixed-3)
	- [[v3.2.4] - 2019-01-13](#v324---2019-01-13)
		- [Changed](#changed-4)
		- [Deprecated](#deprecated)
		- [Fixed](#fixed-4)
	- [[v3.2.3] - 2018-07-01](#v323---2018-07-01)
		- [Added](#added-3)
		- [Changed](#changed-5)
		- [Removed](#removed-2)
	- [[v3.2.2] - 2018-05-03](#v322---2018-05-03)
		- [Changed](#changed-6)
		- [Fixed](#fixed-5)
	- [[v3.2.1] - 2018-03-24](#v321---2018-03-24)
		- [Changed](#changed-7)
		- [Removed](#removed-3)
		- [Fixed](#fixed-6)
	- [[v3.2.0] - 2018-01-31](#v320---2018-01-31)
		- [Added](#added-4)
		- [Changed](#changed-8)
		- [Removed](#removed-4)
		- [Fixed](#fixed-7)
	- [[v3.1.3] - 2016-10-25](#v313---2016-10-25)
		- [Added](#added-5)
		- [Changed](#changed-9)
		- [Fixed](#fixed-8)
	- [[v3.1.2] - 2016-10-23](#v312---2016-10-23)
		- [Added](#added-6)
		- [Changed](#changed-10)
		- [Fixed](#fixed-9)
	- [[v3.1.1] - 2016-10-16](#v311---2016-10-16)
		- [Fixed](#fixed-10)
	- [[v3.1.0] - 2016-09-24](#v310---2016-09-24)
		- [Added](#added-7)
		- [Changed](#changed-11)
		- [Removed](#removed-5)
		- [Fixed](#fixed-11)
	- [[v3.0.1] - 2016-08-23](#v301---2016-08-23)
		- [Changed](#changed-12)
		- [Fixed](#fixed-12)
	- [[v3.0.0] - 2016-06-15](#v300---2016-06-15)
		- [Added](#added-8)
		- [Changed](#changed-13)
		- [Removed](#removed-6)
	- [[v2.2.2] - 2015-12-02](#v222---2015-12-02)
		- [Changed](#changed-14)
		- [Fixed](#fixed-13)
	- [[v2.2.1] - 2015-11-27](#v221---2015-11-27)
		- [Changed](#changed-15)
		- [Fixed](#fixed-14)
	- [[v2.2.0] - 2015-11-21](#v220---2015-11-21)
		- [Added](#added-9)
		- [Changed](#changed-16)
		- [Fixed](#fixed-15)
	- [[v2.1.1] - 2015-08-26](#v211---2015-08-26)
		- [Added](#added-10)
		- [Changed](#changed-17)
		- [Fixed](#fixed-16)
	- [[v2.1.0] - 2015-08-08](#v210---2015-08-08)
		- [Added](#added-11)
		- [Changed](#changed-18)
		- [Fixed](#fixed-17)
	- [[v2.0.3] - 2015-03-15](#v203---2015-03-15)
		- [Added](#added-12)
		- [Changed](#changed-19)
		- [Removed](#removed-7)
		- [Fixed](#fixed-18)
	- [[v2.0.2] - 2015-02-08](#v202---2015-02-08)
		- [Added](#added-13)
		- [Changed](#changed-20)
		- [Fixed](#fixed-19)
	- [[v2.0.1] - 2014-11-23](#v201---2014-11-23)
		- [Added](#added-14)
		- [Changed](#changed-21)
		- [Fixed](#fixed-20)
	- [[v2.0.0] - 2014-11-16](#v200---2014-11-16)
		- [Added](#added-15)
		- [Changed](#changed-22)
		- [Fixed](#fixed-21)
	- [[v1.2.2] - 2014-11-05](#v122---2014-11-05)
		- [Added](#added-16)
		- [Changed](#changed-23)
		- [Fixed](#fixed-22)
	- [[v1.2.1] - 2014-10-24](#v121---2014-10-24)
		- [Added](#added-17)
		- [Changed](#changed-24)
		- [Fixed](#fixed-23)
	- [[v1.2.0] - 2014-10-12](#v120---2014-10-12)
		- [Added](#added-18)
		- [Changed](#changed-25)
		- [Fixed](#fixed-24)
	- [[v1.1.0] - 2014-09-28](#v110---2014-09-28)
		- [Added](#added-19)
		- [Changed](#changed-26)
	- [[v1.0.0] - 2014-09-21](#v100---2014-09-21)

---

## [v4.2.0] - 2021-03-28

### Added

- Add new option `--cpuid-decimal` to display CPUID values in decimal
- Add User Mode Driver (UMD) to Graphics Tab (require GLFW)
- Add GPU DeviceID:RevisonID in Graphics tab
- Add PCIe link speed/width in Graphics tab

### Changed

- Reduce the GPU clock precision in Graphics tab
- Reserve 'HT' keyword for Intel CPUs only, and use 'SMT' for other vendors instead
- Update CPU databases
- Support more GPU in Graphics tab
- Use unit prefixes in System tab
- Add a dropdown list in Graphics tab to choose graphic card to monitor

### Removed

- Remove support for libcpuid before v0.5.0

### Fixed

- Apply dmidecode upstream patch to fix warnings
- Fix for `--issue-fmt`
- Fix overflow caused by some translations
- Fix socket path on FreeBSD

---

## [v4.1.0] - 2021-01-10

### Added

- Add `CPUX_ARGS` environment variable to set default command line
- Add alternative key mapping for NCurses mode (option `--keymap`)
- Add debug mode (option `--debug`)
- Add screen reader accessibility on GUI
- Add Core Voltage, Power Avg and Memory Used in Graphics tab (AMDGPU only)

### Changed

- Prefix all hexadecimal values with `0x`
- Reword nonsensical messages
- Update databases
- Request `Tdie` temperature for `k10temp` if available
- Change bars color in System tab in GTK GUI when Dark theme is used
- Patch dmidecode to version 3.3.3c111e4
- Use binary prefixes for System tab

### Fixed

- Fix build on FreeBSD when GTK is enabled
- Fix build on musl libc
- Fix list of influenceable environment variables in help
- Fix override of refresh value when GTK is enabled
- Fix screen flickering in NCurses TUI
- Ignore batteries voltage when searching CPU voltage
- Fix build when gettext support is disabled
- Allow to run CPU-X daemon from AppImage

---

## [v4.0.1] - 2020-06-13

### Changed

- Optimize images
- Prioritize rootless GPU load percentage retrievement for AMDGPU GPU

### Fixed

- Fix double MHz and % symbols on Graphics page with NVIDIA GPU
- Fix reopening of settings window in GTK GUI
- Fix `optirun: command not found` error with NVIDIA GPU
- Mount debugfs if not mounted before reading GPU load percentage for AMDGPU GPU

---

## [v4.0.0] - 2020-05-17

### Added

- Add a daemon to handle privileged access
- Add basic completions for Bash/Fish/Zsh
- Add settings window in GUI
- Add "Driver" label in Graphics tab
- Retrieve CPU temperature on FreeBSD
- Add support for zenpower module
- Add continuous build of AppImage

### Changed

- Patch dmidecode to version 3.2.5b3c8e9
- Move translations from Transifex to Weblate
- Uniform all units (byte and octet)
- Rewrite all CLI options
- Write output to `/tmp/cpu-x.log` and `/tmp/cpu-x-daemon.log` files when `--issue-fmt` is used
- Update databases
- Replace `nvidia-settings` command calls by `nvidia-smi`
- Support for L1 Instruction Cache information 

### Removed

- Remove portable binary
- Remove libcurl dependency
- Remove libjson-c dependency
- Remove all privileged access in `cpu-x` binary (moved to `cpu-x-daemon`)
- Remove "CPU-X (Root)" desktop launcher

### Fixed

- Fix some awk regex
- Fix `load_module()` function
- Fix options parsing
- Fix build on FreeBSD
- Various C fixes (unsafe functions and warnings)
- Fix Bumblebee support for NVIDIA/Nouveau
- Fix with VFIO GPU driver
- Fix GUI switches appearance in Bench tab
- Fix issues with AppImage
- Various fixes in NCurses TUI

---

## [v3.2.4] - 2019-01-13

### Changed

- Build portable binary without PIE
- Improve AppImage experience
- Use GitHub API to check new version (libjson-c)

### Deprecated

- Deprecate the portable version

### Fixed

- Wrong GPU clocks with AMDGPU driver
- Empty memory bank label
- Build without gettext

---

## [v3.2.3] - 2018-07-01

### Added

- `TEXTDOMAINDIR`/`TERMINFO` support
- Add AppData metainfo file
- NVIDIA Bumblebee support

### Changed

- Patch Dmidecode to v3.1.20180620

### Removed

- Support for CMake < 3.0

---

## [v3.2.2] - 2018-05-03

### Changed

- Update CPU database

### Fixed

- Segfault when retrieving AMD GPU temperature

---

## [v3.2.1] - 2018-03-24

### Changed

- Switch Cache and Swap colors in System tab

### Removed

- Drop 32-bit portable version for future releases

### Fixed

- Swap bar in NCurses TUI
- Bug in Bandwidth related to AVX instructions

---

## [v3.2.0] - 2018-01-31

### Added

- Add ability to read CPUID raw file (CPUX_CPUID_RAW environment variable)
- Add `--issue-fmt` argument
- Allow to enforce BCLK (CPUX_BCLK environment variable)
- New GTK theme for Dark themes
- Retrive CPU frequency in fallback mode
- Add GPU usage and GPU clocks in Graphics tab
- Add Polish translation (thanks to eloaders)
- Add Russian translation (thanks to TotalCaesar659)
- Add Czech translation (thanks to pavelb)
- Add Chinese translation (thanks to 高垚鑫)

### Changed

- Update CPU database
- Display influenceable environment variables in help
- Display CPU family/model in hex only
- Add a second line for Instructions label
- Improve CPU temperature and voltage retrieval in fallback mode
- Improve GPU temperature retrieval
- Rework GPU detection and improve multi-GPU support
- Patch Dmidecode to v3.1.20180131
- Patch Bandwidth to v1.5.1

### Removed

- Support for libcpuid < 0.4.0

### Fixed

- SSE3 feature detection
- Cache labels format
- `--nocolor` option
- Set window icon in GTK GUI
- cpu-x_polkit command on Wayland (used by cpu-x-root.desktop)
- Buffer overflow in some cases when Dmidecode is called

---

## [v3.1.3] - 2016-10-25

### Added

- Add Russian translation to shortcuts (thanks to TotalCaesar659)

### Changed

- Add more AMD Kaveri CPUs in database
- Print CPUID raw dumps when using `--dump --verbose`

### Fixed

- Segfault in `call_libcpuid_static()`
- GTK GUI theme

---

## [v3.1.2] - 2016-10-23

### Added

- Allow to set Bclk through CPUX_BCLK environment variable

### Changed

- Avoid to refresh Bclk and minimum/maximum CPU multipliers
- Rework CPU multipliers calculation
- Add AMD Tyler CPUs in database
- Reorganize databases

### Fixed

- Segfault caused by `free_multi()`
- Socket detection in fallback mode
- Kernel module load in fallback mode

---

## [v3.1.1] - 2016-10-16

### Fixed

- Bandwidth build on system without stropts.h
- Segfault in `cpu_multipliers_fallback()`
- Asking for update when already up-to-date in portable version
- Dynamic allocation checking
- Memory leaks

---

## [v3.1.0] - 2016-09-24

### Added

- Libcurl support
- Libarchive support in portable version
- Add support for L4 cache in Caches tab
- Add `--tab` option

### Changed

- Patch Dmidecode to 3.0.20160907
- Rebase Bandwidth on v1.3.1
- Decrease Dmidecode verbosity with `--verbose`
- Merge Descriptor label in Size label in Caches tab
- Hide absent cache levels in Caches tab in GTK GUI
- Hide empty pages in GTK GUI
- Refactor Memory tab
- Various minor core enhancements

### Removed

- Support for libcpuid < 0.3.0
- Support for GTK 3.8 & 3.10

### Fixed

- Broken redirection with `--dump`
- Memory leaks
- Bandwidth build with -DWITH_LIBCPUID=0
- Set speed to 0 for unavailable Bandwidth tests

---

## [v3.0.1] - 2016-08-23

### Changed

- Add more CPU and sockets in database
- Decrease verbosity in Dmidecode and Bandwidth
- Improve fallback mode

### Fixed

- Refresh in fallback mode
- Detection of GPU temperature with NVIDIA proprietary driver
- Check for a new version when network is unreachable
- Various minor fixes

---

## [v3.0.0] - 2016-06-15

### Added

- Report total CPU usage
- Add a signal handler to provide backtrace on crash
- Add `--nocolor` option
- Add `--cachetest` option
- Add `--bandwidth` option
- Add `--core` option
- Add a Bench tab

### Changed

- More CMake improvements, allow to build with CMake 2.8
- Full core rewrite
- Improve options parsing
- Rewrite update module for portable version + add `--update` option
- Rewrite NCurses UI + add color support + add help
- CPU-X logo redesign
- Patch dmidecode with latest source code (commit cff11af)
- Rebase bandwidth on v1.1
- Recognize more CPUs for the "Technology" label in CPU tab
- Print some values > 9 in hexadecimal in CPU tab
- More fallback support
- More strings in `--verbose` mode
- A lot of GTK UI tweaks
- Run bandwith in a separate thread to avoid UI slowdown

### Removed

- Drop support for Darwin/Mac OS X
- Report CPU BogoMIPS value

---

## [v2.2.2] - 2015-12-02

### Changed

- Add technology report for some Clarksfield & Richland CPUs

### Fixed

- Avoid to free dynamic labels in Caches tab
- Free memory when possible in `pcidev()` function
- Fix a buffer overflow on HT detection, causing bug in bandwidth

---

## [v2.2.1] - 2015-11-27

### Changed

- Add debugs symbols in bandwidth source code when using CMAKE_BUILD_TYPE=Debug

### Fixed

- Segfault on NULL pointer in `bandwidth()`
- Buffer overflow when setting label name in Caches tab

---

## [v2.2.0] - 2015-11-21

### Added

- Add support for libcpuid 0.2.2
- Add new tab Caches
- Add labels Technology, Voltage and Temp in CPU tab
- Add lebel Temperature in Graphics tab

### Changed

- Some changes in options parsing
- Update to dmidecode 3.0
- In NCurses TUI, add tab names
- Merge Architecture label in Instructions label in CPU tab
- Merge GPU driver with GPU vendor in Graphics tab
- Update GUI for GTK >= 3.8 and < 3.14

### Fixed

- Typo
- L3 cache value
- Some problems with `libdmi_fallback()`
- .desktop files
- Build with Clang
- Stop update if curl is missing

---

## [v2.1.1] - 2015-08-26

### Added

- Add translations support in portable version
- Add HyperThreading detection
- Option `--version` informs if a new version is available
- Add an auto-update module for portable version
- Add pt_BR translation (thanks to ShyPixie)

### Changed

- Some text changes in `--verbose` mode
- Improve translations support (rewrite target updatepo, add target newpo...)

### Fixed

- Segfault with `strdup()` on NULL pointer
- Avoid multipliers "(0 ## 0)" in CPU tab and "-nan" values in System tab
- Fixes garbages after freeing
- Some problems with build
- Do changes to enable compilation with libpci.a in portable version

---

## [v2.1.0] - 2015-08-08

### Added

- Add libsystem
- Add a button to change GTK GUI color
- Add new chipset section in Motherboard tab
- Add new tab Graphics
- GTK 3.14 support / partial GTK 3.16 support

### Changed

- Improve drawing bars in System tab in GTK GUI
- Use dynamic arrays instead of static arrays
- Use GResource instead of GdkPixbuf in GTK embedded GUI

### Fixed

- Build
- Options
- Deprecated functions in GTK GUI code

---

## [v2.0.3] - 2015-03-15

### Added

- Add verbose mode (for CPU-X, previously Dmidecode only)
- Colorized messages
- Add a "Run as root" button in GTK GUI
- Add `make uninstall` target to allow to properly uninstall CPU-X
- Add `make genpot` target to generate a pot file
- Add `make updatepo` target to update a po file from a newer pot file

### Changed

- Better support for non-Linux OS
- Improve displaying of memory usage in System tab

### Removed

- Remove RPATH

### Fixed

- Stop spam errors (in Dmidecode)
- Options when built without GTK
- Incomplete possibility for translation
- Output messages (verbose and error)

---

## [v2.0.2] - 2015-02-08

### Added

- Add argument `--dmidecode` to run (internal) Dmidecode alone

### Changed

- Rebase dmidecode code on 2.12
- Improve options parsing
- Less (useless) function recalls on refresh loop
- Display dashes for empty banks in RAM tab

### Fixed

- Empty RAM tab on certain machines

---

## [v2.0.1] - 2014-11-23

### Added

- Add possibility to resize terminal with NCurses TUI

### Changed

- Better CPU multiplier calculation
- Hide non-existent banks in tab RAM
- Better compiler detection
- Improve option `--help`
- Better translation support

### Fixed

- NCurses TUI: correct refresh (when left key/button 3 on mouse are spammed)
- NCurses TUI: segfault when spamming right key
- GTK GUI/NCurses TUI: use timeout instead of new thread to refresh
- Segfault when file `/etc/os-release` could not be open
- Add more static libraries in portable binary

---

## [v2.0.0] - 2014-11-16

### Added

- Add argument `--dump` (no start GUI)
- Add fallback mode for Libdmi (but it not replaces Libdmi)
- Add new tab System
- Add new tab RAM
- Add support for translations (only French available)
- Add CPU-X launchers (depends of GTK); cpu-x.desktop and cpu-x-root.desktop
- Add possibility to run CPU-X as root with pkexec
- Add more CPU vendor logos

### Changed

- Big changes in core
- Full rewrite GTK GUI
- Minor changes in NCurses TUI

### Fixed

- Segfault when compiling with target Release
- Memory leak (with function get_path)

---

## [v1.2.2] - 2014-11-05

### Added

- Add argument `--verbose` (set Dmidecode verbose)

### Changed

- Print nothing on impossible values (Caches section)

### Fixed

- Stop distort GTK GUI when label length is large
- Stop spam errors (about CPU frequencies)
- Typos (BogoMIPS, CPU Vendor, label Manufacturer)

---

## [v1.2.1] - 2014-10-24

### Added

- Add arguments support (`--no-gui`, `--version`, `--help`)
- Add support for custom refresh time (`--refresh`)

### Changed

- Rewrite error messages
- Restructuration of files and functions in files

### Fixed

- Segfault when compiling without GTK & embeded
- Unwanted characters in NCurses
- Compilation with CMake

---

## [v1.2.0] - 2014-10-12

### Added

- Add NCurses mode

### Changed

- Add possibility to disable GTK/NCurses/Libcpuid/Libdmi before compiling

### Fixed

- CPU multipliers calculation
- Relative path of file on error
- Segfault on unknown multiplier
- Unwanted characters

---

## [v1.1.0] - 2014-09-28

### Added

- Add possibility to change install prefix

### Changed

- Change build system from Makefile to CMake
- Remove calls to external command `dmidecode`, use provided library instead of
- Remove calls to external command `lscpu`

---

## [v1.0.0] - 2014-09-21

- Initial release
