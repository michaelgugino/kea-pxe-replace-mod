// load_unload.cc
#include "library_common.h"

using namespace isc::hooks;
namespace pt = boost::property_tree;

std::string param_url;
std::string json_params[3];

extern "C" {
int load(LibraryHandle& handle) {
    pt::ptree root;
    pt::read_json("/etc/kea/kea-pxe-replace4.conf", root);

    json_params[0] = (root.get<std::string>("url"));
    json_params[1] = (root.get<std::string>("siaddr"));
    json_params[2] = (root.get<std::string>("tftp_server"));
    json_params[3] = (root.get<std::string>("bootfile_name"));

    std::cout << "example.so loaded\n" << std::endl;
    return 0;
}
int unload() {
    return (0);
}
}
