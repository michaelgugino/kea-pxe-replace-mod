// library_common.h
#ifndef LIBRARY_COMMON_H
#define LIBRARY_COMMON_H
#include <fstream>
#include <zmq.hpp>
// "Interesting clients" log file handle declaration.
extern std::fstream interesting;
extern zmq::context_t my_context;
extern zmq::socket_t my_socket;
