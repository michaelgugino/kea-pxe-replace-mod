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
#include <io_address.h>

using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
using namespace isc::log;
namespace pt = boost::property_tree;


isc::log::Logger logger("pxe-replace-logger");
const char* log_messages[] = {
    "PRL_BASE", "message: %1",
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

    char *bp;
    size_t size;
    FILE *response_memfile;

    pt::ptree root;
    std::stringstream ss;
    std::string replacement_opt = "/dev/notnull";
    string hwaddr;
    OptionCollection 	options_collection;
    OptionPtr opt_ptr;

    // TODO: we should just get this info here instead of pkt_receive.
    handle.getContext("hwaddr", hwaddr);
    // TODO: Need to check to see if param_url ends in /, if not, append it.
    param_url.append(hwaddr);

    Pkt4Ptr response4_ptr;
    handle.getArgument("response4", response4_ptr);
    options_collection = response4_ptr->options_;
    opt_ptr = response4_ptr->getOption((uint16_t)67);
    LOG_INFO(logger, "PRL_BASE").arg(opt_ptr->toString());
    opt_ptr->setData(replacement_opt.begin(), replacement_opt.end());
    opt_ptr = response4_ptr->getOption((uint16_t)67);
    LOG_INFO(logger, "PRL_BASE").arg(opt_ptr->toString());
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
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
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response_memfile);
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
            fflush(response_memfile);
            // stringstream << FILE *
            ss << bp;

            // Load the json file in this ptree
            pt::read_json(ss, root);
            LOG_INFO(logger, "PRL_BASE").arg(root.get<std::string>("message"));

        }
        /* cleanup */
        curl_easy_cleanup(curl);
        fclose(response_memfile);
        free(bp);

        return(0);
    }
    return(1);
}
}
