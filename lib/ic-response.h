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
#ifndef __IOT_CONNECTIVITY_MANAGER_INTERNAL_RESPONSE_H__
#define __IOT_CONNECTIVITY_MANAGER_INTERNAL_RESPONSE_H__

#include "iotcon-struct.h"
#include "iotcon-constant.h"
#include "ic-request.h"

struct ic_resource_response {
	char *new_uri;
	int error_code;
	iotcon_options_h header_options;
	iotcon_interface_e iface;
	iotcon_response_result_e result;
	iotcon_repr_h repr;
	oc_request_h request_handle;
	oc_resource_h resource_handle;
};

#endif /* __IOT_CONNECTIVITY_MANAGER_INTERNAL_RESPONSE_H__ */