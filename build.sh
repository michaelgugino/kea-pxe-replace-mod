g++ -ggdb -I /usr/include/kea -L /usr/lib/kea/lib -fpic -shared -o example.so \
  load_unload.cc pkt4_receive.cc pkt4_send.cc version.cc \
  -lkea-dhcpsrv -lkea-dhcp++ -lkea-hooks -lkea-log -lkea-util \
  -lkea-exceptions -lzmq -lcurl
