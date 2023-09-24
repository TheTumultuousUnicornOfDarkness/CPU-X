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
	CMAKE_EXTRA_OPTIONS=""
else
	# Standard build
	DST_DIR=""
	CMAKE_EXTRA_OPTIONS="-DWITH_OPENCL=1"
fi

case "$VERSION_ID" in
	"20.04") # Focal Fossa
		PACKAGES=('libncurses-dev' 'libncursesw6' 'libpci3' 'libprocps8' 'libglfw3-dev' 'libglfw3' 'libglvnd-dev' 'libvulkan-dev' 'ocl-icd-opencl-dev')
		;;
	"22.04") # Jammy Jellyfish
		PACKAGES=('libncurses-dev' 'libncursesw6' 'libpci3' 'libprocps8' 'libglfw3-dev' 'libglfw3' 'libglvnd-dev' 'libvulkan-dev' 'ocl-icd-opencl-dev')
		;;
	*)
		echo "Unsupported Ubuntu version: $VERSION_ID" ; exit 1
		;;
esac

echo "Install packages"
sudo apt-get update -y -qq
sudo apt-get install -y -qq \
	gcc-10 \
	g++-10 \
	cmake \
	ninja-build \
	nasm \
	gettext \
	adwaita-icon-theme \
	"${PACKAGES[@]}" \
	libgtkmm-3.0-1v5 \
	libgtkmm-3.0-dev \
	libpci-dev \
	opencl-headers \
	ocl-icd-libopencl1 \
	ocl-icd-opencl-dev \
	libprocps-dev \
	dpkg-dev \
	gawk \
	mawk

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 10
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 9
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 10

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
