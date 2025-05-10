#!/usr/bin/env bash

set -euo pipefail

SRC_DIR=""
BUILD_TYPE="Debug"
INSTALL_DIR=""
CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX:-/usr}"
CMAKE_INSTALL_LIBEXECDIR="${CMAKE_INSTALL_LIBEXECDIR:-/usr/bin}"
CMAKE_EXTRA_OPTIONS=()

display_help() {
	echo "Usage: $(basename "$0") -s SRC_DIR [-t BUILD_TYPE] [-i INSTALL_DIR]"
	echo -e "\nMandatory arguments:"
	echo "  -s SRC_DIR      Path to the CPU-X source directory"
	echo -e "\nOptional arguments:"
	echo "  -t BUILD_TYPE   CMake build type (Debug (default), Release, RelWithDebInfo or MinSizeRel)"
	echo "  -i INSTALL_DIR  Directory where to install files (default is root)"
}

while getopts "s:t:i:h" opt; do
	case "$opt" in
		s) SRC_DIR="$OPTARG";;
		t) BUILD_TYPE="$OPTARG";;
		i) INSTALL_DIR="$OPTARG"; CMAKE_EXTRA_OPTIONS+=('-DAPPIMAGE=1');;
		h) display_help; exit 0;;
		*) display_help; exit 1;;
	esac
done

if [[ -z "$SRC_DIR" ]]; then
	display_help
	exit 1
fi

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
			binutils \
			cairomm \
			cmake \
			dconf \
			findutils \
			gawk \
			gcc-libs \
			git \
			glib2 \
			glibc \
			glibmm \
			gtkmm3 \
			libglvnd \
			libsigc++ \
			nasm \
			ncurses \
			ninja \
			opencl-headers \
			opencl-icd-loader \
			pangomm \
			patchelf \
			pciutils \
			polkit \
			procps-ng \
			strace \
			vulkan-headers \
			vulkan-icd-loader
		;;

	debian)
		case "$VERSION_ID" in
			"11") # Bullseye
				DEBIAN_PACKAGES=('procps' 'libprocps-dev')
				;;
			"12") # Bookworm
				DEBIAN_PACKAGES=('libproc2-0' 'libproc2-dev')
				;;
			*)
				echo "Unsupported Debian version: $VERSION_ID"
				exit 1
				;;
		esac
		sudo apt-get install -y -qq \
			build-essential \
			cmake \
			ninja-build \
			pkgconf \
			nasm \
			gawk \
			gettext \
			libgtkmm-3.0-1v5 \
			libgtkmm-3.0-dev \
			libncurses6 \
			libncurses-dev \
			libpci3 \
			libpci-dev \
			libglvnd0 \
			libglvnd-dev \
			libvulkan1 \
			libvulkan-dev \
			opencl-c-headers \
			ocl-icd-libopencl1 \
			ocl-icd-opencl-dev \
			"${DEBIAN_PACKAGES[@]}"
		;;

	fedora)
		sudo dnf group install -y development-tools
		sudo dnf install -y \
			gcc-c++ \
			cmake \
			ninja-build \
			pkgconf \
			nasm \
			gettext \
			gettext-devel \
			gtkmm3.0 \
			gtkmm3.0-devel \
			ncurses-libs \
			ncurses-devel \
			pciutils \
			pciutils-devel \
			libglvnd-opengl \
			libglvnd-egl \
			libglvnd-devel \
			vulkan-loader \
			vulkan-loader-devel \
			vulkan-headers \
			opencl-headers \
			ocl-icd \
			ocl-icd-devel \
			procps-ng \
			procps-ng-devel
		;;

	freebsd)
		sudo pkg install -y \
			cmake \
			ninja \
			pkgconf \
			gettext \
			nasm \
			gtkmm30 \
			ncurses \
			pciutils \
			libglvnd \
			opencl \
			ocl-icd \
			vulkan-loader \
			vulkan-headers \
			libstatgrab
		# workaround for OpenCL headers
		sudo sed -i .orig "/Requires: OpenCL-Headers/d" /usr/local/libdata/pkgconfig/OpenCL.pc
		;;

	opensuse-leap)
		sudo zypper install -y -t pattern devel_basis
		sudo zypper install -y \
			gcc14-c++ \
			libstdc++6-devel-gcc14 \
			cmake \
			ninja \
			pkg-config \
			nasm \
			gettext-tools \
			libgtkmm-3_0-1 \
			gtkmm3-devel \
			libncurses6 \
			ncurses-devel \
			pciutils \
			pciutils-devel \
			libglvnd \
			libglvnd-devel \
			vulkan \
			vulkan-devel \
			libOpenCL1 \
			ocl-icd-devel \
			libprocps8 \
			procps-devel
		sudo update-alternatives --install /usr/bin/gcc gcc "/usr/bin/gcc-14" 9001
		sudo update-alternatives --install /usr/bin/g++ g++ "/usr/bin/g++-14" 9002
		;;

	ubuntu)
		case "$VERSION_ID" in
			"20.04") # Focal Fossa
				GCC_VER=10
				UBUNTU_PACKAGES=('libprocps-dev' 'libprocps8')
				;;
			"22.04") # Jammy Jellyfish
				GCC_VER=12
				UBUNTU_PACKAGES=('libprocps-dev' 'libprocps8')
				;;
			"24.04") # Noble Numbat
				GCC_VER=14
				UBUNTU_PACKAGES=('libproc2-dev' 'libproc2-0')
				;;
			"25.04") # Plucky Puffin
				GCC_VER=15
				UBUNTU_PACKAGES=('libproc2-dev' 'libproc2-0')
				;;
			*)
				echo "Unsupported Ubuntu version: $VERSION_ID"
				exit 1
				;;
		esac
		sudo apt-get install -y -qq \
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
			libglvnd0 \
			libglvnd-dev \
			libvulkan-dev \
			libglvnd-dev \
			opencl-headers \
			ocl-icd-libopencl1 \
			ocl-icd-opencl-dev \
			"${UBUNTU_PACKAGES[@]}"
		sudo update-alternatives --install /usr/bin/gcc gcc "/usr/bin/gcc-$GCC_VER" 9001 # It's Over 9000
		sudo update-alternatives --install /usr/bin/g++ g++ "/usr/bin/g++-$GCC_VER" 9002 # It's Over 9000
		;;

	*)
		echo "ID '$ID' is not supported by $0."
		exit 1
esac

echo "Run CMake ($BUILD_TYPE)"
cmake -S "$SRC_DIR" \
	-B build \
	-GNinja \
	-DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
	-DCMAKE_INSTALL_PREFIX="$CMAKE_INSTALL_PREFIX" \
	-DCMAKE_INSTALL_LIBEXECDIR="$CMAKE_INSTALL_LIBEXECDIR" \
	"${CMAKE_EXTRA_OPTIONS[@]}"

echo "Build CPU-X"
cmake --build build

if [[ -z "$INSTALL_DIR" ]]; then
	echo "Install CPU-X to system"
	sudo cmake --install build
else
	echo "Install CPU-X to $INSTALL_DIR"
	DESTDIR="$INSTALL_DIR" cmake --install build
fi
