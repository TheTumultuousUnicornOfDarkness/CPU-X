#!/bin/bash

set -euo pipefail
source /etc/os-release

if [[ $# -lt 2 ]]; then
	echo "$0: BUILD_TYPE SOURCE_DIRECTORY [INSTALL_DIRECTORY]"
	exit 1
fi

if [[ "$ID" != "ubuntu" ]]; then
	echo "$0: this script must be run on a Ubuntu system"
	exit 1
fi

BUILD_TYPE="$1"
SRC_DIR="$2"
if [[ $# -ge 3 ]]; then
	# AppImage build
	DST_DIR="$3"
	CMAKE_EXTRA_OPTIONS="-DAPPIMAGE=1"
else
	# Standard build
	DST_DIR=""
	CMAKE_EXTRA_OPTIONS="-DWITH_OPENCL=1"
fi

case "$VERSION_ID" in
	"20.04") # Focal Fossa
		GCC_VER=10
		PACKAGES=('gcc-10' 'g++-10' 'libprocps-dev' 'libprocps8')
		;;
	"22.04") # Jammy Jellyfish
		GCC_VER=12
		PACKAGES=('gcc-12' 'g++-12' 'libprocps-dev' 'libprocps8')
		;;
	"24.04") # Noble Numbat
		GCC_VER=14
		PACKAGES=('gcc-14' 'g++-14' 'libproc2-dev' 'libproc2-0')
		;;
	*)
		echo "Unsupported Ubuntu version: $VERSION_ID" ; exit 1
		;;
esac

echo "Install packages"
sudo apt-get update -y -qq
sudo apt-get install -y -qq \
	valgrind \
	dpkg-dev \
	gawk \
	mawk \
	"gcc-$GCC_VER" \
	"g++-$GCC_VER" \
	cmake \
	ninja-build \
	nasm \
	gettext \
	libpolkit-gobject-1-dev \
	adwaita-icon-theme \
	libgtkmm-3.0-1v5 \
	libgtkmm-3.0-dev \
	libncursesw6 \
	libncurses-dev \
	libpci3 \
	libpci-dev \
	libglfw3 \
	libglfw3-dev \
	libvulkan-dev \
	libglvnd-dev \
	opencl-headers \
	ocl-icd-libopencl1 \
	ocl-icd-opencl-dev \
	libfuse2 \
	"${PACKAGES[@]}"

sudo update-alternatives --install /usr/bin/gcc gcc "/usr/bin/gcc-$GCC_VER" 9001 # It's Over 9000
sudo update-alternatives --install /usr/bin/g++ g++ "/usr/bin/g++-$GCC_VER" 9002 # It's Over 9000

echo "Run CMake"
cmake -S "$SRC_DIR" \
	-B build \
	-GNinja \
	-DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
	-DCMAKE_INSTALL_PREFIX=/usr \
	-DCMAKE_INSTALL_LIBEXECDIR=/usr/bin \
	$CMAKE_EXTRA_OPTIONS

echo "Build CPU-X"
cmake --build build

if [[ -z "$DST_DIR" ]]; then
	echo "Install CPU-X on system"
	sudo ninja -C build install
else
	echo "Install CPU-X in AppDir"
	DESTDIR="$DST_DIR" ninja -C build install
fi
