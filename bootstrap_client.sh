#!/usr/bin/env bash

# Prepare to install Wireshark
sudo apt-get update
sudo add-apt-repository ppa:wireshark-dev/stable -y
sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get -y install wireshark-gtk
sudo addgroup -system wireshark
sudo chown root:wireshark /usr/bin/dumpcap
sudo setcap cap_net_raw,cap_net_admin=eip /usr/bin/dumpcap
sudo usermod -a -G wireshark $USER

# Install other tools
sudo apt-get -y install nmap
sudo apt-get -y install traceroute
sudo apt-get -y install whois
