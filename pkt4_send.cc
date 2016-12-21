#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include "library_common.h"
#include <string>
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <log/logger.h>
#include <log/macros.h>
#include <log/message_initializer.h>
//#include <io_address.h>

using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
using namespace isc::log;
namespace pt = boost::property_tree;


isc::log::Logger logger("pxe-replace-logger");
const char* log_messages[] = {
    "PRL_BASE", "message: %1",
    "PRL_PKT_SEND", "Outgoing packet: \n%1",
    NULL
};

/// @brief Initializer for log messages.
const MessageInitializer message_initializer(log_messages);

extern "C" {
// This callout is called at the "pkt4_send" hook.
int pkt4_send(CalloutHandle& handle) {

    CURL *curl;
    CURLcode res;
    struct curl_slist *list=NULL;
    bool perform_updates = 0;

    char *bp;
    size_t size;
    FILE *response_memfile;

    pt::ptree root;
    std::stringstream ss;
    std::string replacement_opt = "/dev/notnull";
    string hwaddr;
    //OptionCollection 	options_collection;
    OptionPtr tftp_server_name_opt_ptr;
    OptionPtr bootfile_name_opt_ptr;

    // TODO: Need to check to see if param_url ends in /, if not, append it.
    param_url.append(hwaddr);

    Pkt4Ptr response4_ptr;
    handle.getArgument("response4", response4_ptr);
    HWAddrPtr hwaddr_ptr = response4_ptr->getHWAddr();
    hwaddr = hwaddr_ptr->toText(false);

    //options_collection = response4_ptr->options_;
    tftp_server_name_opt_ptr = response4_ptr->getOption((uint16_t)66);
    bootfile_name_opt_ptr = response4_ptr->getOption((uint16_t)67);

    // TODO: need to check siaddr as well in future.
    if (tftp_server_name_opt_ptr != NULL || bootfile_name_opt_ptr != NULL)
        perform_updates = 1;

    /* End Testing */
    if (!perform_updates) {
        LOG_INFO(logger, "PRL_BASE").arg("Nothing to update.");
        return(0);
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(!curl) {
        LOG_ERROR(logger, "PRL_BASE").arg("Could not initialize curl");
        curl_global_cleanup();
        return(1);
    }

    list = curl_slist_append(list, "Accept: application/json");
    if (list == NULL) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 1;
    }

    response_memfile = open_memstream (&bp, &size);
    if (response_memfile == NULL) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 1;
    }

    // TODO: error check the things.
    // If we don't set a timeout, curl will try for 300 seconds by default.
    // if unable to connect.
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1L);
    // libcurl's docs say to cast as void, don't blame me.
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response_memfile);
    // CURLOPT_URL takes a char*
    curl_easy_setopt(curl, CURLOPT_URL, param_url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    #ifdef SKIP_PEER_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    #endif

    #ifdef SKIP_HOSTNAME_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    #endif

    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK) {
        // Log something.
    }
    else {
        // make bp available for reading.
        fflush(response_memfile);
        // stringstream << FILE *
        ss << bp;
        // Load the json file in this ptree
        // TODO: this might throw an error if json invalid.
        pt::read_json(ss, root);
        // TODO: change to debug
        LOG_INFO(logger, "PRL_BASE").arg(root.get<std::string>("message"));

        boost::optional<std::string> bad_field = root.get_optional<std::string>("message");
        if (!bad_field)
            LOG_INFO(logger, "PRL_BASE").arg("empty field");
        else
            LOG_INFO(logger, "PRL_BASE").arg(*bad_field);
    }

    if (tftp_server_name_opt_ptr != NULL)
        tftp_server_name_opt_ptr->setData(replacement_opt.begin(), replacement_opt.end());
    if (bootfile_name_opt_ptr != NULL)
        bootfile_name_opt_ptr->setData(replacement_opt.begin(), replacement_opt.end());

    // TODO: change to debug
    LOG_INFO(logger, "PRL_PKT_SEND").arg(response4_ptr->toText());
    /* cleanup */
    curl_easy_cleanup(curl);
    fclose(response_memfile);
    free(bp);
    curl_global_cleanup();
    return(0);
}
}
