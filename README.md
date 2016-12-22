# kea-pxe-replace-mod
Kea module utilizing hooks api to replace pxe options (next server, file) via web request

# Usage
Install the compiled library somewhere sensible (such as /usr/local/lib/kea-pxe-replace4.so)

Configure /etc/kea/kea-pxe-replace4.conf

Configure /etc/kea/kea-dhcp4.conf

## Build requirements
This software has been developed on Debian Stretch and Ubuntu 16.04.  This
software is currently experimental, use at your own risk (as always).

apt-get install g++ libcurl4-gnutls-dev libboost-dev kea-dev


## Testing locally

You can create a veth pair:

ip link add veth0 type veth peer name veth1

ip l set veth0 up && ip l set veth1 up

ip a add dev veth0 192.0.2.1/24

dhclient -d -v veth1

## Testing with libvirt locally
You probably need to kill the dnsmasq agents that are running.

brctl addbr brx && brctl addif brx veth1

virt-install --pxe --network bridge=brx --name pxe1 --memory 128 --disk none

Also checkout dhcpdump!
