#!/bin/bash
# This script is used to create archives for releases

set -euo pipefail

GIT_DIR="$(git rev-parse --show-toplevel)"
VER=$(git describe --tags --exclude continuous | cut -d- -f1)


#########################################################
#                    FUNCTIONS                          #
#########################################################




help() {
	echo -e "Usage: $0 OPTION"
	echo -e "Options:"
	echo -e "\t-p, --package\tMake tarballs which contain packages"
	echo -e "\t    --local\tBuild packages locally (with --package)"
	echo -e "\t-h, --help\tDisplay this help and exit"
}


#########################################################
#                    MAIN                               #
#########################################################

if [[ $# < 1 ]]; then
	help
	exit 1
fi

case "$1" in
	-p|--package)   choice="package";;
	-h|--help)      help; exit 0;;
	- |--)          help; exit 1;;
	*)              help; exit 1;;
esac

case "$choice" in
	package)
		REPO_URL="https://download.opensuse.org/repositories/home:/Xorg/"
		OBS_DIR="$(realpath "$GIT_DIR/../OBS")"
		PKGS_DIR="$OBS_DIR/pkgs"
		ARCHIVES_DIR="$GIT_DIR/packages"
		COMPRESS="tar -zcvf"

		if [[ "$2" == "--local" ]] && [[ ! -d "$PKGS_DIR" ]]; then
			"$GIT_DIR/scripts/osc_build.sh" "$OBS_DIR" "libcpuid"
			"$GIT_DIR/scripts/osc_build.sh" "$OBS_DIR" "cpu-x"
			cd "$PKGS_DIR"
		else
			mkdir -p "$PKGS_DIR" && cd "$_"
			wget --no-parent --no-host-directories --cut-dirs=3 --quiet --show-progress --continue \
			--accept "*.pkg.tar.zst","*.rpm","*.deb" \
			--reject "*.src.rpm","*git*" \
			--recursive "$REPO_URL"
		fi

		find "$PKGS_DIR" -type f -not -name '*.deb' -and -not -name '*.rpm' -and -not -name '*.pkg.tar.*' -delete
		find "$PKGS_DIR" -type d -empty -delete
		mkdir -p "$ARCHIVES_DIR"

		$COMPRESS "$ARCHIVES_DIR/CPU-X_${VER}_ArchLinux.tar.gz" Arch*
		$COMPRESS "$ARCHIVES_DIR/CPU-X_${VER}_Debian.tar.gz"    Debian*
		$COMPRESS "$ARCHIVES_DIR/CPU-X_${VER}_openSUSE.tar.gz"  openSUSE*
		$COMPRESS "$ARCHIVES_DIR/CPU-X_${VER}_Ubuntu.tar.gz"    xUbuntu*
		;;
esac
