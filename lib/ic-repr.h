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
#ifndef __IOT_CONNECTIVITY_MANAGER_INTERNAL_REPRESENTATION_H__
#define __IOT_CONNECTIVITY_MANAGER_INTERNAL_REPRESENTATION_H__

#include <json-glib/json-glib.h>

#include "ic-struct.h"

#define IOTCON_KEY_OC "oc"
#define IOTCON_KEY_URI "href"
#define IOTCON_KEY_RESOURCETYPES "rt"
#define IOTCON_KEY_INTERFACES "if"
#define IOTCON_KEY_PROPERTY "prop"
#define IOTCON_KEY_REP "rep"

int ic_repr_get_value(iotcon_repr_h repr, const char *key, iotcon_value_h *retval);
int ic_repr_replace_value(iotcon_repr_h repr, const char *key, iotcon_value_h value);

JsonObject* ic_repr_generate_json_repr(iotcon_repr_h repr, int *err_code);

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @brief Generates JSON string from Representation.
 * @since_tizen 3.0
 * @remarks The returned string must be released with free() by you.
 *
 * @param[in] repr The handle to the Representation
 * @param[in] set_pretty Whether the generated JSON string should be pretty printed
 *
 * @return Generated JSON string, otherwise a null pointer if a parse error
 */
char* ic_repr_generate_json(iotcon_repr_h repr, bool set_pretty);

iotcon_repr_h ic_repr_parse_json_obj(JsonObject *obj, int *err_code);
iotcon_repr_h ic_repr_parse_json(const char *json_string);

#endif // __IOT_CONNECTIVITY_MANAGER_INTERNAL_REPRESENTATION_H__