// load_unload.cc
#include <hooks/hooks.h>
#include "library_common.h"
#include <string>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace isc::hooks;
namespace pt = boost::property_tree;

std::string param_url;
extern "C" {
int load(LibraryHandle& handle) {
    pt::ptree root;

    pt::read_json("/etc/kea/params.json", root);
    param_url = root.get<std::string>("url");
    if (param_url.empty())
        return 1;
    std::cout << "example.so loaded\n" << std::endl;
    return 0;
}
int unload() {
    return (0);
}
}
