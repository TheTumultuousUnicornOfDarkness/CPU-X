#!/bin/bash
# shellcheck disable=SC2155

# Functions
die() {
	echo "[FATAL] $*"
	exit 1
}

safeExport() {
	local expr="$1"
	local var="$(echo "$expr" | cut -d= -f1)"
	local dirs="$(echo "$expr" | cut -d= -f2)"
	local ok=false

	for dir in ${dirs//:/$'\n'}; do
		if [[ -d "$dir" ]]; then
			ok=true
		else
			echo "[WARNING] Directory '$dir' does not exist (used by '$var')"
		fi
	done

	if $ok; then
		export "${expr?}"
	else
		die "'$var' use invalid paths"
	fi
}

cleanUp() {
	if [[ -n "$CACHEDIR" ]] && [[ -d "$CACHEDIR" ]]; then
		rm -rf "$CACHEDIR"
	fi
}

# Debug helpers
if [[ -n "$CPUX_APPIMAGE_DEBUG" ]]; then
	set -x
fi

if [[ -n "$CPUX_APPIMAGE_GDB" ]]; then
	EXEC="gdb --cd usr/bin --args"
else
	EXEC="exec"
fi

# Constant variables
gsettings get org.gnome.desktop.interface gtk-theme 2> /dev/null | grep -qi "dark" && GTK_THEME_VARIANT="dark" || GTK_THEME_VARIANT="light"
CACHEDIR="$(mktemp --tmpdir --directory CPU-X.XXXXXXXX)"
LIBARCHDIR="lib/x86_64-linux-gnu"
CPUX_GTK_THEME=${CPUX_GTK_THEME:-"Adwaita:$GTK_THEME_VARIANT"}

# Export lot of variables
safeExport APPDIR="${APPDIR:-"$(dirname "$(realpath "$0")")"}" # Workaround to run extracted AppImage
export     GTK_THEME="$CPUX_GTK_THEME"
export     GDK_BACKEND=x11 # Crash with Wayland backend on Wayland
safeExport GTK_DATA_PREFIX="$APPDIR"
safeExport GTK_EXE_PREFIX="$APPDIR/usr"
safeExport GTK_PATH="$APPDIR/usr/$LIBARCHDIR/gtk-3.0"
safeExport GTK_IM_MODULE_DIR="$APPDIR/usr/$LIBARCHDIR/gtk-3.0/3.0.0/immodules"
export     GTK_IM_MODULE_FILE="$CACHEDIR/immodules.cache"
safeExport GDK_PIXBUF_MODULEDIR="$APPDIR/usr/$LIBARCHDIR/gdk-pixbuf-2.0/2.10.0/loaders"
export     GDK_PIXBUF_MODULE_FILE="$CACHEDIR/loaders.cache"
safeExport GSETTINGS_SCHEMA_DIR="$APPDIR/usr/share/glib-2.0/schemas"
safeExport PANGO_LIBDIR="$APPDIR/usr/lib"
#safeExport PANGO_SYSCONFDIR="$APPDIR/etc"
#safeExport XDG_CONFIG_DIRS="$APPDIR/etc/xdg"
safeExport XDG_DATA_DIRS="$APPDIR/usr/share:/usr/share:$XDG_DATA_DIRS"
safeExport TEXTDOMAINDIR="$APPDIR/usr/share/locale"
safeExport TERMINFO="$APPDIR/usr/share/terminfo"
safeExport LD_LIBRARY_PATH="$APPDIR/usr/lib/:$APPDIR/usr/$LIBARCHDIR:$LD_LIBRARY_PATH:$GDK_PIXBUF_MODULEDIR"
safeExport PATH="$APPDIR/usr/bin:$PATH"

# Make cache to avoid warnings (note: paths are absolute so that is why we do that are runtime)
"$APPDIR/usr/$LIBARCHDIR/libgtk-3-0/gtk-query-immodules-3.0"      "$GTK_IM_MODULE_DIR/"*    > "$GTK_IM_MODULE_FILE"
"$APPDIR/usr/$LIBARCHDIR/gdk-pixbuf-2.0/gdk-pixbuf-query-loaders" "$GDK_PIXBUF_MODULEDIR/"* > "$GDK_PIXBUF_MODULE_FILE"

# Launch application
cd "$APPDIR" || die "failed to set current directory to '$APPDIR'"
trap cleanUp EXIT
$EXEC "$APPDIR/usr/bin/cpu-x" "$@"
