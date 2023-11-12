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

WORKSPACE="$1"
APPDIR="$2"
WGET_ARGS=(--continue --no-verbose)

# Reset arguments
set --

# Download linuxdeploy and plugins
BUNDLER="$WORKSPACE/linuxdeploy.AppImage"
runCmd wget "${WGET_ARGS[@]}" "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" --output-document="$BUNDLER" \
	&& set -- "$@" --output appimage
runCmd wget "${WGET_ARGS[@]}" "https://raw.githubusercontent.com/linuxdeploy/linuxdeploy-plugin-gtk/master/linuxdeploy-plugin-gtk.sh" \
	&& set -- "$@" --plugin gtk
runCmd wget "${WGET_ARGS[@]}" "https://raw.githubusercontent.com/linuxdeploy/linuxdeploy-plugin-ncurses/master/linuxdeploy-plugin-ncurses.sh" \
	&& set -- "$@" --plugin ncurses
runCmd wget "${WGET_ARGS[@]}" "https://raw.githubusercontent.com/linuxdeploy/misc-plugins/master/gettext/linuxdeploy-plugin-gettext.sh" \
	&& set -- "$@" --plugin gettext
#if [[ -z "$VERSION" ]]; then
#	export LINUXDEPLOY_PLUGIN_GDB_SRC="$WORKSPACE/src"
#	runCmd wget "${WGET_ARGS[@]}" "https://raw.githubusercontent.com/linuxdeploy/misc-plugins/master/gdb/linuxdeploy-plugin-gdb.sh" \
#		&& set -- "$@" --plugin gdb
#fi
runCmd chmod --verbose a+x ./*.AppImage ./*.sh

# Set useful variables for linuxdeploy
export ARCH=x86_64
export UPDATE_INFORMATION="gh-releases-zsync|${GITHUB_REPOSITORY//\//|}|${VERSION:-"continuous"}|CPU-X-*$ARCH.AppImage.zsync"
export VERBOSE=1
#export DEBUG=1
export DISABLE_COPYRIGHT_FILES_DEPLOYMENT=1

# Run linuxdeploy
runCmd mkdir --parents --verbose "$WORKSPACE/AppImage" && runCmd cd "$_"
runCmd "$BUNDLER" \
	--appdir="$APPDIR" \
	--desktop-file="$APPDIR/usr/share/applications/io.github.thetumultuousunicornofdarkness.cpu-x.desktop" \
	--verbosity=1 \
	"$@"
