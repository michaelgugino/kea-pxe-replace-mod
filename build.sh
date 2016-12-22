g++ -ggdb -I /usr/include/kea -L /usr/lib/kea/lib -fpic -shared -o kea-pxe-replace4.so \
  src/load_unload.cc src/pkt4_send.cc src/version.cc \
  -lkea-dhcpsrv -lkea-dhcp++ -lkea-hooks -lkea-log -lkea-util \
  -lkea-exceptions -lcurl
