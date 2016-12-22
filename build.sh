g++ -ggdb -I /usr/include/kea -L /usr/lib/kea/lib -fpic -shared -o kea-pxe-replace4.so \
  load_unload.cc pkt4_send.cc version.cc \
  -lkea-dhcpsrv -lkea-dhcp++ -lkea-hooks -lkea-log -lkea-util \
  -lkea-exceptions -lcurl
