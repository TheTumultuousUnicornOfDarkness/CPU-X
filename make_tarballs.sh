#!/bin/sh

# This script helps to generate tarballs CPU-X_vX.X.X_portable.tar.gz and CPU-X_vX.X.X_portable_noGTK.tar.gz.

DIR=/tmp/CPU-X
JOBS=-j2

make_release() {
	ssh $1 << EOF

[[ ! -d $DIR ]] && git clone https://github.com/X0rg/CPU-X $DIR
mkdir -pv $DIR/{,g}n_build
cd $DIR && VER=\$(git tag | tail -n1)

cd $DIR/gn_build && cmake -DCMAKE_BUILD_TYPE=Release -DEMBED=1 .. && make $JOBS
cd $DIR/n_build  && cmake -DCMAKE_BUILD_TYPE=Release -DEMBED=1 -DWITH_GTK=0 .. && make $JOBS

[[ $1 == "Arch32" ]] && ARCH="linux32" || ARCH="linux64"
cp -v $DIR/gn_build/accomplished/bin/cpu-x "/mnt/Echange/CPU-X_\${VER}_portable.\$ARCH"
cp -v $DIR/n_build/accomplished/bin/cpu-x  "/mnt/Echange/CPU-X_\${VER}_portable_noGTK.\$ARCH"
EOF
}

VBoxManage list runningvms | grep -q "Arch Linux i686"   || (echo "Start 32-bit Linux VM" ; VBoxHeadless --startvm "Arch Linux i686" &)
VBoxManage list runningvms | grep -q "Arch Linux x86_64" || (echo "Start 64-bit Linux VM" ; VBoxHeadless --startvm "Arch Linux x86_64" &)
VBoxManage list runningvms | grep -q "GhostBSD" || (echo "Start 32-bit BSD VM" ; VBoxHeadless --startvm "GhostBSD" &)

while [[ `ssh Arch32 true` ]] && [[ `ssh Arch64 true` ]] && [[ `ssh BSD true` ]]; do
	echo "Waiting for start..."
	sleep 1
done

make_release Arch32
make_release Arch64
make_release BSD
sshfs BSD:/tmp/CPU-X/ ~/.VirtualBox/Echange/SSHFS

ssh Arch32 sudo poweroff
ssh Arch64 sudo poweroff
ssh BSD poweroff

cd ~/.VirtualBox/Echange/SSHFS
VER=$(git tag | tail -n1)
cd ..
cp -v SSHFS/gn_build/accomplished/bin/cpu-x "./CPU-X_${VER}_portable.bsd32"
cp -v SSHFS/n_build/accomplished/bin/cpu-x  "./CPU-X_${VER}_portable_noGTK.bsd32"
fusermount -u SSHFS

tar -zcvf CPU-X_${VER}_portable.tar.gz CPU-X_${VER}_portable.*
tar -zcvf CPU-X_${VER}_portable_noGTK.tar.gz CPU-X_${VER}_portable_noGTK.*
