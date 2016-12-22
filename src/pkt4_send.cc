#include <dhcp/pkt4.h>
#include <log/logger.h>
#include <log/macros.h>
#include <log/message_initializer.h>
#include "library_common.h"
#include <curl/curl.h>
#include <sstream>

using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
using namespace isc::log;
namespace pt = boost::property_tree;


isc::log::Logger logger("pxe-replace-logger");
const char* log_messages[] = {
    "PRL_BASE", "message: %1",
    "PRL_PKT_SEND", "Outgoing packet: \n%1",
    "PRL_CURL_RECEIVED", "Received: \n%1",
    "PRL_CURL_FAILED", "Unable to receive data from %1",
    "PRL_REPLACE_FIELD", "Replacing: %1 with %2",
    "PRL_NO_SIADDR", "siaddr field not being replaced, this is probably an error.",
    NULL
};

/// Initializer for log messages.
const MessageInitializer message_initializer(log_messages);

extern "C" {
int pkt4_send(CalloutHandle& handle) {

    CURL *curl;
    CURLcode res;
    struct curl_slist *list=NULL;
    bool perform_updates = 0;
    int curl_opt_res = 0;

    char *bp;
    size_t size;
    FILE *response_memfile;

    pt::ptree root;
    std::stringstream ss;
    string hwaddr;
    string final_url;
    //OptionCollection 	options_collection;
    //options_collection = response4_ptr->options_;
    OptionPtr tftp_server_name_opt_ptr;
    OptionPtr bootfile_name_opt_ptr;
    boost::optional<std::string> siaddr_json_field;
    boost::optional<std::string> tftp_server_json_field;
    boost::optional<std::string> bootfile_json_field;

    Pkt4Ptr response4_ptr;
    handle.getArgument("response4", response4_ptr);
    HWAddrPtr hwaddr_ptr = response4_ptr->getHWAddr();
    isc::asiolink::IOAddress orig_siaddr(response4_ptr->getSiaddr());
    hwaddr = hwaddr_ptr->toText(false);
    //json_params[0].append(hwaddr);
    final_url = json_params[0] + hwaddr;
    LOG_DEBUG(logger, 0, "PRL_BASE").arg(final_url);

    tftp_server_name_opt_ptr = response4_ptr->getOption((uint16_t)66);
    bootfile_name_opt_ptr = response4_ptr->getOption((uint16_t)67);

    if (tftp_server_name_opt_ptr != NULL || bootfile_name_opt_ptr != NULL
        || orig_siaddr.toText() != "0.0.0.0")
        perform_updates = 1;

    if (!perform_updates) {
        LOG_WARN(logger, "PRL_BASE").arg("Nothing to update.");
        return(0);
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(!curl) {
        curl_global_cleanup();
        LOG_ERROR(logger, "PRL_BASE").arg("Could not initialize curl");
        return(1);
    }

    list = curl_slist_append(list, "Accept: application/json");
    if (list == NULL) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        LOG_ERROR(logger, "PRL_BASE").arg("Could not create curl slist.");
        return(1);
    }

    response_memfile = open_memstream (&bp, &size);
    if (response_memfile == NULL) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        LOG_ERROR(logger, "PRL_BASE").arg("Could not create memfile.");
        return(1);
    }

    // If we don't set a timeout, curl will try for 300 seconds by default.
    curl_opt_res += curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);
    curl_opt_res += curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1L);
    // libcurl's docs say to cast as void, don't blame me.
    curl_opt_res += curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response_memfile);
    // CURLOPT_URL takes a char*
    curl_opt_res += curl_easy_setopt(curl, CURLOPT_URL, (final_url).c_str());
    curl_opt_res += curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    #ifdef SKIP_PEER_VERIFICATION
    curl_opt_res += curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    #endif

    #ifdef SKIP_HOSTNAME_VERIFICATION
    curl_opt_res += curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    #endif
    if (curl_opt_res > 0) {
      fclose(response_memfile);
      free(bp);
      curl_easy_cleanup(curl);
      curl_global_cleanup();
      LOG_ERROR(logger, "PRL_BASE").arg("Error setting curl options.");
      return(1);
    }
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK) {
        fclose(response_memfile);
        free(bp);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        LOG_ERROR(logger, "PRL_CURL_FAILED").arg(ss.str());
        return(1);
    }
    // make bp available for reading.
    fclose(response_memfile);
    // stringstream << FILE *
    ss << bp;
    free(bp);
    LOG_DEBUG(logger, 0, "PRL_CURL_RECEIVED").arg(ss.str());
    // Load the json file in this ptree
    // TODO: this might throw an error if json invalid.
    pt::read_json(ss, root);

    // Fetch values from fieldnames defined in kea-pxe-replace.conf
    siaddr_json_field = root.get_optional<std::string>(json_params[1]);
    tftp_server_json_field = root.get_optional<std::string>(json_params[2]);
    bootfile_json_field = root.get_optional<std::string>(json_params[3]);

    if (orig_siaddr.toText() != "0.0.0.0" && siaddr_json_field) {
        LOG_DEBUG(logger, 0, "PRL_REPLACE_FIELD").arg("siaddr").arg(*siaddr_json_field);
        isc::asiolink::IOAddress new_siaddr(*siaddr_json_field);
        response4_ptr->setSiaddr(new_siaddr);
    }
    else
        LOG_WARN(logger, "PRL_NO_SIADDR");

    if (tftp_server_name_opt_ptr != NULL && tftp_server_json_field) {
        LOG_DEBUG(logger, 0, "PRL_REPLACE_FIELD").arg("tftp_server_name").arg(*tftp_server_json_field);
        tftp_server_name_opt_ptr->setData((*tftp_server_json_field).begin(), (*tftp_server_json_field).end());
    }
    if (bootfile_name_opt_ptr != NULL && bootfile_json_field) {
        LOG_DEBUG(logger, 0, "PRL_REPLACE_FIELD").arg("bootfile_name").arg(*bootfile_json_field);
        bootfile_name_opt_ptr->setData((*bootfile_json_field).begin(), (*bootfile_json_field).end());
    }

    LOG_DEBUG(logger, 0, "PRL_PKT_SEND").arg(response4_ptr->toText());

    /* cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return(0);
}
}
