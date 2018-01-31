#!/bin/bash
# This script helps to track build troubles and make portable versions

VER=$(git tag | tail -n1)
SRCDIR=/tmp/CPU-X
DESTDIR="$(dirname $0)/portable"
VMs=("Arch32"  "Arch64"  "BSD32" "BSD64")
EXT=("linux32" "linux64" "bsd32" "bsd64")


#########################################################
#			FUNCTIONS			#
#########################################################

wait_for_vm_up() {
	while ! $(ssh -q $1 exit); do
		sleep 1
	done
}

check_deps() {
	if [[ "$1" == "Arch"* ]]; then
		LIBS="/usr/lib/{libncursesw.a,libcurl.a,libssl.a,libcrypto.a,libarchive.a,libcpuid.a,libpci.a,libprocps.a}"
		wait_for_vm_up $1
		if ! ssh $1 ls $LIBS > /dev/null; then
			echo -e "\033[1;31mMissing static librairies. Aborted."
			exit 255
		fi
	fi
}

make_build() {
	wait_for_vm_up $1
	ssh $1 << EOF
_cmakeopts_install() {
	cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -GNinja \$@ .. > /dev/null
}

_cmakeopts_portable() {
	cmake -DCMAKE_BUILD_TYPE=Debug -DPORTABLE_BINARY=1 -GNinja \$@ .. > /dev/null
}

_makeopts() {
	if ninja; then
		echo -e "\n\t\033[1;42m*** Build passed for $1 ***\033[0m\n\n"
		sleep 2
	else
		echo -e "\n\t\033[1;41m*** Build failed for $1 ***\033[0m\n\n"
		sleep 10
	fi
}

[[ ! -d $SRCDIR ]] && git clone https://github.com/X0rg/CPU-X $SRCDIR || (cd $SRCDIR && git pull)
mkdir -pv $SRCDIR/{,e}build{0..9}

echo -e "\n\n\033[1;44m*** Start normal build for $1\033[0m\n"
cd $SRCDIR/build0 && _cmakeopts_install                    && _makeopts
cd $SRCDIR/build1 && _cmakeopts_install -DWITH_GTK=0       && _makeopts
cd $SRCDIR/build2 && _cmakeopts_install -DWITH_NCURSES=0   && _makeopts
cd $SRCDIR/build3 && _cmakeopts_install -DWITH_GETTEXT=0   && _makeopts
cd $SRCDIR/build4 && _cmakeopts_install -DWITH_LIBCURL=0   && _makeopts
cd $SRCDIR/build5 && _cmakeopts_install -DWITH_LIBCPUID=0  && _makeopts
cd $SRCDIR/build6 && _cmakeopts_install -DWITH_LIBPCI=0    && _makeopts
cd $SRCDIR/build7 && _cmakeopts_install -DWITH_LIBPROCPS=0 -DWITH_LIBSTATGRAB=0 .. && _makeopts
cd $SRCDIR/build8 && _cmakeopts_install -DWITH_DMIDECODE=0 && _makeopts
cd $SRCDIR/build9 && _cmakeopts_install -DWITH_BANDWIDTH=0 && _makeopts

sleep 5
echo -e "\n\n\033[1;44m*** Start portable build for $1\033[0m\n"
cd $SRCDIR/ebuild0 && _cmakeopts_portable                    .. && _makeopts
cd $SRCDIR/ebuild1 && _cmakeopts_portable -DWITH_GTK=0       .. && _makeopts
cd $SRCDIR/ebuild2 && _cmakeopts_portable -DWITH_NCURSES=0   .. && _makeopts
cd $SRCDIR/ebuild3 && _cmakeopts_portable -DWITH_GETTEXT=0   .. && _makeopts
cd $SRCDIR/ebuild4 && _cmakeopts_portable -DWITH_LIBCURL=0   .. && _makeopts
cd $SRCDIR/ebuild5 && _cmakeopts_portable -DWITH_LIBCPUID=0  .. && _makeopts
cd $SRCDIR/ebuild6 && _cmakeopts_portable -DWITH_LIBPCI=0    .. && _makeopts
cd $SRCDIR/ebuild7 && _cmakeopts_portable -DWITH_LIBPROCPS=0 -DWITH_LIBSTATGRAB=0 .. && _makeopts
cd $SRCDIR/ebuild8 && _cmakeopts_portable -DWITH_DMIDECODE=0 .. && _makeopts
cd $SRCDIR/ebuild9 && _cmakeopts_portable -DWITH_BANDWIDTH=0 .. && _makeopts
EOF
}

make_release() {
	wait_for_vm_up $1
	ssh $1 << EOF
_cmakeopts_portable() {
	cmake -DCMAKE_BUILD_TYPE=Release -DPORTABLE_BINARY=1 -GNinja \$@ .. > /dev/null
}

_makeopts() {
	if ninja; then
		echo -e "\n\t\033[1;42m*** Build passed for $1 ***\033[0m\n\n"
	else
		echo -e "\n\t\033[1;41m*** Build failed for $1 ***\033[0m\n\n"
		exit
	fi
}

[[ ! -d $SRCDIR ]] && git clone https://github.com/X0rg/CPU-X $SRCDIR || (cd $SRCDIR && git pull)
mkdir -pv $SRCDIR/{,g}n_build

cd $SRCDIR/gn_build && _cmakeopts_portable              .. && _makeopts
cd $SRCDIR/n_build  && _cmakeopts_portable -DWITH_GTK=0 .. && _makeopts
EOF
}

stop_vms() {
	echo "Shutdown VMs (y/N)?"
	read -n1 s

	if [[ $s == "y" ]]; then
		for i in {0..3}; do
			ssh ${VMs[i]} sudo poweroff
		done
	fi
}

help() {
	echo -e "Usage: $0 OPTION"
	echo -e "Options:"
	echo -e "\t-b, --build\tStart multiples builds to find build troubles"
	echo -e "\t-r, --release\tBuild portable versions when a new version is tagged"
	echo -e "\t-p, --package\tMake tarballs which contain packages"
	echo -e "\t-s, --shutdown\tStop all virtual machines"
	echo -e "\t-h, --help\tDisplay this help and exit"
}


#########################################################
#			MAIN				#
#########################################################

if [[ $# < 1 ]]; then
	help
	exit 1
fi

case "$1" in
	-b|--build)	choice="build";;
	-r|--release)	choice="release";;
	-p|--package)   choice="package";;
	-s|--shutdown)  stop_vms; exit 0;;
	-h|--help)	help; exit 0;;
	- |--)		help; exit 1;;
	*)		help; exit 1;;
esac

case "$choice" in
	build)
		# Start VMs
		VBoxManage list runningvms | grep -q "Arch Linux i686"   || (echo "Start 32-bit Linux VM" ; VBoxHeadless --startvm "Arch Linux i686" &)
		VBoxManage list runningvms | grep -q "GhostBSD i386"     || (echo "Start 32-bit BSD VM"   ; VBoxHeadless --startvm "GhostBSD i386" &)

		# Start build
		check_deps Arch32
		make_build Arch32
		echo "Press a key to continue..." ; read
		make_build BSD32

		stop_vms
		;;

	release)
		# Start VMs
		VBoxManage list runningvms | grep -q "Arch Linux i686"   || (echo "Start 32-bit Linux VM" ; VBoxHeadless --startvm "Arch Linux i686" &)
		VBoxManage list runningvms | grep -q "Arch Linux x86_64" || (echo "Start 64-bit Linux VM" ; VBoxHeadless --startvm "Arch Linux x86_64" &)
		VBoxManage list runningvms | grep -q "GhostBSD i386"     || (echo "Start 32-bit BSD VM"   ; VBoxHeadless --startvm "GhostBSD i386" &)
		VBoxManage list runningvms | grep -q "GhostBSD x86_64"   || (echo "Start 64-bit BSD VM"   ; VBoxHeadless --startvm "GhostBSD x86_64" &)

		# Start build
		[[ -d "$DESTDIR" ]] && rm -rf "$DESTDIR"
		mkdir -pv "$DESTDIR/sshfs"
		for i in {0..3}; do
			check_deps ${VMs[i]}
			make_release ${VMs[i]}
			sshfs ${VMs[i]}:$SRCDIR "$DESTDIR/sshfs"
			cp -v "$DESTDIR/sshfs/gn_build/cpu-x" "$DESTDIR/CPU-X_${VER}_portable.${EXT[i]}"
			cp -v "$DESTDIR/sshfs/n_build/cpu-x"  "$DESTDIR/CPU-X_${VER}_portable_noGTK.${EXT[i]}"
			fusermount -u "$DESTDIR/sshfs"
		done

		# Make tarball
		cd "$DESTDIR"
		tar -zcvf CPU-X_${VER}_portable.tar.gz       CPU-X_${VER}_portable.*
		tar -zcvf CPU-X_${VER}_portable_noGTK.tar.gz CPU-X_${VER}_portable_noGTK.*

		stop_vms
		;;

	package)
		DESTDIR="$(dirname $0)/packages/"
		REPOURL="https://download.opensuse.org/repositories/home:/X0rg/"
		COMPRESS="tar -zcvf"

		mkdir -p "$DESTDIR" && cd "$DESTDIR"
		wget --no-parent --no-host-directories --cut-dirs=3 --quiet --show-progress --continue \
			--accept "*.pkg.tar.xz","*.rpm","*.deb" \
			--reject "*.src.rpm","ncurses-devel-openSUSE-Leap-workaround*" \
			--recursive "$REPOURL"
		find . -type d -empty -delete

		$COMPRESS CPU-X_${VER}_ArchLinux.tar.gz Arch*
		$COMPRESS CPU-X_${VER}_Debian.tar.gz    Debian*
		$COMPRESS CPU-X_${VER}_Fedora.tar.gz    Fedora*
		$COMPRESS CPU-X_${VER}_openSUSE.tar.gz  openSUSE*
		$COMPRESS CPU-X_${VER}_Ubuntu.tar.gz    xUbuntu*
		;;
esac
