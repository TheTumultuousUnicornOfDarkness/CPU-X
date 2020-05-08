#!/bin/bash

if [[ $# -lt 3 ]]; then
	echo "$0: BUILD_TYPE SOURCE_DIRECTORY INSTALL_DIRECTORY"
	exit 1
fi

BUILD_TYPE="$1"
SRC_DIR="$2"
DST_DIR="$3"

echo "Add OBS repository"
curl http://download.opensuse.org/repositories/home:/X0rg/xUbuntu_16.04/Release.key | sudo apt-key add -
echo "deb http://download.opensuse.org/repositories/home:/X0rg/xUbuntu_16.04/ /" | sudo tee -a /etc/apt/sources.list
sudo apt-get update -qq

echo "Install packages"
sudo apt-get install -y --allow-unauthenticated -qq cmake ninja-build nasm gettext libgtk-3-dev libncursesw5-dev libcpuid-dev libpci-dev libprocps-dev libarchive-dev libgtk-3-0 libncursesw5 libcpuid14 libpci3 libprocps4 adwaita-icon-theme

echo "Run CMake"
cmake -S "$SRC_DIR" -B build -GNinja -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBEXECDIR=/usr/bin

echo "Build CPU-X"
cmake --build build

echo "Install CPU-X"
DESTDIR="$DST_DIR" ninja -C build install
