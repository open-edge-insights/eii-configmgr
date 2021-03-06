// Copyright (c) 2021 Intel Corporation.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eii/msgbus/msgbus.h"
#include "eii/utils/logger.h"
#include "eii/utils/json_config.h"
#include "eii/config_manager/cfgmgr.h"

int main(int argc, char** argv) {

    // Set log level
    set_log_level(LOG_LVL_DEBUG);

    // Fetching Publisher config from
    // VideoIngestion interface
    setenv("AppName", "VideoIngestion", 1);

    // Initializing ConfigMgr
    cfgmgr_ctx_t* cfg_mgr = cfgmgr_initialize();
    if (cfg_mgr == NULL) {
        LOG_ERROR_0("Failed to initialize cfg_mgr");
        return -1;
    }

    // Testing cfgmgr_is_dev_mode API
    bool dev_mode = cfgmgr_is_dev_mode(cfg_mgr);

    // Testing cfgmgr_get_appname API
    config_value_t* app_name = cfgmgr_get_appname(cfg_mgr);
    if (app_name == NULL) {
        LOG_ERROR_0("Failed to fetch app_name");
        return -1;
    }
    LOG_INFO("AppName is %s\n", app_name->body.string);

    // Testing cfgmgr_get_num_publishers API
    int num_publishers = cfgmgr_get_num_publishers(cfg_mgr);
    LOG_INFO("Number of publishers %d\n", num_publishers);

    // Fetching cfgmgr_interface_t for a publisher example
    cfgmgr_interface_t* publisher_interface = cfgmgr_get_publisher_by_index(cfg_mgr, 0);
    // cfgmgr_interface_t* publisher_interface = cfgmgr_get_publisher_by_name(cfg_mgr, "default");
    if (publisher_interface == NULL) {
        LOG_ERROR_0("Failed to fetch publisher_interface");
        return -1;
    }

    // Fetching msgbus config for publisher example
    // using the cfgmgr_interface_t obtained
    config_t* pub_config = cfgmgr_get_msgbus_config(publisher_interface);
    if (pub_config == NULL) {
        LOG_ERROR_0("Failed to fetch pub_config");
        return -1;
    }
    char* pub_config_char = configt_to_char(pub_config);
    LOG_INFO("Msgbus config for publisher is %s\n", pub_config_char);

    // Fetching EndPoint for publisher example
    // using the cfgmgr_interface_t obtained
    config_value_t* ep = cfgmgr_get_endpoint(publisher_interface);
    if (ep == NULL) {
        LOG_ERROR_0("Failed to fetch endpoint");
        return -1;
    }
    LOG_INFO("EndPoint for publisher is %s\n", ep->body.string);

    int topics_length = 3;
    char **topics_to_be_set = NULL;
    topics_to_be_set = (char**)calloc(topics_length, sizeof(char*));
    topics_to_be_set[0] = "camera1_stream";
    topics_to_be_set[1] = "camera2_stream";
    topics_to_be_set[2] = "camera3_stream";

    // Setting topics for publisher example
    bool topics_set = cfgmgr_set_topics(publisher_interface, topics_to_be_set, 3);

    // Fetching Topics for publisher example
    // using the cfgmgr_interface_t obtained
    config_value_t* topics = cfgmgr_get_topics(publisher_interface);
    if (topics == NULL) {
        LOG_ERROR_0("Failed to fetch topics");
        return -1;
    }
    config_value_t* topic_value;
    size_t arr_len = config_value_array_len(topics);
    if (arr_len == 0) {
        LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
    }
    for (size_t i = 0; i < arr_len; i++) {
        topic_value = config_value_array_get(topics, i);
        if (topic_value == NULL) {
            LOG_ERROR_0("topic_value initialization failed");
            config_value_destroy(topics);
        }
        LOG_INFO("topic value is %s\n", topic_value->body.string);
        // Destroying topic_value
        config_value_destroy(topic_value);
    }

    // Fetching allowed clients for publisher example
    // using the cfgmgr_interface_t obtained
    config_value_t* all_clients = cfgmgr_get_allowed_clients(publisher_interface);
    if (all_clients == NULL) {
        LOG_ERROR_0("Failed to fetch all_clients");
        return -1;
    }
    config_value_t* topic_val;
    arr_len = config_value_array_len(all_clients);
    if (arr_len == 0) {
        LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
    }
    for (size_t i = 0; i < arr_len; i++) {
        topic_val = config_value_array_get(all_clients, i);
        if (topic_val == NULL) {
            LOG_ERROR_0("topic_value initialization failed");
            config_value_destroy(all_clients);
        }
        LOG_INFO("client value is %s\n", topic_val->body.string);
        // Destroying topic_value
        config_value_destroy(topic_val);
    }

    // Testing cfgmgr_get_app_config API
    config_t* app_config = cfgmgr_get_app_config(cfg_mgr);
    if (app_config == NULL) {
        LOG_ERROR_0("Failed to fetch app_config");
        return -1;
    }
    char* app_config_char = configt_to_char(app_config);
    LOG_INFO("App config is %s\n", app_config_char);

    // Testing cfgmgr_get_app_interface API
    config_t* app_interface = cfgmgr_get_app_interface(cfg_mgr);
    if (app_interface == NULL) {
        LOG_ERROR_0("Failed to fetch app_interface");
        return -1;
    }
    char* app_interface_char = configt_to_char(app_interface);
    LOG_INFO("App interface is %s\n", app_interface_char);

    // Testing cfgmgr_get_app_config_value API
    config_value_t* app_config_value = cfgmgr_get_app_config_value(cfg_mgr, "encoding");
    if (app_config_value == NULL) {
        LOG_ERROR_0("Failed to fetch app_config_value");
        return -1;
    }
    char* app_config_value_char = cvt_to_char(app_config_value);
    LOG_INFO("App config value of encoding is %s\n", app_config_value_char);

    // Testing cfgmgr_get_app_interface_value API
    config_value_t* app_interface_value = cfgmgr_get_app_interface_value(cfg_mgr, "Publishers");
    if (app_interface_value == NULL) {
        LOG_ERROR_0("Failed to fetch app_interface_value");
        return -1;
    }
    char* app_interface_value_char = cvt_to_char(app_interface_value);
    LOG_INFO("App interface value of Publishers is %s\n", app_interface_value_char);


    // Freeing all variables
    if (publisher_interface != NULL) {
        cfgmgr_interface_destroy(publisher_interface);
    }
    if (cfg_mgr != NULL) {
        cfgmgr_destroy(cfg_mgr);
    }
    if (app_name != NULL) {
        config_value_destroy(app_name);
    }
    if (ep != NULL) {
        config_value_destroy(ep);
    }
    if (app_interface_value != NULL) {
        config_value_destroy(app_interface_value);
    }
    if (app_config_value != NULL) {
        config_value_destroy(app_config_value);
    }
    if (topics != NULL) {
        config_value_destroy(topics);
    }
    if (all_clients != NULL) {
        config_value_destroy(all_clients);
    }
    if (pub_config != NULL) {
        config_destroy(pub_config);
    }
    if (pub_config_char != NULL) {
        free(pub_config_char);
    }
    if (app_config_char != NULL) {
        free(app_config_char);
    }
    if (app_interface_char != NULL) {
        free(app_interface_char);
    }
    if (app_config_value_char != NULL) {
        free(app_config_value_char);
    }
    if (app_interface_value_char != NULL) {
        free(app_interface_value_char);
    }
    if (topics_to_be_set != NULL) {
        free(topics_to_be_set);
    }
    return 0;
}
