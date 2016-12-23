# kea-pxe-replace-mod
Kea module utilizing hooks api to replace pxe options (siaddr, option 66, 67) via web request.

# Usage
Install the compiled library somewhere sensible (such as /usr/local/lib/kea-pxe-replace4.so)

Configure /etc/kea/kea-pxe-replace4.conf

Please see the example config in the etc/ directory.  Each key is mandatory,
however the value can be a value of your choosing. These refer to the fields
in the json response that this module will use for each parameter; this is so
you can configure an existing API service to interact with the module.  Nested
fields should be dot (.) separated.  IE: 'myserver.params.siaddr'.

For the response, the field does not have to be present.  If the field is not
present, it will not attempt to override the value.

These values are only applied to fields already set.  If next-server is not defined
somewhere in your kea config, or otherwise not set during packet processing, it
will not be overridden.  The same logic applies to the option fields; they are
only updated if they are set in the first place.

Configure /etc/kea/kea-dhcp4.conf

Will result in requesting json from "url" in the form of "url+mac"
Example: http://myurl.com/aa:bb:cc:dd:ee:ff
Note:  a '/' is not inserted between the url and the mac address for you, you
must provide that in the url if necessary.

## Build requirements
This software has been developed on Debian Stretch and Ubuntu 16.04.  This
software is currently experimental, use at your own risk (as always).

apt-get install g++ libcurl4-gnutls-dev libboost-dev kea-dev

./build.sh

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
