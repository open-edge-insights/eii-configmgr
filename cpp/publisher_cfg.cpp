// Copyright (c) 2020 Intel Corporation.
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
// FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @brief PublisherCfg Implementation
 * Holds the implementaion of APIs supported by PublisherCfg class
 */


#include "eii/config_manager/publisher_cfg.hpp"

using namespace eii::config_manager;

// Constructor
PublisherCfg::PublisherCfg(cfgmgr_interface_t* cfgmgr_interface):AppCfg(NULL) {
    m_cfgmgr_interface = cfgmgr_interface;
}

PublisherCfg::PublisherCfg(const PublisherCfg& src) :
    AppCfg(NULL)
{
    throw "This object should not be copied";
}

PublisherCfg& PublisherCfg::operator=(const PublisherCfg& src) {
    return *this;
}

// m_cfgmgr_interface getter
cfgmgr_interface_t* PublisherCfg::getPubCfg() {
    return m_cfgmgr_interface;
}

// getMsgBusConfig of Publisher class
config_t* PublisherCfg::getMsgBusConfig() {
    // Calling the base C get_msgbus_config() API
    config_t* pub_config = cfgmgr_get_msgbus_config(m_cfgmgr_interface);
    if (pub_config == NULL) {
        throw "Unable to fetch publisher msgbus config";
    }
    return pub_config;
}

// Get the Interface Value of Publisher.
config_value_t* PublisherCfg::getInterfaceValue(const char* key){
    config_value_t* interface_value = cfgmgr_get_interface_value(m_cfgmgr_interface, key);
    if (interface_value == NULL) {
        throw "Getting interface value from base c layer failed";
    }

    return interface_value;
}

// To fetch endpoint from config
std::string PublisherCfg::getEndpoint() {
    // Calling the base C get_endpoint() API
    config_value_t* ep = cfgmgr_get_endpoint(m_cfgmgr_interface);
    if (ep == NULL) {
        throw "Endpoint not found";
    }

    char* value;
    value = cvt_obj_str_to_char(ep);
    if (value == NULL) {
        config_value_destroy(ep);
        throw "Endpoint object to string conversion failed";
    }

    std::string s(value);
    // Destroying ep
    config_value_destroy(ep);
    return s;
}

// To fetch topics from config
std::vector<std::string> PublisherCfg::getTopics() {

    std::vector<std::string> topic_list;
    // Calling the base C get_topics() API
    config_value_t* topics = cfgmgr_get_topics(m_cfgmgr_interface);
    if (topics == NULL) {
        throw "topics initialization failed";
    }
    config_value_t* topic_value;
    size_t arr_len = config_value_array_len(topics);
    if (arr_len == 0) {
        throw "Empty array is not supported, atleast one value should be given.";
    }
    for (size_t i = 0; i < arr_len; i++) {
        topic_value = config_value_array_get(topics, i);
        if (topic_value == NULL) {
            config_value_destroy(topics);
            throw "topic_value initialization failed";
        }
        topic_list.push_back(topic_value->body.string);
        // Destroying topic_value
        config_value_destroy(topic_value);
    }
    // Destroying topics
    config_value_destroy(topics);
    return topic_list;
}

// To set topics in config
bool PublisherCfg::setTopics(std::vector<std::string> topics_list) {

    size_t topics_length = topics_list.size();
    char **topics_to_be_set = NULL;
    topics_to_be_set = (char**)calloc(topics_length, sizeof(char*));

    if (topics_to_be_set == NULL) {
        LOG_ERROR_0("calloc failed for topics_to_be_set");
        return false;
    }
    for (size_t i = 0; i < topics_length; i++) {
        topics_to_be_set[i] = strdup(topics_list[i].c_str());
        if (topics_to_be_set[i] == NULL) {
            free_mem(topics_to_be_set);
            return false;
        }
    }
    // Calling the base C set_topics() API
    bool topics_set = cfgmgr_set_topics(m_cfgmgr_interface, topics_to_be_set, topics_length);
    if (topics_set) {
        LOG_DEBUG_0("Topics successfully set");
        if (topics_to_be_set != NULL) {
            free_mem(topics_to_be_set);
        }
        return true;
    }

    // Freeing topics_to_be_set
    if (topics_to_be_set != NULL) {
        free_mem(topics_to_be_set);
    }
    return false;
}

// To fetch list of allowed clients from config
std::vector<std::string> PublisherCfg::getAllowedClients() {

    std::vector<std::string> client_list;
    // Calling the base C get_topics() API
    config_value_t* clients = cfgmgr_get_allowed_clients(m_cfgmgr_interface);
    if (clients == NULL) {
        throw "clients initialization failed";
    }
    config_value_t* client_value;
    size_t arr_len = config_value_array_len(clients);
    if (arr_len == 0) {
        throw "Empty array is not supported, atleast one value should be given.";
    }
    for (size_t i = 0; i < arr_len; i++) {
        client_value = config_value_array_get(clients, i);
        if (client_value == NULL) {
            throw "client_value initialization failed";
        }
        client_list.push_back(client_value->body.string);
        // Destroying client_value
        config_value_destroy(client_value);
    }
    // Destroying clients
    config_value_destroy(clients);
    return client_list;
}

// Destructor
PublisherCfg::~PublisherCfg() {
    if (m_cfgmgr_interface) {
        cfgmgr_interface_destroy(m_cfgmgr_interface);
    }
    LOG_DEBUG_0("PublisherCfg destructor");
}