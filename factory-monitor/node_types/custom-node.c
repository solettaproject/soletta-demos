/*
 * This file is part of the Soletta Project
 *
 * Copyright (C) 2015 Intel Corporation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "custom-node-gen.h"
#include "monitor-gen.h"
#include "sol-flow-static.h"

#include <sol-mainloop.h>
#include <errno.h>

struct resource_data {
    char *device_id;
    char *name;
    double temperature;
    bool failure;
    bool reset_failure;
    union {
        struct {
            uint8_t temperature_ready : 1;
            uint8_t name_ready : 1;
            uint8_t failure_ready : 1;
        };
        uint8_t ready;
    };
};

struct internal_data {
    struct sol_vector resources;
    struct sol_timeout *timeout;
    struct sol_flow_node *node;
    uint16_t current_idx;
    uint16_t fetch_idx;
};

static int
next_idx(int idx, int max_idx)
{
    if (++idx >= max_idx)
        idx = 0;

    return idx;
}

static bool
timeout_cb(void *data)
{
    struct internal_data *mdata = data;
    struct resource_data *resource;

    if (mdata->resources.len == 0) {
        SOL_DBG("No resources available yet");
        return true;
    }

    mdata->fetch_idx = next_idx(mdata->fetch_idx, mdata->resources.len);

    resource = sol_vector_get(&mdata->resources, mdata->fetch_idx);
    SOL_NULL_CHECK(resource, true);

    sol_flow_send_string_packet(mdata->node,
        SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__OUT__SET_DEVICE_ID,
        resource->device_id);

    return true;
}

static int
internal_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct internal_data *mdata = data;

    sol_vector_init(&mdata->resources, sizeof(struct resource_data));

    mdata->timeout = sol_timeout_add(900, timeout_cb, mdata);
    SOL_NULL_CHECK(mdata->timeout, -ENOMEM);

    mdata->node = node;

    return 0;
}

static void
internal_close(struct sol_flow_node *node, void *data)
{
    struct internal_data *mdata = data;
    struct resource_data *resource;
    uint16_t i;

    SOL_VECTOR_FOREACH_IDX (&mdata->resources, resource, i) {
        free(resource->name);
        free(resource->device_id);
    }
    sol_vector_clear(&mdata->resources);

    sol_timeout_del(mdata->timeout);
}

static int
internal_process_failure(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct internal_data *mdata = data;
    struct resource_data *resource;
    bool in_value;
    int r;

    r = sol_flow_packet_get_bool(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    resource = sol_vector_get(&mdata->resources, mdata->fetch_idx);
    SOL_NULL_CHECK(resource, -ENOMEM);

    /* If previously on failure state and not anymore, we send packet
     * even without tick, because timer was stopped */
    if (resource->failure && !in_value) {
        sol_flow_send_bool_packet(node,
            SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__OUT__FAILURE,
            in_value);
    }
    if (resource->reset_failure) {
        sol_flow_send_bool_packet(node,
            SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__OUT__SET_FAILURE,
            false);
        resource->reset_failure = false;
    }
    resource->failure = in_value;
    resource->failure_ready = true;

    return 0;
}

static int
internal_process_name(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct internal_data *mdata = data;
    struct resource_data *resource;
    const char *in_value;
    int r;

    r = sol_flow_packet_get_string(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    resource = sol_vector_get(&mdata->resources, mdata->fetch_idx);
    SOL_NULL_CHECK(resource, -ENOENT);

    if (!resource->name || strcmp(resource->name, in_value)) {
        free(resource->name);
        resource->name = strdup(in_value);
        resource->name_ready = true;
    }

    return 0;
}

static int
internal_process_temperature(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct internal_data *mdata = data;
    struct resource_data *resource;
    struct sol_drange in_value;
    int r;

    r = sol_flow_packet_get_drange(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    resource = sol_vector_get(&mdata->resources, mdata->fetch_idx);
    SOL_NULL_CHECK(resource, -ENOENT);

    resource->temperature = in_value.val;
    resource->name_ready = true;

    return 0;
}

static int
process_tick(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct internal_data *mdata = data;
    struct resource_data *resource;
    uint16_t last_idx;

    if (mdata->resources.len == 0) {
        SOL_WRN("No resource available");
        return 0;
    }

    last_idx = mdata->current_idx;
    mdata->current_idx = next_idx(mdata->current_idx, mdata->resources.len);

    resource = sol_vector_get(&mdata->resources, mdata->current_idx);
    SOL_NULL_CHECK(resource, -ENOENT);

    /* Let's try to find a ready resource, i.e., one whose data is known
     * If we fail, give up*/
    while (!resource->ready) {
        mdata->current_idx = next_idx(mdata->current_idx, mdata->resources.len);
        if (mdata->current_idx == last_idx) {
            SOL_DBG("No ready resource");
            return 0; /* No ready resource*/
        }

        resource = sol_vector_get(&mdata->resources, mdata->current_idx);
        SOL_NULL_CHECK(resource, -ENOENT);
    }

    sol_flow_send_string_packet(node,
        SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__OUT__DEVICE_ID, resource->device_id);
    sol_flow_send_string_packet(node,
        SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__OUT__NAME, resource->name);
    sol_flow_send_bool_packet(node,
        SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__OUT__FAILURE, resource->failure);
    sol_flow_send_drange_value_packet(node,
        SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__OUT__TEMPERATURE,
        resource->temperature);

    return 0;
}

static int
process_failure(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct internal_data *mdata = data;
    int r;
    bool in_value;
    struct resource_data *resource;

    r = sol_flow_packet_get_bool(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    resource = sol_vector_get(&mdata->resources, mdata->current_idx);
    /* Our internal oic monitor node may not be pointing to device that
     * failed, so we let it do it when it reach its turn.
     * TODO improve this. Maybe stopping fetch round robin when gets a failure? */
    resource->reset_failure = true;

    return 0;
}

static int
add_device_id_process(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct internal_data *mdata = data;
    const char *in_value;
    struct resource_data *resource;
    uint16_t i;
    int r;

    r = sol_flow_packet_get_string(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    SOL_VECTOR_FOREACH_IDX (&mdata->resources, resource, i) {
        if (!strcmp(resource->device_id, in_value)) {
            SOL_DBG("Ignoring known resource %s", in_value);
            return 0;
        }
    }

    resource = sol_vector_append(&mdata->resources);
    SOL_NULL_CHECK(resource, -ENOMEM);

    resource->device_id = strdup(in_value);
    SOL_NULL_CHECK_GOTO(resource->device_id, err);

    return 0;

err:
    sol_vector_del_last(&mdata->resources);
    return -ENOMEM;
}

static void
custom_pool_new_type(const struct sol_flow_node_type **current)
{
    struct sol_flow_node_type *type;
    const struct sol_flow_node_type **oic_client, **controller;

    static struct sol_flow_static_node_spec nodes[] = {
        { NULL, "custom-controller", NULL },
        { NULL, "monitor-client-temperature", NULL },
        SOL_FLOW_STATIC_NODE_SPEC_GUARD
    };

    /**
     * Remember: SET_FAILURE port will send a message to monitor about failure;
     *           FAILURE port will send locally current status, like
     *           TEMPERATURE.
     */
    static const struct sol_flow_static_conn_spec conns[] = {
        { 0, SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__OUT__SET_DEVICE_ID, 1, SOL_FLOW_NODE_TYPE_MONITOR_CLIENT_TEMPERATURE__IN__DEVICE_ID },
        { 0, SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__OUT__SET_FAILURE, 1, SOL_FLOW_NODE_TYPE_MONITOR_CLIENT_TEMPERATURE__IN__FAILURE },

        { 1, SOL_FLOW_NODE_TYPE_MONITOR_CLIENT_TEMPERATURE__OUT__FAILURE, 0, SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__IN__FAILURE },
        { 1, SOL_FLOW_NODE_TYPE_MONITOR_CLIENT_TEMPERATURE__OUT__TEMPERATURE, 0, SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__IN__TEMPERATURE },
        { 1, SOL_FLOW_NODE_TYPE_MONITOR_CLIENT_TEMPERATURE__OUT__NAME, 0, SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__IN__NAME },

        SOL_FLOW_STATIC_CONN_SPEC_GUARD
    };

    static const struct sol_flow_static_port_spec exported_out[] = {
        { 0, SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__OUT__DEVICE_ID},
        { 0, SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__OUT__TEMPERATURE},
        { 0, SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__OUT__NAME},
        { 0, SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__OUT__FAILURE},
        SOL_FLOW_STATIC_PORT_SPEC_GUARD
    };

    static const struct sol_flow_static_port_spec exported_in[] = {
        { 0, SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__IN__SET_FAILURE},
        { 0, SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__IN__TICK},
        { 0, SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER__IN__ADD_DEVICE_ID},
        SOL_FLOW_STATIC_PORT_SPEC_GUARD
    };

    static const struct sol_flow_static_spec spec = {
        SOL_SET_API_VERSION(.api_version = SOL_FLOW_STATIC_API_VERSION, )
        .nodes = nodes,
        .conns = conns,
        .exported_in = exported_in,
        .exported_out = exported_out
    };

    if (sol_flow_get_node_type("monitor", SOL_FLOW_NODE_TYPE_MONITOR_CLIENT_TEMPERATURE, &oic_client) < 0) {
        *current = NULL;
        return;
    }
    if ((*oic_client)->init_type)
        (*oic_client)->init_type();

    controller = &SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL_CONTROLLER;
    if ((*controller)->init_type)
        (*controller)->init_type();

    nodes[0].type = *controller;
    nodes[1].type = *oic_client;

    type = sol_flow_static_new_type(&spec);
    SOL_NULL_CHECK(type);
#ifdef SOL_FLOW_NODE_TYPE_DESCRIPTION_ENABLED
    type->description = (*current)->description;
#endif
    type->options_size = (*current)->options_size;
    type->default_options = (*current)->default_options;
    *current = type;
}

static void
custom_init_type(void)
{
    custom_pool_new_type(&SOL_FLOW_NODE_TYPE_CUSTOM_NODE_MONITOR_POOL);
}

#include "custom-node-gen.c"
