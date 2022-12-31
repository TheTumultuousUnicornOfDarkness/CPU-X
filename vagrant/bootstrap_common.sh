#!/bin/bash

source /etc/os-release

if [[ "$ID" == "ubuntu" ]]; then
	export DEBIAN_FRONTEND=noninteractive
	sed -i 's/1/0/g' /etc/apt/apt.conf.d/20auto-upgrades
	apt-get update -y
	apt-get upgrade -y
	apt-get install -y --no-install-recommends ubuntu-desktop unity-lens-applications unity-lens-files fonts-inconsolata gdm3 gnome-terminal firefox dbus-x11 console-data
	apt-get install -y --no-install-recommends virtualbox-guest-utils virtualbox-guest-x11

	echo "/usr/sbin/gdm3" > /etc/X11/default-display-manager
	systemctl enable --now gdm3.service
	DEBCONF_NONINTERACTIVE_SEEN=true dpkg-reconfigure gdm3
	echo "set shared/default-x-display-manager gdm3" | debconf-communicate

	GDM_CONFIG_DIR="/etc/gdm3"
	GDM_DEFAULT_SESSION="Ubuntu"
elif [[ "$ID" == "fedora" ]]; then
	dnf update -y
	dnf -y group install "Basic Desktop" GNOME --allowerasing
	dnf install -y levien-inconsolata-fonts firefox xdg-user-dirs dbus-x11 polkit-gnome gnome-tweak-tool virtualbox-guest-additions
	usermod -a -G wheel vagrant

	GDM_CONFIG_DIR="/etc/gdm"
	GDM_DEFAULT_SESSION="GNOME"
else
	echo "$ID not supported." > /dev/stderr
	exit 1
fi

# System settings
localectl set-keymap "${HOST_LANG//[_.]*/}"
if ! grep -q "127.0.0.1 $(hostname)" /etc/hosts; then
	echo "127.0.0.1 $(hostname)" >> /etc/hosts
fi

# Autologin to GDM
mkdir -pv "$GDM_CONFIG_DIR"
cat > "$GDM_CONFIG_DIR/custom.conf" << EOF
[daemon]
AutomaticLogin=vagrant
AutomaticLoginEnable=True
DefaultSession=$GDM_DEFAULT_SESSION
EOF
systemctl set-default graphical.target
systemctl restart gdm

# Vagrant user
su - vagrant -c "xdg-user-dirs-update --force"
su - vagrant -c "dbus-launch gsettings set org.gnome.desktop.input-sources sources '[]'"
su - vagrant -c "dbus-launch gsettings set org.gnome.desktop.screensaver lock-enabled false"
su - vagrant -c "dbus-launch gsettings set org.gnome.desktop.session idle-delay 0"
su - vagrant -c "dbus-launch gsettings set org.gnome.desktop.lockdown disable-lock-screen true"
su - vagrant -c "dbus-launch gsettings set org.gnome.desktop.interface monospace-font-name 'Inconsolata Regular 12'"
su - vagrant -c "echo yes > /home/vagrant/.config/gnome-initial-setup-done"
