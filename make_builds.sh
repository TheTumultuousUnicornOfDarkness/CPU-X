#!/bin/bash
# This script helps to track build troubles and make portable versions

VER=$(git tag | tail -n1)
SRCDIR=/tmp/CPU-X
DESTDIR="$(dirname $0)/portable"
JOBS=-j2
VMs=("Arch32"  "Arch64"  "BSD32" "BSD64")
EXT=("linux32" "linux64" "bsd32" "bsd64")


#########################################################
#			FUNCTIONS			#
#########################################################

make_build() {
	while ! $(ssh -q $1 exit); do
		sleep 1
	done

	[[ $1 == "Arch"* ]] && (ssh $1 ls /usr/lib/{libncursesw.a,libcpuid.a,libpci.a,libprocps.a} || exit 2)
	ssh $1 << EOF

makeopts() {
	if make $JOBS; then
		echo -e "\n\t\033[1;42m*** Build passed for $1 ***\033[0m\n\n"
		sleep 2
	else
		echo -e "\n\t\033[1;41m*** Build failed for $1 ***\033[0m\n\n"
		sleep 10
	fi
}

[[ ! -d $SRCDIR ]] && git clone https://github.com/X0rg/CPU-X $SRCDIR || (cd $SRCDIR && git pull)
mkdir -pv $SRCDIR/{,e}build{1..9}

echo -e "\n\n\033[1;44m*** Start normal build for $1\033[0m\n"
cd $SRCDIR/build1 && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr                    .. && makeopts
cd $SRCDIR/build2 && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DWITH_GTK=0       .. && makeopts
cd $SRCDIR/build3 && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DWITH_NCURSES=0   .. && makeopts
cd $SRCDIR/build4 && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DWITH_GETTEXT=0   .. && makeopts
cd $SRCDIR/build5 && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DWITH_LIBCPUID=0  .. && makeopts
cd $SRCDIR/build6 && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DWITH_LIBPCI=0    .. && makeopts
cd $SRCDIR/build7 && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DWITH_LIBSYSTEM=0 .. && makeopts
cd $SRCDIR/build8 && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DWITH_DMIDECODE=0 .. && makeopts
cd $SRCDIR/build9 && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DWITH_BANDWIDTH=0 .. && makeopts

sleep 5
echo -e "\n\n\033[1;44m*** Start portable build for $1\033[0m\n"
cd $SRCDIR/ebuild1 && cmake -DCMAKE_BUILD_TYPE=Debug -DPORTABLE_BINARY=1                    .. && makeopts
cd $SRCDIR/ebuild2 && cmake -DCMAKE_BUILD_TYPE=Debug -DPORTABLE_BINARY=1 -DWITH_GTK=0       .. && makeopts
cd $SRCDIR/ebuild3 && cmake -DCMAKE_BUILD_TYPE=Debug -DPORTABLE_BINARY=1 -DWITH_NCURSES=0   .. && makeopts
cd $SRCDIR/ebuild4 && cmake -DCMAKE_BUILD_TYPE=Debug -DPORTABLE_BINARY=1 -DWITH_GETTEXT=0   .. && makeopts
cd $SRCDIR/ebuild5 && cmake -DCMAKE_BUILD_TYPE=Debug -DPORTABLE_BINARY=1 -DWITH_LIBCPUID=0  .. && makeopts
cd $SRCDIR/ebuild6 && cmake -DCMAKE_BUILD_TYPE=Debug -DPORTABLE_BINARY=1 -DWITH_LIBPCI=0    .. && makeopts
cd $SRCDIR/ebuild7 && cmake -DCMAKE_BUILD_TYPE=Debug -DPORTABLE_BINARY=1 -DWITH_LIBSYSTEM=0 .. && makeopts
cd $SRCDIR/ebuild8 && cmake -DCMAKE_BUILD_TYPE=Debug -DPORTABLE_BINARY=1 -DWITH_DMIDECODE=0 .. && makeopts
cd $SRCDIR/ebuild9 && cmake -DCMAKE_BUILD_TYPE=Debug -DPORTABLE_BINARY=1 -DWITH_BANDWIDTH=0 .. && makeopts
EOF
}

make_release() {
	while ! $(ssh -q $1 exit); do
		sleep 1
	done

	[[ $1 == "Arch"* ]] && (ssh $1 ls /usr/lib/{libncursesw.a,libcpuid.a,libpci.a,libprocps.a} || exit 2)
	ssh $1 << EOF

makeopts() {
	if make $JOBS > /dev/null ; then
		echo -e "\n\t\033[1;42m*** Build passed for $1 ***\033[0m\n\n"
	else
		echo -e "\n\t\033[1;41m*** Build failed for $1 ***\033[0m\n\n"
		exit
	fi
}

[[ ! -d $SRCDIR ]] && git clone https://github.com/X0rg/CPU-X $SRCDIR || (cd $SRCDIR && git pull)
mkdir -pv $SRCDIR/{,g}n_build

cd $SRCDIR/gn_build && cmake -DCMAKE_BUILD_TYPE=Release -DPORTABLE_BINARY=1              .. > /dev/null && makeopts
cd $SRCDIR/n_build  && cmake -DCMAKE_BUILD_TYPE=Release -DPORTABLE_BINARY=1 -DWITH_GTK=0 .. > /dev/null && makeopts
EOF
}

stop_vms() {
	for i in {0..3}; do
		ssh ${VMs[i]} sudo poweroff
	done
}

help() {
	echo -e "Usage: $0 OPTION"
	echo -e "Options:"
	echo -e "\t-b, --build\tStart multiples builds to find build troubles"
	echo -e "\t-r, --release\tBuild portable versions when a new version is tagged"
	echo -e "\t-h, --help\tDisplay this help and exit"
}


#########################################################
#			MAIN				#
#########################################################

if [[ $# < 1 ]]; then
	help
	exit 1
else
	case "$1" in
		-b|--build)	choice="build";;
		-r|--release)	choice="release";;
		-h|--help)	help; exit 0;;
		- |--)		help; exit 1;;
		*)		help; exit 1;;
	esac
fi

if [[ $choice == "build" ]]; then
	# Start VMs
	VBoxManage list runningvms | grep -q "Arch Linux i686"   || (echo "Start 32-bit Linux VM" ; VBoxHeadless --startvm "Arch Linux i686" &)
	VBoxManage list runningvms | grep -q "GhostBSD i386"     || (echo "Start 32-bit BSD VM"   ; VBoxHeadless --startvm "GhostBSD i386" &)
	sleep 1

	# Start build
	make_build Arch32
	echo "Press a key to continue..." ; read
	make_build BSD32

elif [[ $choice == "release" ]]; then
	# Start VMs
	VBoxManage list runningvms | grep -q "Arch Linux i686"   || (echo "Start 32-bit Linux VM" ; VBoxHeadless --startvm "Arch Linux i686" &)
	VBoxManage list runningvms | grep -q "Arch Linux x86_64" || (echo "Start 64-bit Linux VM" ; VBoxHeadless --startvm "Arch Linux x86_64" &)
	VBoxManage list runningvms | grep -q "GhostBSD i386"     || (echo "Start 32-bit BSD VM"   ; VBoxHeadless --startvm "GhostBSD i386" &)
	VBoxManage list runningvms | grep -q "GhostBSD x86_64"   || (echo "Start 64-bit BSD VM"   ; VBoxHeadless --startvm "GhostBSD x86_64" &)
	sleep 1

	# Start build
	[[ -d "$DESTDIR" ]] && rm -rf "$DESTDIR"
	mkdir -pv "$DESTDIR/sshfs"
	for i in {0..3}; do
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
else
	echo "Bad option."
fi

echo "Shutdown VMs (y/N)?"
read -n1 s
[[ $s == "y" ]] && stop_vms
