// load_unload.cc
#include <hooks/hooks.h>
#include "library_common.h"
#include <string>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace isc::hooks;
namespace pt = boost::property_tree;
// "Interesting clients" log file handle definition.
std::fstream interesting;
std::string param_url;
extern "C" {
int load(LibraryHandle& handle) {
    pt::ptree root;
    interesting.open("/data/clients/interesting.log",
                     std::fstream::out | std::fstream::app);
    pt::read_json("/etc/kea/params.json", root);
    param_url = root.get<std::string>("url");
    interesting << "example.so loaded\n" << std::endl;
    interesting << param_url << "\n";
    return (interesting ? 0 : 1);
}
int unload() {
    if (interesting) {
        interesting.close();
    }
    return (0);
}
}
