# kea-pxe-replace-mod
Kea module utilizing hooks api to replace pxe options (next server, file) via web request

## Build requirements
This software has been developed on Debian Stretch and Ubuntu 16.04.  This software is currently unstable, use at your own risk (as always).

PR's welcome.

apt-get install g++ libcurl4-gnutls-dev libboost-dev kea-dev


## Testing locally

You can create a veth pair:

ip link add veth0 type veth peer name veth1

ip l set veth0 up && ip l set veth1 up

ip a add dev veth0 192.0.2.1/24

dhclient -d -v veth1
