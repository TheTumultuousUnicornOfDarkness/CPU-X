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
	DST_DIR="$3"
	APPIMAGE=1
else
	DST_DIR=""
	APPIMAGE=0
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
sudo apt-get install -y -qq \
	cmake \
	ninja-build \
	nasm \
	gettext \
	adwaita-icon-theme \
	"${PACKAGES[@]}" \
	libgtk-3-0 \
	libgtk-3-dev \
	libpci-dev \
	opencl-headers \
	ocl-icd-libopencl1 \
	ocl-icd-opencl-dev \
	libprocps-dev

echo "Run CMake"
cmake -S "$SRC_DIR" \
	-B build \
	-GNinja \
	-DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
	-DCMAKE_INSTALL_PREFIX=/usr \
	-DCMAKE_INSTALL_LIBEXECDIR=/usr/bin \
	-DAPPIMAGE=$APPIMAGE

echo "Build CPU-X"
cmake --build build

if [[ -z "$DST_DIR" ]]; then
	echo "Install CPU-X on system"
	sudo ninja -C build install
else
	echo "Install CPU-X in AppDir"
	DESTDIR="$DST_DIR" ninja -C build install
fi
