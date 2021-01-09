#!/bin/bash

if [[ $# -lt 2 ]]; then
	echo "$0: BUILD_TYPE SOURCE_DIRECTORY [INSTALL_DIRECTORY]"
	exit 1
fi

BUILD_TYPE="$1"
SRC_DIR="$2"
DST_DIR="$3"

echo "Add OBS repository"
echo 'deb http://download.opensuse.org/repositories/home:/Xorg/xUbuntu_16.04/ /' | sudo tee /etc/apt/sources.list.d/home:Xorg.list
curl -sSL https://download.opensuse.org/repositories/home:Xorg/xUbuntu_16.04/Release.key | sudo apt-key add -
sudo apt-get update -qq

echo "Install packages"
sudo apt-get install -y -qq cmake ninja-build nasm gettext libgtk-3-dev libncursesw5-dev libcpuid-dev-git libpci-dev libprocps-dev libgtk-3-0 libncursesw5 libcpuid15-git libpci3 libprocps4 adwaita-icon-theme

echo "Run CMake"
cmake -S "$SRC_DIR" -B build -GNinja -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBEXECDIR=/usr/bin -DAPPIMAGE=1

echo "Build CPU-X"
cmake --build build

echo "Install CPU-X"
if [[ -z "$DST_DIR" ]]; then
	sudo ninja -C build install
else
	DESTDIR="$DST_DIR" ninja -C build install
fi
