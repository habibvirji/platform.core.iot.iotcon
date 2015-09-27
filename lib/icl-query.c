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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

#include "iotcon-struct.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-resource-types.h"
#include "icl-query.h"

API int iotcon_query_create(iotcon_query_h *ret_query)
{
	RETV_IF(NULL == ret_query, IOTCON_ERROR_INVALID_PARAMETER);

	iotcon_query_h query = calloc(1, sizeof(struct icl_query));
	if (NULL == query) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	query->hash = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);

	*ret_query = query;

	return IOTCON_ERROR_NONE;
}


API void iotcon_query_destroy(iotcon_query_h query)
{
	RET_IF(NULL == query);

	g_hash_table_unref(query->hash);
	free(query);
}


/* The full length of query should be less than or equal to 64. */
API int iotcon_query_insert(iotcon_query_h query, const char *key, const char *value)
{
	int query_len;

	RETV_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);

	/* first query : ?key=value
	 * Rest of query : &key=value */
	query_len = strlen(key) + strlen(value) + 2;
	if (ICL_QUERY_LENGTH_MAX < (query->len + query_len)) {
		ERR("Length of query is invalid.");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	g_hash_table_insert(query->hash, ic_utils_strdup(key), ic_utils_strdup(value));
	query->len += query_len;

	return IOTCON_ERROR_NONE;
}


API int iotcon_query_delete(iotcon_query_h query, const char *key)
{
	gboolean is_removed;
	int query_len;
	char *value;

	RETV_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(query->hash, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	query_len = strlen(key) + strlen(value) + 2;

	is_removed = g_hash_table_remove(query->hash, key);
	if (FALSE == is_removed) {
		ERR("g_hash_table_remove() Fail");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	query->len -= query_len;

	return IOTCON_ERROR_NONE;
}


API int iotcon_query_lookup(iotcon_query_h query, const char *key, char **data)
{
	char *value = NULL;

	RETV_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == data, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(query->hash, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	*data = value;

	return IOTCON_ERROR_NONE;
}

API int iotcon_query_foreach(iotcon_query_h query, iotcon_query_foreach_cb cb,
		void *user_data)
{
	GHashTableIter iter;
	gpointer key, value;

	RETV_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	g_hash_table_iter_init(&iter, query->hash);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		if (false == cb(key, value, user_data))
			break;
	}

	return IOTCON_ERROR_NONE;
}

