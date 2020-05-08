#!/bin/bash

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

safeCopy() {
	local src=("${@:1:$#-1}")
	local dst="${*:$#}"

	for s in "${src[@]}"; do
		runCmd cp "$s" --archive --parents --target-directory="$dst" --verbose
	done
}

# Constant variables
if [[ $# -lt 2 ]]; then
	echo "$0: WORKSPACE APPDIR"
	exit 1
fi

WORKSPACE="$1"
APPDIR="$2"
LIBARCHDIR="lib/x86_64-linux-gnu"

# Add extra files in AppDir
[[ -z "$VERSION" ]] && cp --recursive --verbose "$WORKSPACE/src" "$APPDIR/usr/" # Debug info
safeCopy \
	"/usr/share/"{glib-2.0,terminfo} \
	"/usr/share/icons/"{Adwaita,hicolor,locolor} \
	"/usr/$LIBARCHDIR/"{gdk-pixbuf-2.0,gtk-3.0,libgtk-3-0} \
	"/usr/$LIBARCHDIR/libgdk_pixbuf"* \
	"/usr/$LIBARCHDIR/libgobject"* \
	"/usr/$LIBARCHDIR/libgio"* \
	"$APPDIR"
runCmd glib-compile-schemas "$APPDIR/usr/share/glib-2.0/schemas"

# Download linuxdeploy
BUNDLER="$WORKSPACE/linuxdeploy.AppImage"
runCmd wget --no-verbose "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" --output-document="$BUNDLER"
runCmd chmod --verbose a+x "$BUNDLER"

# Set useful variables for linuxdeploy
export ARCH=x86_64
export UPDATE_INFORMATION="gh-releases-zsync|${GITHUB_REPOSITORY//\//|}|${VERSION:-"continuous"}|CPU-X-*$ARCH.AppImage.zsync"
export VERBOSE=1
export DISABLE_COPYRIGHT_FILES_DEPLOYMENT=1

# Run linuxdeploy
runCmd mkdir --parents --verbose "$WORKSPACE/AppImage" && runCmd cd "$_"
runCmd "$BUNDLER" \
	--appdir="$APPDIR" \
	--custom-apprun="$WORKSPACE/scripts/run_appimage.sh" \
	--desktop-file="$APPDIR/usr/share/applications/cpu-x.desktop" \
	--output appimage \
	--verbosity=1
