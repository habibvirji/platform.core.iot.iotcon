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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_UTILITY_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_UTILITY_H__

#include <glib.h>

#include "icl-resource-types.h"

const char** icl_dbus_resource_types_to_array(iotcon_resource_types_h types);
GVariant* icl_dbus_notimsg_to_gvariant(struct icl_notify_msg *msg);
GVariant* icl_dbus_response_to_gvariant(struct icl_resource_response *response);
GVariant* icl_dbus_client_to_gvariant(struct icl_remote_resource *resource);
#ifdef DEVICE_INFO_IMPL /* not implemented in iotivity 0.9.1 */
GVariant* icl_dbus_device_info_to_gvariant(iotcon_device_info_s *device_info);
#endif
GVariant* icl_dbus_platform_info_to_gvariant(iotcon_platform_info_s *platform_info);
GVariant* icl_dbus_query_to_gvariant(iotcon_query_h query);
GVariant* icl_dbus_options_to_gvariant(iotcon_options_h options);
GVariant* icl_dbus_observers_to_gvariant(iotcon_observers_h observers);

#endif /* __IOT_CONNECTIVITY_MANAGER_LIBRARY_UTILITY_H__ */
