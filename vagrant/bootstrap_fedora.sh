#!/bin/bash

# Install packages
sudo dnf update -y
sudo dnf group install -y "Fedora Workstation" --allowerasing
sudo dnf install -y polkit-gnome gnome-tweak-tool virtualbox-guest-additions
sudo usermod -a -G wheel,mock vagrant

# Autologin to GDM
sudo mkdir -pv "/etc/gdm/"
cat | sudo tee "/etc/gdm/custom.conf" << EOF
[daemon]
AutomaticLogin=vagrant
AutomaticLoginEnable=True
EOF
sudo systemctl set-default graphical.target

# Vagrant user settings
localectl set-keymap "${HOST_LANG//[_.]*/}"
gsettings set org.gnome.desktop.input-sources sources '[]'
gsettings set org.gnome.desktop.screensaver lock-enabled false
echo "yes" > "/home/vagrant/.config/gnome-initial-setup-done"
