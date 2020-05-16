#!/bin/bash

die() {
	echo "Error: $*"
	exit 1
}

detach() {
	local distro="$1"
	local arch="$2"
	local descr="$3"
	local pkgs="$4"
	local logfile="$5"

	echo -e "\n\033[1m==> Build for $distro $arch\033[0m"
	if time nohup osc build "$distro" "$arch" "$descr" \
		--trust-all-projects --clean --ccache --local-package \
		--keep-pkgs="$pkgs" --prefer-pkgs="$pkgs" \
		--no-debug-packages --jobs=2 > "$logfile"
	then
		color="\033[1;32m"
	else
		color="\033[1;31m"
	fi
	echo -e "\n$color==> End of $distro $arch\033[0m\n$(tail -n10 "$logfile")"
}

if [[ $# -lt 2 ]]; then
	echo "$0: OBS_DIR PACKAGE_NAME"
	exit 1
fi

OLD_IFS="$IFS"
OBS_DIR="$1"
PACKAGE="$2"
PKGS_DIR="$OBS_DIR/pkgs"
cd "$OBS_DIR/$PACKAGE" || die "failed to change directory to '$PACKAGE'"
mkdir -pv "$PKGS_DIR"

IFS=$'\n'
repos="${REPOS:-"$(osc repos)"}"
for repo in $repos; do
	IFS="$OLD_IFS"
	array=($repo)
	distro="${array[0]}"
	arch="${array[1]}"
	pkgs="$PKGS_DIR/$distro"
	logfile=".osc/_buildlog-$distro.$arch.log"
	case "$distro" in
		Debian*|xUbuntu_*) descr="$PACKAGE.dsc";;
		Fedora*|openSUSE*) descr="$PACKAGE.spec";;
		Arch)              descr="PKGBUILD";;
	esac
	detach "$distro" "$arch" "$descr" "$pkgs" "$logfile" &
	sleep 3 # Workaround to avoid issues with _service files
done

echo -e "\n\033[1m==> Wait for processes:\033[0m"
jobs -p
wait

echo -e "\n\033[1mAll done!\033[0m"
