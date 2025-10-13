#!/usr/bin/env bash

set -euo pipefail

SRC_DIR=""
APPDIR=""
GH_USERNAME=""
VERSION=""
ARCH="$(uname -m)"
APPIMAGETOOL_OPTIONS=()

download_file() {
	local url="$1"
	local file

	if [[ -z "$url" ]]; then
		echo "download_file: missing URL"
		exit 1
	fi

	echo "Downloading $url..."
	file="$(basename "$url")"
	wget --continue --no-verbose "$url"
	if grep --no-messages --quiet --ignore-case --extended-regexp 'executable|shell script' "$file"; then
		chmod --verbose a+x "$file"
	fi
}

display_help() {
	echo "Usage: $(basename "$0") -s SRC_DIR -a APPDIR [-u GH_USERNAME] [-v VERSION]"
	echo -e "\nMandatory arguments:"
	echo "  -s SRC_DIR      Path to the CPU-X source directory"
	echo "  -a APPDIR       Path for AppDir"
	echo -e "\nOptional arguments:"
	echo "  -u GH_USERNAME  GitHub repository owner name (for update information)"
	echo "  -v VERSION      CPU-X version"
}

while getopts "s:a:u:v:h" opt; do
	case "$opt" in
		s) SRC_DIR="$(realpath "$OPTARG")";;
		a) APPDIR="$(realpath "$OPTARG")";;
		u) GH_USERNAME="$OPTARG";;
		v) VERSION="$OPTARG"; export VERSION;;
		h) display_help; exit 0;;
		*) display_help; exit 1;;
	esac
done

if [[ -z "$SRC_DIR" ]] || [[ -z "$APPDIR" ]]; then
	display_help
	exit 1
fi

if [[ -n "$GH_USERNAME" ]]; then
	[[ -n "$VERSION" ]] && release="latest" || release="continuous"
	update_information="gh-releases-zsync|${GH_USERNAME}|${release}|CPU-X-*$ARCH.AppImage.zsync"
	APPIMAGETOOL_OPTIONS+=("--updateinformation" "$update_information")
	echo "Update information: $update_information"
fi

# Install dependencies
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
		sudo pacman -S --noconfirm wget appstream
		# install debloated packages
		case "$ARCH" in
			x86_64) PKG_TYPE="x86_64.pkg.tar.zst";;
			aarch64) PKG_TYPE="aarch64.pkg.tar.xz";;
			*) echo "Arch '$ARCH' is not supported by $0."; exit 1;;
		esac
		pushd "/tmp"
		download_file "https://github.com/pkgforge-dev/llvm-libs-debloated/releases/download/continuous/libxml2-mini-$PKG_TYPE"
		download_file "https://github.com/pkgforge-dev/llvm-libs-debloated/releases/download/continuous/gdk-pixbuf2-mini-$PKG_TYPE"
		download_file "https://github.com/pkgforge-dev/llvm-libs-debloated/releases/download/continuous/gtk3-mini-$PKG_TYPE"
		sudo pacman -U --noconfirm "libxml2-mini-$PKG_TYPE" "gtk3-mini-$PKG_TYPE" "gdk-pixbuf2-mini-$PKG_TYPE"
		popd
		;;

	*)
		echo "ID '$ID' is not supported by $0."
		exit 1
esac

echo "Prepare '$APPDIR' AppDir"
mkdir --verbose --parents "$APPDIR"
cp --verbose "$APPDIR/usr/share/applications/io.github.thetumultuousunicornofdarkness.cpu-x.desktop" "$APPDIR"
cp --verbose "$APPDIR/usr/share/icons/hicolor/256x256/apps/io.github.thetumultuousunicornofdarkness.cpu-x.png" "$APPDIR"
mv --verbose "$APPDIR"/usr "$APPDIR/shared"
ln --verbose --symbolic "./" "$APPDIR/usr"

echo "Bundle binaries in '$APPDIR' AppDir"
download_file "https://raw.githubusercontent.com/VHSgunzo/sharun/refs/heads/main/lib4bin"
./lib4bin \
	--verbose \
	--hard-links \
	--strip \
	--with-hooks \
	--dst-dir "$APPDIR" \
	"$APPDIR"/shared/bin/cpu-x \
	/usr/lib/libcpuid.so* \
	/usr/lib/libvulkan*.so* \
	/usr/lib/libEGL.so* \
	/usr/lib/libgirepository-*.so* \
	/usr/lib/gvfs/* \
	/usr/lib/gio/modules/libdconfsettings.so \
	/usr/lib/gtk-*/*/immodules/*.so \
	/usr/lib/gdk-pixbuf-*/*/loaders/*
./lib4bin \
	--strip \
	--with-wrappe \
	--dst-dir "$APPDIR/bin" \
	"$APPDIR/shared/bin/cpu-x-daemon"
cp --verbose --recursive "$APPDIR/shared/share/"* "$APPDIR/share"
rm --verbose --recursive --force "$APPDIR/shared/share"
glib-compile-schemas "$APPDIR/share/glib-"*/schemas
ln --verbose "$APPDIR/sharun" "$APPDIR/AppRun"
"$APPDIR/sharun" --gen-lib-path

echo "Create AppImage from '$APPDIR' AppDir"
mkdir --parents --verbose "$SRC_DIR/AppImage" && cd "$_"
download_file "https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-$ARCH.AppImage"
download_file "https://github.com/VHSgunzo/uruntime/releases/latest/download/uruntime-appimage-squashfs-lite-$ARCH"
APPIMAGE_EXTRACT_AND_RUN=1 "./appimagetool-$ARCH.AppImage" \
	--mksquashfs-opt -Xcompression-level --mksquashfs-opt 22 \
	--mksquashfs-opt -b --mksquashfs-opt 1M \
	--runtime-file "./uruntime-appimage-squashfs-lite-$ARCH" \
	"${APPIMAGETOOL_OPTIONS[@]}" \
	"$APPDIR"
