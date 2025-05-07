#!/bin/bash

set -eo pipefail

# Functions
die() {
	echo "[FATAL] $*"
	exit 1
}

runCmd() {
	if ! "$@"; then
		die "command failed: '$*'"
	fi
}

# Constant variables
if [[ $# -lt 2 ]]; then
	echo "$0: WORKSPACE APPDIR"
	exit 1
fi

# declare variables
BUILD_TYPE="${BUILD_TYPE:-RelWithDebInfo}"
WORKSPACE="$(realpath "$1")"
APPDIR="$(realpath "$2")"
WGET_ARGS=(--continue --no-verbose)
ARCH="$(uname -m)"

runCmd mkdir -p "$APPDIR"

[[ -n "$VERSION" ]] && export RELEASE="latest" || export RELEASE="continuous"
export UPDATE_INFORMATION="gh-releases-zsync|${GITHUB_REPOSITORY//\//|}|${RELEASE}|CPU-X-*$ARCH.AppImage.zsync"
echo "UPDATE_INFORMATION=$UPDATE_INFORMATION"

# Install dependencies
runCmd pacman -Syu --noconfirm \
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
	libcpuid \
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
	valgrind \
	vulkan-headers \
	vulkan-icd-loader \
	wget

# install debloated packages
if [ "$(uname -m)" = 'x86_64' ]; then
	PKG_TYPE='x86_64.pkg.tar.zst'
else
	PKG_TYPE='aarch64.pkg.tar.xz'
fi

LIBXML_URL="https://github.com/pkgforge-dev/llvm-libs-debloated/releases/download/continuous/libxml2-iculess-$PKG_TYPE"
runCmd wget "${WGET_ARGS[@]}" "$LIBXML_URL" -O  /tmp/libxml2.pkg.tar.zst
runCmd pacman -U --noconfirm /tmp/libxml2.pkg.tar.zst

echo "Clone libcpuid Git repository"
git clone https://github.com/anrieff/libcpuid.git /tmp/libcpuid && (
	cd /tmp/libcpuid
	echo "Run CMake to build libcpuid"
	runCmd cmake -B build \
		-GNinja \
		-DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DBUILD_SHARED_LIBS=OFF
	runCmd cmake --build build
	ninja -C build install
)

# Build CPU-X
echo "Run CMake to build CPU-X"
runCmd cmake -S . \
	-B build \
	-GNinja \
	-DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
	-DCMAKE_INSTALL_PREFIX=/usr \
	-DCMAKE_INSTALL_LIBEXECDIR=/usr/bin \
	-DAPPIMAGE=1
DESTDIR="$APPDIR" ninja -C build install

# Prepare AppDir
runCmd cp --verbose "$APPDIR/usr/share/applications/io.github.thetumultuousunicornofdarkness.cpu-x.desktop" "$APPDIR"
runCmd cp --verbose "$APPDIR/usr/share/icons/hicolor/256x256/apps/io.github.thetumultuousunicornofdarkness.cpu-x.png" "$APPDIR"
runCmd mv --verbose "$APPDIR"/usr "$APPDIR"/shared
runCmd ln -s ./ "$APPDIR"/usr

# Bundle deps
runCmd wget "${WGET_ARGS[@]}" "https://raw.githubusercontent.com/VHSgunzo/sharun/refs/heads/main/lib4bin"
runCmd chmod --verbose a+x ./lib4bin
runCmd ./lib4bin -p -v -s -k \
	--dst-dir "$APPDIR" \
	"$APPDIR"/shared/bin/cpu-x \
	/usr/lib/libcpuid.so* \
	/usr/lib/libvulkan*.so* \
	/usr/lib/libEGL.so* \
	/usr/lib/libgirepository-*.so* \
	/usr/lib/gvfs/* \
	/usr/lib/gtk-*/*/immodules/*.so \
	/usr/lib/gdk-pixbuf-*/*/loaders/*
runCmd ./lib4bin -s --with-wrappe --dst-dir "$APPDIR"/bin "$APPDIR"/shared/bin/cpu-x-daemon

runCmd cp -r "$APPDIR"/shared/share/* "$APPDIR"/share
runCmd rm -rf "$APPDIR"/shared/share
runCmd glib-compile-schemas "$APPDIR"/share/glib-*/schemas

runCmd ln "$APPDIR"/sharun "$APPDIR"/AppRun
runCmd "$APPDIR"/sharun -g

# Make AppImage
runCmd mkdir --parents --verbose "$WORKSPACE/AppImage" && runCmd cd "$_"
runCmd wget "${WGET_ARGS[@]}" "https://github.com/pkgforge-dev/appimagetool-uruntime/releases/download/continuous/appimagetool-$ARCH.AppImage"
runCmd chmod --verbose a+x ./appimagetool-"$ARCH".AppImage
APPIMAGE_EXTRACT_AND_RUN=1 ./appimagetool-"$ARCH".AppImage \
	--no-appstream -u "$UPDATE_INFORMATION" "$APPDIR"
