/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdlib.h>
#include <errno.h>
#include <glib.h>

#include "iotcon.h"
#include "iotcon-internal.h"
#include "icl.h"
#include "icl-remote-resource.h"

static void _monitoring_get_cb(iotcon_remote_resource_h resource,
		iotcon_error_e err,
		iotcon_request_type_e request_type,
		iotcon_response_h response,
		void *user_data)
{
	int ret;
	iotcon_response_result_e response_result;
	iotcon_remote_resource_state_e resource_state;

	RET_IF(NULL == resource);
	RET_IF(NULL == resource->monitoring_handle);
	RETM_IF(IOTCON_ERROR_NONE != err, "_monitoring_get() Fail(%d)", err);

	ret = iotcon_response_get_result(response, &response_result);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_response_get_result() Fail(%d)", ret);
		return;
	}

	if (IOTCON_RESPONSE_RESULT_OK <= response_result)
		resource_state = IOTCON_REMOTE_RESOURCE_STATE_ALIVE;
	else
		resource_state = IOTCON_REMOTE_RESOURCE_STATE_LOST_SIGNAL;

	if (resource_state == resource->monitoring_handle->resource_state)
		return;

	resource->monitoring_handle->resource_state = resource_state;

	if (resource->monitoring_handle->cb)
		resource->monitoring_handle->cb(resource, resource_state,
				resource->monitoring_handle->user_data);
}


static gboolean _monitoring_get_timer(gpointer user_data)
{
	int ret;
	iotcon_remote_resource_h resource = user_data;

	RETV_IF(NULL == resource, G_SOURCE_REMOVE);

	ret = iotcon_remote_resource_get(resource, NULL, _monitoring_get_cb, NULL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("iotcon_remote_resource_get() for caching Fail(%d)", ret);

	return G_SOURCE_CONTINUE;
}


static void _monitoring_presence_cb(iotcon_presence_h presence, iotcon_error_e err,
		iotcon_presence_response_h response, void *user_data)
{
	int ret, time_interval;
	unsigned int get_timer_id;
	iotcon_presence_result_e result;
	iotcon_presence_trigger_e trigger;
	iotcon_remote_resource_h resource = user_data;

	RET_IF(NULL == resource);
	RET_IF(NULL == resource->monitoring_handle);
	RETM_IF(IOTCON_ERROR_NONE != err, "_monitoring_presence() Fail(%d)", err);

	ret = iotcon_presence_response_get_result(response, &result);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_presence_response_get_result() Fail(%d)", ret);
		return;
	}

	if (IOTCON_PRESENCE_OK == result) {
		ret = iotcon_presence_response_get_trigger(response, &trigger);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_presence_response_get_trigger() Fail(%d)", ret);
			return;
		}

		if (IOTCON_PRESENCE_TRIGGER_RESOURCE_DESTROYED != trigger)
			return;
	}

	ret = iotcon_remote_resource_get_time_interval(&time_interval);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_remote_resource_get_time_interval() Fail(%d)", ret);
		return;
	}

	g_source_remove(resource->monitoring_handle->get_timer_id);

	_monitoring_get_timer(resource);
	get_timer_id = g_timeout_add_seconds(time_interval, _monitoring_get_timer, resource);
	resource->monitoring_handle->get_timer_id = get_timer_id;
}


API int iotcon_remote_resource_start_monitoring(iotcon_remote_resource_h resource,
		iotcon_remote_resource_state_changed_cb cb, void *user_data)
{
	char *host_address;
	int ret, time_interval;
	unsigned int get_timer_id;
	iotcon_connectivity_type_e connectivity_type;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	if (resource->monitoring_handle) {
		ERR("Already Start Monitoring");
		return IOTCON_ERROR_ALREADY;
	}

	INFO("Start Monitoring");

	resource->monitoring_handle = calloc(1, sizeof(struct icl_remote_resource_monitoring));
	if (NULL == resource->monitoring_handle) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	_monitoring_get_timer(resource);

	/* GET METHOD (Resource Presence) */
	resource->monitoring_handle->cb = cb;
	resource->monitoring_handle->user_data = user_data;

	ret = iotcon_remote_resource_get_time_interval(&time_interval);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_remote_resource_get_time_interval() Fail(%d)", ret);
		free(resource->monitoring_handle);
		resource->monitoring_handle = NULL;
		return ret;
	}

	get_timer_id = g_timeout_add_seconds(time_interval, _monitoring_get_timer, resource);
	resource->monitoring_handle->get_timer_id = get_timer_id;

	/* Device Presence */
	ret = iotcon_remote_resource_get_host_address(resource, &host_address);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_remote_resource_get_host_address() Fail(%d)", ret);
		g_source_remove(resource->monitoring_handle->get_timer_id);
		free(resource->monitoring_handle);
		resource->monitoring_handle = NULL;
		return ret;
	}

	ret = iotcon_remote_resource_get_connectivity_type(resource, &connectivity_type);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_remote_resource_get_connectivity_type() Fail(%d)", ret);
		g_source_remove(resource->monitoring_handle->get_timer_id);
		free(resource->monitoring_handle);
		resource->monitoring_handle = NULL;
		return ret;
	}

	ret = iotcon_add_presence_cb(host_address, connectivity_type, NULL,
			_monitoring_presence_cb, resource, &resource->monitoring_handle->presence);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_add_presence_cb() Fail(%d)", ret);
		g_source_remove(resource->monitoring_handle->get_timer_id);
		free(resource->monitoring_handle);
		resource->monitoring_handle = NULL;
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_stop_monitoring(iotcon_remote_resource_h resource)
{
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == resource->monitoring_handle) {
		ERR("Not Monitoring");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	INFO("Stop Monitoring");

	/* Device Presence */
	ret = iotcon_remove_presence_cb(resource->monitoring_handle->presence);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_remove_presence_cb() Fail(%d)", ret);
		return ret;
	}

	/* GET METHOD */
	g_source_remove(resource->monitoring_handle->get_timer_id);

	free(resource->monitoring_handle);
	resource->monitoring_handle = NULL;

	return IOTCON_ERROR_NONE;
}
