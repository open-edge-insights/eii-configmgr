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
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @brief ConfigManager Client usage example
 */

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eis/config_manager/config_mgr.hpp"
#include "eis/msgbus/msgbus.h"
#include "eis/utils/logger.h"
#include "eis/utils/json_config.h"

#define SERVICE_NAME "echo_service"


using namespace eis::config_manager;

// Globals for cleaning up nicely
void* g_msgbus_ctx = NULL;
recv_ctx_t* g_service_ctx = NULL;

/**
 * Signal handler
 */
void signal_handler(int signo) {
    LOG_INFO_0("Cleaning up");
    if(g_service_ctx != NULL) {
        LOG_INFO_0("Freeing publisher");
        msgbus_recv_ctx_destroy(g_msgbus_ctx, g_service_ctx);
        g_service_ctx = NULL;
    }
    if(g_msgbus_ctx != NULL) {
        LOG_INFO_0("Freeing message bus context");
        msgbus_destroy(g_msgbus_ctx);
        g_msgbus_ctx = NULL;
    }
    LOG_INFO_0("Done.");
}

int main() {

    // Set log level
    set_log_level(LOG_LVL_DEBUG);

    // Setting up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    msg_envelope_serialized_part_t* parts = NULL;
    msg_envelope_t* msg = NULL;
    int num_parts = 0;
    msgbus_ret_t ret = MSG_SUCCESS;

    setenv("DEV_MODE", "FALSE", 1);
    // Replace 2nd parameter with path to certs
    setenv("CONFIGMGR_CERT", "", 1);
    setenv("CONFIGMGR_KEY", "", 1);
    setenv("CONFIGMGR_CACERT", "", 1);
    
    // Uncomment below lines to test DEV mode
    // setenv("DEV_MODE", "TRUE", 1);
    // setenv("CONFIGMGR_CERT", "", 1);
    // setenv("CONFIGMGR_KEY", "", 1);
    // setenv("CONFIGMGR_CACERT", "", 1);

    setenv("AppName","VideoAnalytics", 1);
    ConfigMgr* ch = new ConfigMgr();

    ClientCfg* client_ctx = ch->getClientByName("default");
    config_t* config = client_ctx->getMsgBusConfig();

    AppCfg* cfg = ch->getAppConfig();
    config_value_t* app_config = cfg->getInterfaceValue("Clients");
    config_value_t* cli_config = config_value_array_get(app_config, 0);
    config_value_t* cli_name = config_value_object_get(cli_config, "Name");
    char* name = cli_name->body.string;

    // Initailize request
    msg_envelope_elem_body_t* integer = msgbus_msg_envelope_new_integer(42);
    msg_envelope_elem_body_t* fp = msgbus_msg_envelope_new_floating(55.5);

    if(config == NULL) {
        LOG_ERROR_0("Failed to load JSON configuration");
        goto err;
    }

    g_msgbus_ctx = msgbus_initialize(config);
    if(g_msgbus_ctx == NULL) {
        LOG_ERROR_0("Failed to initialize message bus");
        goto err;
    }

    ret = msgbus_service_get(
            g_msgbus_ctx, name, NULL, &g_service_ctx);
    if(ret != MSG_SUCCESS) {
        LOG_ERROR("Failed to initialize service (errno: %d)", ret);
        goto err;
    }

    msg = msgbus_msg_envelope_new(CT_JSON);

    msgbus_msg_envelope_put(msg, "hello", integer);
    msgbus_msg_envelope_put(msg, "world", fp);

    LOG_INFO_0("Sending request");
    ret = msgbus_request(g_msgbus_ctx, g_service_ctx, msg);
    if(ret != MSG_SUCCESS) {
        LOG_ERROR("Failed to send request (errno: %d)", ret);
        goto err;
    }

    msgbus_msg_envelope_destroy(msg);
    msg = NULL;

    LOG_INFO_0("Waiting for response");
    ret = msgbus_recv_wait(g_msgbus_ctx, g_service_ctx, &msg);
    if(ret != MSG_SUCCESS) {
        // Interrupt is an acceptable error
        if(ret != MSG_ERR_EINTR)
            LOG_ERROR("Failed to receive message (errno: %d)", ret);
        goto err;
    }

    num_parts = msgbus_msg_envelope_serialize(msg, &parts);
    if(num_parts <= 0) {
        LOG_ERROR_0("Failed to serialize message");
        goto err;
    }

    LOG_INFO("Received Response: %s", parts[0].bytes);

    msgbus_msg_envelope_serialize_destroy(parts, num_parts);
    msgbus_msg_envelope_destroy(msg);
    msg = NULL;

    msgbus_recv_ctx_destroy(g_msgbus_ctx, g_service_ctx);
    msgbus_destroy(g_msgbus_ctx);

    return 0;

err:
    if(msg != NULL)
        msgbus_msg_envelope_destroy(msg);
    if(parts != NULL)
        msgbus_msg_envelope_serialize_destroy(parts, num_parts);
    if(g_service_ctx != NULL)
        msgbus_recv_ctx_destroy(g_msgbus_ctx, g_service_ctx);
    if(g_msgbus_ctx != NULL)
        msgbus_destroy(g_msgbus_ctx);
    return -1;
}
