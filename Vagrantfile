# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|

  config.ssh.insert_key = false
  config.vbguest.auto_update = false

  # Servidor de Peers (UDP)
  config.vm.define "udpserver" do |udp_config|
    udp_config.vm.box = "ubuntu/trusty64"
    udp_config.vm.hostname = "udpserver"
    udp_config.vm.network "private_network", ip: "192.168.56.21"
    # Porta UDP do servidor de peers (58000)
    udp_config.vm.network "forwarded_port", guest: 58000, host: 58000, protocol: "udp"
    udp_config.vm.provider "virtualbox" do |vb|
      vb.name = "udpserver"
      opts = ["modifyvm", :id, "--natdnshostresolver1", "on"]
      vb.customize opts
      vb.memory = "512"
    end
    udp_config.vm.provision "shell", path: "bootstrap_server.sh"
  end

  # Cliente Peer (TCP)
  config.vm.define "client1" do |client1_config|
    client1_config.vm.box = "ubuntu/trusty64"
    client1_config.vm.hostname = "client1"
    client1_config.vm.network "private_network", ip: "192.168.56.11"
    client1_config.vm.provider "virtualbox" do |vb|
      vb.name = "client1"
      opts = ["modifyvm", :id, "--natdnshostresolver1", "on"]
      vb.customize opts
      vb.memory = "512"
    end
    client1_config.vm.provision "shell", path: "bootstrap_client.sh"
  end
end
