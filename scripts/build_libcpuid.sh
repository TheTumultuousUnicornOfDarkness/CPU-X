#!/bin/bash

set -euo pipefail

BUILD_PATH="$(mktemp --directory --tmpdir libcpuid-build.XXXXXX)"
BUILD_TYPE="Debug"
INSTALL_DIR=""
CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX:-/usr}"

display_help() {
	echo "Usage: $(basename "$0") [-t BUILD_TYPE] [-i INSTALL_DIR]"
	echo -e "\nOptional arguments:"
	echo "  -t BUILD_TYPE   CMake build type (Debug (default), Release, RelWithDebInfo or MinSizeRel)"
	echo "  -i INSTALL_DIR  Directory where to install files (default is root)"
}

while getopts "t:i:h" opt; do
	case "$opt" in
		t) BUILD_TYPE="$OPTARG";;
		i) INSTALL_DIR="$OPTARG";;
		h) display_help; exit 0;;
		*) display_help; exit 1;;
	esac
done

if [[ -f "/etc/os-release" ]]; then
	source "/etc/os-release"
elif [[ -f "/usr/lib/os-release" ]]; then
	source /usr/lib/os-release
else
	echo "os-release file is not present."
	exit 1
fi
echo "Install packages for $ID"
case "$ID" in
	arch|archarm)
		sudo pacman -S --noconfirm \
			base-devel \
			cmake \
			git \
			ninja
		;;

	debian)
		sudo apt-get install -y -qq \
			build-essential \
			cmake \
			ninja-build \
			git
		;;

	fedora)
		sudo dnf group install -y development-tools
		sudo dnf install -y \
			cmake \
			ninja-build \
			git
		;;

	freebsd)
		CMAKE_INSTALL_PREFIX="/usr/local"
		sudo pkg install -y \
			cmake \
			ninja \
			git
		# workaround for libcpuid
		ln -s /usr/local/lib/pkgconfig/libcpuid.pc /usr/local/libdata/pkgconfig/libcpuid.pc
		;;

	ubuntu)
		sudo apt-get install -y -qq \
			gcc \
			cmake \
			ninja-build \
			git
		;;

	*)
		echo "ID '$ID' is not supported by $0."
		exit 1
esac

echo "Clone libcpuid Git repository to $BUILD_PATH"
git clone https://github.com/anrieff/libcpuid.git "$BUILD_PATH"
cd "$BUILD_PATH"

echo "Run CMake ($BUILD_TYPE)"
cmake -B build \
	-GNinja \
	-DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
	-DCMAKE_INSTALL_PREFIX="$CMAKE_INSTALL_PREFIX" \
	-DBUILD_SHARED_LIBS=OFF

echo "Build libcpuid"
cmake --build build

if [[ -z "$INSTALL_DIR" ]]; then
	echo "Install libcpuid to system"
	sudo cmake --install build
else
	echo "Install libcpuid to $INSTALL_DIR"
	DESTDIR="$INSTALL_DIR" cmake --install build
fi
