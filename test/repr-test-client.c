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
#include <glib.h>
#include <iotcon.h>
#include "test.h"

static char *room_resource_sid;
static GList *resource_sid_list;

static const char* const room_uri_path = "/a/room";

static int _get_int_list_cb(int pos, const int value, void *user_data)
{
	DBG("%d°C", value);

	return IOTCON_FUNC_CONTINUE;
}

static void _on_get(iotcon_representation_h recv_repr, int response_result)
{
	int i, ret, int_val;
	bool is_null, bool_val;
	char *uri_path, *str_val;
	iotcon_list_h list_val;
	iotcon_representation_h child_repr;
	iotcon_state_h recv_state, child_state;
	unsigned int key_count, children_count;

	RETM_IF(IOTCON_RESPONSE_RESULT_OK != response_result, "_on_get Response error(%d)",
			response_result);
	INFO("GET request was successful");

	DBG("[ parent representation ]");
	ret = iotcon_representation_get_uri_path(recv_repr, &uri_path);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_get_uri_path() Fail(%d)", ret);
		return;
	}
	DBG("uri_path : %s", uri_path);

	ret = iotcon_representation_get_state(recv_repr, &recv_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_get_state() Fail(%d)", ret);
		return;
	}

	ret = iotcon_state_get_keys_count(recv_state, &key_count);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_get_keys_count() Fail(%d)", ret);
		return;
	}

	if (key_count) {
		ret = iotcon_state_get_str(recv_state, "name", &str_val);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_state_get_str() Fail(%d)", ret);
			return;
		}
		DBG("name : %s", str_val);

		ret = iotcon_state_get_list(recv_state, "today_temp", &list_val);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_state_get_list() Fail(%d)", ret);
			return;
		}

		DBG("today's temperature :");
		ret = iotcon_list_foreach_int(list_val, _get_int_list_cb, NULL);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_foreach_int() Fail(%d)", ret);
			return;
		}

		ret = iotcon_state_is_null(recv_state, "null value", &is_null);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_state_is_null() Fail(%d)", ret);
			return;
		}

		if (is_null)
			DBG("null value is null");
	}

	ret = iotcon_representation_get_children_count(recv_repr, &children_count);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_get_children_count() Fail(%d)", ret);
		return;
	}

	for (i = 0; i < children_count; i++) {
		DBG("[ child representation ]");

		ret = iotcon_representation_get_nth_child(recv_repr, i, &child_repr);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_representation_get_nth_child(%d) Fail(%d)", i, ret);
			continue;
		}

		ret = iotcon_representation_get_uri_path(child_repr, &uri_path);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_representation_get_uri_path() Fail(%d)", ret);
			continue;
		}
		DBG("uri_path : %s", uri_path);

		ret = iotcon_representation_get_state(child_repr, &child_state);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_representation_get_state() Fail(%d)", ret);
			continue;
		}

		if (TEST_STR_EQUAL == strcmp("/a/light", uri_path)) {
			ret = iotcon_state_get_keys_count(child_state, &key_count);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_state_get_keys_count() Fail(%d)", ret);
				continue;
			}

			if (key_count) {
				ret = iotcon_state_get_int(child_state, "brightness", &int_val);
				if (IOTCON_ERROR_NONE != ret) {
					ERR("iotcon_state_get_int() Fail(%d)", ret);
					continue;
				}
				DBG("brightness : %d", int_val);
			}
		} else if (TEST_STR_EQUAL == strcmp("/a/switch", uri_path)) {
			ret = iotcon_state_get_keys_count(child_state, &key_count);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_state_get_keys_count() Fail(%d)", ret);
				continue;
			}
			if (key_count) {
				ret = iotcon_state_get_bool(child_state, "switch", &bool_val);
				if (IOTCON_ERROR_NONE != ret) {
					ERR("iotcon_state_get_bool() Fail(%d)", ret);
					continue;
				}
				DBG("switch : %d", bool_val);
			}
		}
	}
}

static void _on_get_2nd(iotcon_client_h resource, iotcon_representation_h recv_repr,
		iotcon_options_h header_options, int response_result, void *user_data)
{
	_on_get(recv_repr, response_result);
}

static void _on_get_1st(iotcon_client_h resource, iotcon_representation_h recv_repr,
		iotcon_options_h header_options, int response_result, void *user_data)
{
	int ret;
	iotcon_query_h query_params;

	_on_get(recv_repr, response_result);

	ret = iotcon_query_create(&query_params);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_query_create() Fail(%d)", ret);
		return;
	}

	ret = iotcon_query_insert(query_params, "if", "oic.if.b");
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_query_insert() Fail(%d)", ret);
		iotcon_query_destroy(query_params);
		return;
	}

	/* send GET request again with BATCH interface */
	ret = iotcon_get(resource, query_params, _on_get_2nd, NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_get() Fail(%d)", ret);
		iotcon_query_destroy(query_params);
		return;
	}

	iotcon_query_destroy(query_params);
}

static int _get_res_type_fn(const char *string, void *user_data)
{
	char *resource_uri_path = user_data;

	DBG("[%s] resource type : %s", resource_uri_path, string);

	return IOTCON_FUNC_CONTINUE;
}

static int _sid_compare(const void *a, const void *b)
{
	return strcmp(a, b);
}

static void _found_resource(iotcon_client_h resource, void *user_data)
{
	int ret;
	GList *node;
	char *resource_sid;
	char *resource_host;
	char *resource_uri_path;
	int resource_interfaces;
	iotcon_resource_types_h resource_types;

	if (NULL == resource)
		return;

	INFO("===== resource found =====");

	/* get the resource URI */
	ret = iotcon_client_get_uri_path(resource, &resource_uri_path);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_uri_path() Fail(%d)", ret);
		return;
	}

	/* get the resource server id */
	ret = iotcon_client_get_server_id(resource, &resource_sid);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_server_id() Fail(%d)", ret);
		return;
	}
	DBG("[%s] resource server id : %s", resource_uri_path, resource_sid);

	node = g_list_find_custom(resource_sid_list, resource_sid, _sid_compare);

	if (node && TEST_STR_EQUAL == strcmp(room_uri_path, resource_uri_path)) {
		DBG("uri_path \"%s\" already found. skip !", resource_uri_path);
		return;
	}

	room_resource_sid = strdup(resource_sid);
	if (NULL == room_resource_sid) {
		ERR("strdup(room_resource_sid) Fail");
		return;
	}

	resource_sid_list = g_list_append(resource_sid_list, room_resource_sid);

	/* get the resource host address */
	ret = iotcon_client_get_host(resource, &resource_host);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_host() Fail(%d)", ret);
		resource_sid_list = g_list_remove(resource_sid_list, room_resource_sid);
		free(room_resource_sid);
		return;
	}
	DBG("[%s] resource host : %s", resource_uri_path, resource_host);

	/* get the resource interfaces */
	ret = iotcon_client_get_interfaces(resource, &resource_interfaces);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_interfaces() Fail(%d)", ret);
		resource_sid_list = g_list_remove(resource_sid_list, room_resource_sid);
		free(room_resource_sid);
		return;
	}
	if (IOTCON_INTERFACE_DEFAULT & resource_interfaces)
		DBG("[%s] resource interface : DEFAULT_INTERFACE", resource_uri_path);
	if (IOTCON_INTERFACE_LINK & resource_interfaces)
		DBG("[%s] resource interface : LINK_INTERFACE", resource_uri_path);
	if (IOTCON_INTERFACE_BATCH & resource_interfaces)
		DBG("[%s] resource interface : BATCH_INTERFACE", resource_uri_path);
	if (IOTCON_INTERFACE_GROUP & resource_interfaces)
		DBG("[%s] resource interface : GROUP_INTERFACE", resource_uri_path);

	/* get the resource types */
	ret = iotcon_client_get_types(resource, &resource_types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_types() Fail(%d)", ret);
		resource_sid_list = g_list_remove(resource_sid_list, room_resource_sid);
		free(room_resource_sid);
		return;
	}
	ret = iotcon_resource_types_foreach(resource_types, _get_res_type_fn,
			resource_uri_path);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_foreach() Fail(%d)", ret);
		resource_sid_list = g_list_remove(resource_sid_list, room_resource_sid);
		free(room_resource_sid);
		return;
	}

	if (TEST_STR_EQUAL == strcmp(room_uri_path, resource_uri_path)) {
		/* send GET request */
		ret = iotcon_get(resource, NULL, _on_get_1st, NULL);
		if (IOTCON_ERROR_NONE != ret)
			ERR("iotcon_get() Fail(%d)", ret);
	}

	resource_sid_list = g_list_remove(resource_sid_list, room_resource_sid);
	free(room_resource_sid);
}

int main(int argc, char **argv)
{
	FN_CALL;
	int ret;
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon open */
	ret = iotcon_open();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_open() Fail(%d)", ret);
		return -1;
	}

	/* find room typed resources */
	ret = iotcon_find_resource(IOTCON_MULTICAST_ADDRESS, "core.room", &_found_resource,
			NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_find_resource() Fail(%d)", ret);
		iotcon_close();
		return -1;
	}

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	g_list_free_full(resource_sid_list, free);

	/* iotcon close */
	iotcon_close();

	return 0;
}
