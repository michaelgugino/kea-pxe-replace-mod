// load_unload.cc
// Example to utilize zeromq
// Based on tutorial.

#include <hooks/hooks.h>
#include "library_common.h"
#include <string>
#include <iostream>

using namespace isc::hooks;
// Need to declare objects in top scope to be able to access them in later hook calls.
std::fstream interesting;
zmq::context_t my_context(1);
zmq::socket_t my_socket(my_context, ZMQ_REQ);

extern "C" {
int load(LibraryHandle&) {
    interesting.open("/data/clients/interesting.log",
                     std::fstream::out | std::fstream::app);
    interesting << "Connecting to hello world server…" << std::endl;
    
    // We should do some checking on my_socket before returning as well.
    my_socket.connect ("tcp://localhost:5555"); 
    return (interesting ? 0 : 1);
}
int unload() {
    if (interesting) {
        interesting.close();
        my_socket.close();
    }
    return (0);
}
}
