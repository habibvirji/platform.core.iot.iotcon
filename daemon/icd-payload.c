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
#include <stdint.h>
#include <stdlib.h>
#include <glib.h>

#include <ocstack.h>
#include <octypes.h>
#include <ocpayload.h>
#include <ocrandom.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "icd.h"
#include "icd-ioty.h"
#include "icd-ioty-type.h"
#include "icd-payload.h"

union icd_state_value_u {
	int i;
	double d;
	bool b;
};

struct icd_state_list_s {
	OCRepPayloadPropType type;
	size_t dimensions[MAX_REP_ARRAY_DEPTH];
	GList *list;
};

static GVariant* _icd_payload_representation_to_gvariant(OCRepPayload *repr, gboolean is_parent);
static void _icd_state_value_from_gvariant(OCRepPayload *repr, GVariantIter *iter);
static GVariantBuilder* _icd_state_value_to_gvariant_builder(OCRepPayload *repr);

GVariant** icd_payload_res_to_gvariant(OCPayload *payload, OCDevAddr *dev_addr)
{
	int port = 0;
	int ifaces = 0;
	GVariant **value;
	OCStringLL *node;
	iotcon_interface_e iface;
	GVariantBuilder types;
	OCRandomUuidResult random_res;
	OCDiscoveryPayload *discovered;
	struct OCResourcePayload *resource;
	int i, properties, ret, res_count;
	char device_id[UUID_STRING_SIZE] = {0};

	discovered = (OCDiscoveryPayload*)payload;
	resource = discovered->resources;

	res_count = OCDiscoveryPayloadGetResourceCount(discovered);

	value = calloc(res_count + 1, sizeof(GVariant*));
	if (NULL == value) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}
	for (i = 0; resource; i++) {
		/* uri path */
		if (NULL == resource->uri) {
			ERR("resource uri is NULL");
			resource = resource->next;
		}

		/* device id */
		random_res = OCConvertUuidToString(discovered->sid, device_id);
		if (RAND_UUID_OK != random_res) {
			ERR("OCConvertUuidToString() Fail(%d)", random_res);
			resource = resource->next;
		}

		/* Resource Types */
		g_variant_builder_init(&types, G_VARIANT_TYPE("as"));
		node = resource->types;
		if (NULL == node) {
			ERR("resource types is NULL");
			resource = resource->next;
		}
		while (node) {
			g_variant_builder_add(&types, "s", node->value);
			node = node->next;
		}

		/* Resource Interfaces */
		node = resource->interfaces;
		if (NULL == node) {
			ERR("resource interfaces is NULL");
			resource = resource->next;
		}
		while (node) {
			ret = ic_utils_convert_interface_string(node->value, &iface);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("ic_utils_convert_interface_string() Fail(%d)", ret);
				resource = resource->next;
			}
			ifaces |= iface;

			node = node->next;
		}

		/* Resource Properties */
		properties = icd_ioty_oic_properties_to_properties(resource->bitmap);

		/* port */
		port = (resource->port) ? resource->port : dev_addr->port;

		/* TODO
		 * Check "resource->secure" and "resource->bitmap" */
		value[i] = g_variant_new("(ssiasibsi)", resource->uri, device_id, ifaces, &types,
				properties, resource->secure, dev_addr->addr, port);
		DBG("found resource[%d] : %s", i, g_variant_print(value[i], FALSE));

		resource = resource->next;
	}

	return value;
}


static GVariant* _icd_state_array_attr_to_gvariant(OCRepPayloadValueArray *arr, int len,
		int index)
{
	int i;
	GVariant *var;
	GVariantBuilder builder;

	g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);

	switch (arr->type) {
	case OCREP_PROP_INT:
		for (i = 0; i < len; i++)
			g_variant_builder_add(&builder, "i", arr->iArray[index + i]);
		break;
	case OCREP_PROP_BOOL:
		for (i = 0; i < len; i++)
			g_variant_builder_add(&builder, "b", arr->bArray[index + i]);
		break;
	case OCREP_PROP_DOUBLE:
		for (i = 0; i < len; i++)
			g_variant_builder_add(&builder, "d", arr->dArray[index + i]);
		break;
	case OCREP_PROP_STRING:
		for (i = 0; i < len; i++)
			g_variant_builder_add(&builder, "s", arr->strArray[index + i]);
		break;
	case OCREP_PROP_NULL:
		for (i = 0; i < len; i++)
			g_variant_builder_add(&builder, "s", IC_STR_NULL);
		break;
	case OCREP_PROP_OBJECT:
		for (i = 0; i < len; i++) {
			GVariantBuilder *state_var = _icd_state_value_to_gvariant_builder(arr->objArray[index + i]);
			var = g_variant_builder_end(state_var);
			g_variant_builder_add(&builder, "v", var);
		}
		break;
	case OCREP_PROP_ARRAY:
	default:
		break;
	}

	return g_variant_builder_end(&builder);
}


static GVariant* _icd_state_array_to_gvariant(OCRepPayloadValueArray *arr,
		int current_depth, int current_len, int index)
{
	int i, next_len;
	GVariantBuilder builder;
	GVariant *arr_var = NULL;

	if ((MAX_REP_ARRAY_DEPTH - 1) == current_depth
			|| 0 == arr->dimensions[current_depth + 1])
		return _icd_state_array_attr_to_gvariant(arr, current_len, index);

	i = current_len + index;

	next_len = current_len / arr->dimensions[current_depth];
	index -= next_len;

	g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);

	while ((index += next_len) < i) {
		arr_var = _icd_state_array_to_gvariant(arr, current_depth + 1, next_len, index);
		g_variant_builder_add(&builder, "v", arr_var);
	}

	return g_variant_builder_end(&builder);
}

static GVariant* _icd_state_value_to_gvariant(OCRepPayload *state)
{
	GVariantBuilder *builder;
	GVariant *var;

	builder = _icd_state_value_to_gvariant_builder(state);
	var = g_variant_builder_end(builder);

	return var;
}

static GVariantBuilder* _icd_state_value_to_gvariant_builder(OCRepPayload *repr)
{
	int total_len;
	GVariant *var = NULL;
	GVariantBuilder *builder;
	OCRepPayloadValue *val = repr->values;

	builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

	while (val) {
		switch (val->type) {
		case OCREP_PROP_INT:
			var = g_variant_new_int32(val->i);
			break;
		case OCREP_PROP_BOOL:
			var = g_variant_new_boolean(val->b);
			break;
		case OCREP_PROP_DOUBLE:
			var = g_variant_new_double(val->d);
			break;
		case OCREP_PROP_STRING:
			var = g_variant_new_string(val->str);
			break;
		case OCREP_PROP_NULL:
			var = g_variant_new_string(IC_STR_NULL);
			break;
		case OCREP_PROP_ARRAY:
			total_len = calcDimTotal(val->arr.dimensions);
			var = _icd_state_array_to_gvariant(&(val->arr), 0, total_len, 0);
			break;
		case OCREP_PROP_OBJECT:
			var = _icd_state_value_to_gvariant(val->obj);
			break;
		default:
			ERR("Invalid Type");
		}
		if (var) {
			g_variant_builder_add(builder, "{sv}", val->name, var);
			var = NULL;
		}
		val = val->next;
	}

	return builder;
}


static GVariant* _icd_payload_representation_to_gvariant(OCRepPayload *repr,
		gboolean is_parent)
{
	OCStringLL *node;
	int ret, ifaces = 0;
	GVariant *child, *value;
	OCRepPayload *child_node;
	iotcon_interface_e iface;
	GVariantBuilder *repr_gvar;
	GVariantBuilder children, types_builder;

	RETV_IF(NULL == repr, NULL);

	/* Resource Types */
	g_variant_builder_init(&types_builder, G_VARIANT_TYPE("as"));

	node = repr->types;
	while (node) {
		g_variant_builder_add(&types_builder, "s", node->value);
		node = node->next;
	}

	/* Resource Interfaces */
	node = repr->interfaces;
	while (node) {
		ret = ic_utils_convert_interface_string(node->value, &iface);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("ic_utils_convert_interface_string() Fail(%d)", ret);
			return NULL;
		}
		ifaces |= iface;

		node = node->next;
	}

	/* Representation */
	repr_gvar = _icd_state_value_to_gvariant_builder(repr);

	/* Children */
	g_variant_builder_init(&children, G_VARIANT_TYPE("av"));

	child_node = repr->next;
	while (is_parent && child_node) {
		/* generate recursively */
		child = _icd_payload_representation_to_gvariant(child_node, FALSE);
		g_variant_builder_add(&children, "v", child);
		child_node = child_node->next;
	}

	value = g_variant_new("(siasa{sv}av)", ic_utils_dbus_encode_str(repr->uri), ifaces,
			&types_builder, repr_gvar, &children);

	return value;
}


GVariant* icd_payload_representation_empty_gvariant(void)
{
	GVariant *value;
	GVariantBuilder types, repr, children;

	g_variant_builder_init(&types, G_VARIANT_TYPE("as"));
	g_variant_builder_init(&repr, G_VARIANT_TYPE("a{sv}"));
	g_variant_builder_init(&children, G_VARIANT_TYPE("av"));

	value = g_variant_new("(siasa{sv}av)", IC_STR_NULL, 0, &types, &repr, &children);

	return value;
}


static GVariant* _icd_payload_platform_to_gvariant(OCPlatformPayload *repr)
{
	GVariant *value;

	value = g_variant_new("(sssssssssss)",
			repr->info.platformID,
			repr->info.manufacturerName,
			ic_utils_dbus_encode_str(repr->info.manufacturerUrl),
			ic_utils_dbus_encode_str(repr->info.modelNumber),
			ic_utils_dbus_encode_str(repr->info.dateOfManufacture),
			ic_utils_dbus_encode_str(repr->info.platformVersion),
			ic_utils_dbus_encode_str(repr->info.operatingSystemVersion),
			ic_utils_dbus_encode_str(repr->info.hardwareVersion),
			ic_utils_dbus_encode_str(repr->info.firmwareVersion),
			ic_utils_dbus_encode_str(repr->info.supportUrl),
			ic_utils_dbus_encode_str(repr->info.systemTime));

	return value;
}


static GVariant* _icd_payload_device_to_gvariant(OCDevicePayload *repr)
{
	GVariant *value;
	OCRandomUuidResult random_res;
	char device_id[UUID_STRING_SIZE] = {0};

	random_res = OCConvertUuidToString(repr->sid, device_id);
	if (RAND_UUID_OK != random_res) {
		ERR("OCConvertUuidToString() Fail(%d)", random_res);
		return NULL;
	}

	value = g_variant_new("(ssss)", repr->deviceName, repr->specVersion,
			device_id, repr->dataModelVersion);

	return value;
}


GVariant* icd_payload_to_gvariant(OCPayload *repr)
{
	GVariant *value = NULL;

	if (NULL == repr)
		return value;

	switch (repr->type) {
	case PAYLOAD_TYPE_REPRESENTATION:
		value = _icd_payload_representation_to_gvariant((OCRepPayload*)repr, TRUE);
		break;
	case PAYLOAD_TYPE_PLATFORM:
		value = _icd_payload_platform_to_gvariant((OCPlatformPayload*)repr);
		break;
	case PAYLOAD_TYPE_DEVICE:
		value = _icd_payload_device_to_gvariant((OCDevicePayload*)repr);
		break;
	case PAYLOAD_TYPE_PRESENCE:
	case PAYLOAD_TYPE_SECURITY:
	default:
		break;
	}

	return value;
}


static void _icd_state_list_from_gvariant(GVariant *var,
		struct icd_state_list_s *value_list, int depth)
{
	GVariantIter iter;
	const GVariantType *type;
	union icd_state_value_u *value;

	type = g_variant_get_type(var);

	g_variant_iter_init(&iter, var);

	value_list->dimensions[depth] = g_variant_iter_n_children(&iter);
	DBG("[%d]list dim : %d", depth, value_list->dimensions[depth]);

	if (g_variant_type_equal(G_VARIANT_TYPE("ab"), type)) {
		bool b;
		value_list->type = OCREP_PROP_BOOL;
		while (g_variant_iter_loop(&iter, "b", &b)) {
			value = calloc(1, sizeof(union icd_state_value_u));
			if (NULL == value) {
				ERR("calloc() Fail(%d)", errno);
				return;
			}
			value->b = b;
			value_list->list = g_list_append(value_list->list, value);
		}
	} else if (g_variant_type_equal(G_VARIANT_TYPE("ai"), type)) {
		int i;
		value_list->type = OCREP_PROP_INT;
		while (g_variant_iter_loop(&iter, "i", &i)) {
			value = calloc(1, sizeof(union icd_state_value_u));
			if (NULL == value) {
				ERR("calloc() Fail(%d)", errno);
				return;
			}
			value->i = i;
			value_list->list = g_list_append(value_list->list, value);
		}
	} else if (g_variant_type_equal(G_VARIANT_TYPE("ad"), type)) {
		double d;
		value_list->type = OCREP_PROP_DOUBLE;
		while (g_variant_iter_loop(&iter, "d", &d)) {
			value = calloc(1, sizeof(union icd_state_value_u));
			if (NULL == value) {
				ERR("calloc() Fail(%d)", errno);
				return;
			}
			value->d = d;
			value_list->list = g_list_append(value_list->list, value);
		}
	} else if (g_variant_type_equal(G_VARIANT_TYPE("as"), type)) {
		char *s;
		value_list->type = OCREP_PROP_STRING;
		while (g_variant_iter_next(&iter, "s", &s))
			value_list->list = g_list_append(value_list->list, s);
	} else if (g_variant_type_equal(G_VARIANT_TYPE("av"), type)) {
		GVariant *value;
		if (g_variant_iter_loop(&iter, "v", &value)) {
			if (g_variant_is_of_type(value, G_VARIANT_TYPE("a{sv}"))) {
				OCRepPayload *repr;
				GVariantIter state_iter;
				value_list->type = OCREP_PROP_OBJECT;
				do {
					repr = OCRepPayloadCreate();
					g_variant_iter_init(&state_iter, value);
					_icd_state_value_from_gvariant(repr, &state_iter);
					value_list->list = g_list_append(value_list->list, repr);
				} while (g_variant_iter_loop(&iter, "v", &value));

			} else if (g_variant_is_of_type(value, G_VARIANT_TYPE_ARRAY)) {
				do {
					_icd_state_list_from_gvariant(value, value_list, depth + 1);
				} while (g_variant_iter_loop(&iter, "v", &value));
			}
		}
	}

	return;
}


static void _icd_state_list_free(gpointer node)
{
	OCRepPayloadDestroy(node);
}


static void _icd_state_array_from_list(OCRepPayload *repr,
		struct icd_state_list_s *value_list, const char *key)
{
	int i, len;
	GList *node;
	bool *b_arr;
	double *d_arr;
	char **str_arr;
	int64_t *i_arr;
	union icd_state_value_u *value;
	struct OCRepPayload **state_arr;

	len = calcDimTotal(value_list->dimensions);

	switch (value_list->type) {
	case OCREP_PROP_INT:
		i_arr = calloc(len, sizeof(int64_t));
		if (NULL == i_arr) {
			ERR("calloc() Fail(%d)", errno);
			return;
		}
		for (node = value_list->list, i = 0; node; node = node->next, i++) {
			value = node->data;
			i_arr[i] = value->i;
		}
		g_list_free_full(value_list->list, free);
		OCRepPayloadSetIntArrayAsOwner(repr, key, i_arr, value_list->dimensions);
		break;
	case OCREP_PROP_BOOL:
		b_arr = calloc(len, sizeof(bool));
		if (NULL == b_arr) {
			ERR("calloc() Fail(%d)", errno);
			return;
		}
		for (node = value_list->list, i = 0; node; node = node->next, i++) {
			value = node->data;
			b_arr[i] = value->b;
		}
		g_list_free_full(value_list->list, free);
		OCRepPayloadSetBoolArrayAsOwner(repr, key, b_arr, value_list->dimensions);
		break;
	case OCREP_PROP_DOUBLE:
		d_arr = calloc(len, sizeof(double));
		if (NULL == d_arr) {
			ERR("calloc() Fail(%d)", errno);
			return;
		}
		for (node = value_list->list, i = 0; node; node = node->next, i++) {
			value = node->data;
			d_arr[i] = value->d;
		}
		g_list_free_full(value_list->list, free);
		OCRepPayloadSetDoubleArrayAsOwner(repr, key, d_arr, value_list->dimensions);
		break;
	case OCREP_PROP_STRING:
		str_arr = calloc(len, sizeof(char *));
		if (NULL == str_arr) {
			ERR("calloc() Fail(%d)", errno);
			return;
		}
		for (node = value_list->list, i = 0; node; node = node->next, i++)
			str_arr[i] = strdup(node->data);
		g_list_free_full(value_list->list, free);
		OCRepPayloadSetStringArrayAsOwner(repr, key, str_arr, value_list->dimensions);
		break;
	case OCREP_PROP_OBJECT:
		state_arr = calloc(len, sizeof(struct OCRepPayload *));
		if (NULL == state_arr) {
			ERR("calloc() Fail(%d)", errno);
			return;
		}
		for (node = value_list->list, i = 0; node; node = node->next, i++)
			state_arr[i] = OCRepPayloadClone(node->data);
		g_list_free_full(value_list->list, _icd_state_list_free);
		OCRepPayloadSetPropObjectArrayAsOwner(repr, key, state_arr,
				value_list->dimensions);
		break;
	case OCREP_PROP_ARRAY:
	case OCREP_PROP_NULL:
	default:
		ERR("Invalid Type");
	}
}

static void _icd_state_value_from_gvariant(OCRepPayload *repr, GVariantIter *iter)
{
	char *key;
	GVariant *var;
	const char *str_value;
	OCRepPayload *repr_value;
	struct icd_state_list_s value_list = {0};

	while (g_variant_iter_loop(iter, "{sv}", &key, &var)) {

		if (g_variant_is_of_type(var, G_VARIANT_TYPE_BOOLEAN)) {
			OCRepPayloadSetPropBool(repr, key, g_variant_get_boolean(var));

		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE_INT32)) {
			OCRepPayloadSetPropInt(repr, key, g_variant_get_int32(var));

		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE_DOUBLE)) {
			OCRepPayloadSetPropDouble(repr, key, g_variant_get_double(var));

		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE_STRING)) {
			str_value = g_variant_get_string(var, NULL);
			if (NULL == str_value) {
				ERR("g_variant_get_string() Fail");
				return;
			}
			if (IC_STR_EQUAL == strcmp(IC_STR_NULL, str_value))
				OCRepPayloadSetNull(repr, key);
			else
				OCRepPayloadSetPropString(repr, key, str_value);

		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE("a{sv}"))) {
			GVariantIter state_iter;
			repr_value = OCRepPayloadCreate();
			g_variant_iter_init(&state_iter, var);
			_icd_state_value_from_gvariant(repr_value, &state_iter);
			OCRepPayloadSetPropObjectAsOwner(repr, key, repr_value);

		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE_ARRAY)) {
			memset(&value_list, 0, sizeof(struct icd_state_list_s));
			_icd_state_list_from_gvariant(var, &value_list, 0);
			_icd_state_array_from_list(repr, &value_list, key);

		} else {
			ERR("Invalid type(%s)", g_variant_get_type_string(var));
		}
	}

	return;
}

OCRepPayload* icd_payload_representation_from_gvariant(GVariant *var)
{
	GVariant *child;
	int ret, i, ifaces = 0;
	OCRepPayload *repr, *cur;
	char *uri_path, *iface_str, *resource_type;
	GVariantIter *resource_types, *repr_gvar, *children;

	repr = OCRepPayloadCreate();

	g_variant_get(var, "(&siasa{sv}av)", &uri_path, &ifaces, &resource_types, &repr_gvar,
			&children);

	if (IC_STR_EQUAL != strcmp(IC_STR_NULL, uri_path))
		OCRepPayloadSetUri(repr, uri_path);

	for (i = 1; i <= IC_INTERFACE_MAX; i = i << 1) {
		if (IOTCON_INTERFACE_NONE == (ifaces & i)) /* this interface not exist */
			continue;
		ret = ic_utils_convert_interface_flag((ifaces & i), &iface_str);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("ic_utils_convert_interface_flag(%d) Fail(%d)", i, ret);
			OCRepPayloadDestroy(repr);
			return NULL;
		}
		OCRepPayloadAddInterface(repr, iface_str);
	}
	while (g_variant_iter_loop(resource_types, "s", &resource_type))
		OCRepPayloadAddResourceType(repr, resource_type);

	_icd_state_value_from_gvariant(repr, repr_gvar);

	cur = repr;
	while (g_variant_iter_loop(children, "v", &child)) {
		cur->next = icd_payload_representation_from_gvariant(child);
		if (NULL == cur->next) {
			ERR("icd_payload_representation_from_gvariant() Fail");
			OCRepPayloadDestroy(repr);
			return NULL;
		}
		cur = cur->next;
	}
	return repr;
}


static int _oic_string_list_length(OCStringLL *str_list)
{
	int len = 0;

	while (str_list) {
		len++;
		str_list = str_list->next;
	}

	return len;
}


static bool _oic_string_list_contain(OCStringLL *str_list, char *str_value)
{
	OCStringLL *c;

	for (c = str_list; c; c = c->next) {
		if (IC_STR_EQUAL == g_strcmp0(str_value, str_list->value))
			return true;
	}

	return false;
}


static int _representation_compare_string_list(OCStringLL *list1, OCStringLL *list2)
{
	OCStringLL *c;

	if (NULL == list1 || NULL == list2)
		return !!(list1 - list2);

	if (_oic_string_list_length(list1) != _oic_string_list_length(list2))
		return 1;

	for (c = list1; c; c = c->next) {
		if (false == _oic_string_list_contain(list2, c->value))
			return 1;
	}

	return IC_EQUAL;
}


static int _representation_compare_array(OCRepPayloadValueArray arr1,
		OCRepPayloadValueArray arr2)
{
	int i, len1, len2;

	len1 = calcDimTotal(arr1.dimensions);
	len2 = calcDimTotal(arr2.dimensions);

	if (len1 != len2)
		return 1;

	switch (arr1.type) {
	case OCREP_PROP_INT:
		for (i = 0; i < len1; i++) {
			if (arr1.iArray[i] != arr2.iArray[i])
				return 1;
		}
		break;
	case OCREP_PROP_BOOL:
		for (i = 0; i < len1; i++) {
			if (arr1.bArray[i] != arr2.bArray[i])
				return 1;
		}
		break;
	case OCREP_PROP_DOUBLE:
		for (i = 0; i < len1; i++) {
			if (arr1.dArray[i] != arr2.dArray[i])
				return 1;
		}
		break;
	case OCREP_PROP_STRING:
		for (i = 0; i < len1; i++) {
			if (IC_STR_EQUAL != g_strcmp0(arr1.strArray[i], arr2.strArray[i]))
				return 1;
		}
		break;
	case OCREP_PROP_OBJECT:
		for (i = 0; i < len1; i++) {
			if (IC_EQUAL != icd_payload_representation_compare(arr1.objArray[i],
						arr2.objArray[i]))
				return 1;
		}
		break;
	default:
		ERR("Invalid Type (%d)", arr1.type);
		return 1;
	}

	return IC_EQUAL;
}


static int _representation_compare_value(OCRepPayloadValue *value1,
		OCRepPayloadValue *value2)
{
	int ret = 1;

	if (NULL == value1 || NULL == value2)
		return !!(value1 - value2);

	/* compare key */
	if (IC_STR_EQUAL != g_strcmp0(value1->name, value2->name))
		return 1;

	/* compare value */
	if (value1->type != value2->type)
		return 1;

	switch (value1->type) {
	case OCREP_PROP_NULL:
		ret = IC_EQUAL;
		break;
	case OCREP_PROP_INT:
		ret = (value1->i == value2->i) ? IC_EQUAL : 1;
		break;
	case OCREP_PROP_DOUBLE:
		ret = (value1->d == value2->d) ? IC_EQUAL : 1;
		break;
	case OCREP_PROP_BOOL:
		ret = (value1->b == value2->b) ? IC_EQUAL : 1;
		break;
	case OCREP_PROP_STRING:
		ret = (IC_STR_EQUAL == g_strcmp0(value1->str, value2->str)) ? IC_EQUAL : 1;
		break;
	case OCREP_PROP_OBJECT:
		ret = icd_payload_representation_compare(value1->obj, value2->obj);
		break;
	case OCREP_PROP_ARRAY:
		ret = _representation_compare_array(value1->arr, value2->arr);
		break;
	default:
		ERR("Invalid Type (%d)", value1->type);
	}

	return ret;
}


static int _representation_values_list_length(OCRepPayloadValue *value_list)
{
	int len = 0;

	while (value_list) {
		len++;
		value_list = value_list->next;
	}

	return len;
}


static bool _representation_values_list_contain(OCRepPayloadValue *value_list,
		OCRepPayloadValue *value)
{
	OCRepPayloadValue *c;

	for (c = value_list; c; c = c->next) {
		if (IC_EQUAL == _representation_compare_value(c, value))
			return true;
	}

	return false;
}


static int _representation_compare_values_list(OCRepPayloadValue *value1,
		OCRepPayloadValue *value2)
{
	OCRepPayloadValue *c;

	if (NULL == value1 || NULL == value2)
		return !!(value1 - value2);

	if (_representation_values_list_length(value1)
			!= _representation_values_list_length(value2))
		return 1;

	for (c = value1; c; c = c->next) {
		if (false == _representation_values_list_contain(value2, c))
			return 1;
	}

	return IC_EQUAL;
}


static int _representation_compare_without_children(OCRepPayload *repr1,
		OCRepPayload *repr2)
{
	int ret;

	/* compare uri */
	if (IC_STR_EQUAL != g_strcmp0(repr1->uri, repr2->uri))
		return 1;

	/* compare resource types */
	ret = _representation_compare_string_list(repr1->types, repr2->types);
	if (IC_EQUAL != ret)
		return ret;

	/* compare resource interfaces */
	ret = _representation_compare_string_list(repr1->interfaces, repr2->interfaces);
	if (IC_EQUAL != ret)
		return ret;

	/* compare values */
	ret = _representation_compare_values_list(repr1->values, repr2->values);
	if (IC_EQUAL != ret)
		return ret;

	return IC_EQUAL;
}


static int _representation_list_length(OCRepPayload *repr_list)
{
	int len = 0;

	while (repr_list) {
		len++;
		repr_list = repr_list->next;
	}

	return len;
}


static bool _representation_list_contain(OCRepPayload *repr_list, OCRepPayload *repr)
{
	OCRepPayload *c;

	for (c = repr_list; c; c = c->next) {
		if (IC_EQUAL == _representation_compare_without_children(c, repr))
			return true;
	}

	return false;
}


static int _representation_compare_children(OCRepPayload *repr1, OCRepPayload *repr2)
{
	OCRepPayload *c;

	if (NULL == repr1 || NULL == repr2)
		return !!(repr1 - repr2);

	if (_representation_list_length(repr1) != _representation_list_length(repr2))
		return 1;

	for (c = repr1; c; c = c->next) {
		if (false == _representation_list_contain(repr2, c))
			return 1;
	}

	return IC_EQUAL;
}


int icd_payload_representation_compare(OCRepPayload *repr1, OCRepPayload *repr2)
{
	int ret;

	if (NULL == repr1 || NULL == repr2)
		return !!(repr1 - repr2);

	ret = _representation_compare_without_children(repr1, repr2);
	if (IC_EQUAL != ret)
		return ret;

	/* compare childrean */
	ret = _representation_compare_children(repr1->next, repr2->next);
	if (IC_EQUAL != ret)
		return ret;

	return IC_EQUAL;
}
