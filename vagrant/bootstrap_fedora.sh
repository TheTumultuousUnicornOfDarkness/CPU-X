#!/bin/bash

# Install packages
sudo dnf update -y
sudo dnf group install -y "Fedora Workstation" --allowerasing
sudo dnf install -y polkit-gnome gnome-tweak-tool virtualbox-guest-additions flatpak-builder flatpak-module-tools
sudo usermod -a -G wheel,mock vagrant

# Autologin to GDM
sudo mkdir -pv "/etc/gdm/"
cat | sudo tee "/etc/gdm/custom.conf" << EOF
[daemon]
AutomaticLogin=vagrant
AutomaticLoginEnable=True
EOF
echo "X-GNOME-Autostart-enabled=false" | sudo tee -a "/etc/xdg/autostart/gnome-initial-setup-first-login.desktop"
sudo systemctl set-default graphical.target

# Vagrant user settings
localectl set-keymap "${HOST_LANG//[_.]*/}"
gsettings set org.gnome.desktop.input-sources sources '[]'
gsettings set org.gnome.desktop.screensaver lock-enabled false

# Flatpak
flatpak remote-add --if-not-exists --user flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak install -y flathub org.gnome.Platform//3.36 org.gnome.Sdk//3.36
