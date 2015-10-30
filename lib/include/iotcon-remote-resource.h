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
#ifndef __IOT_CONNECTIVITY_MANAGER_CLIENT_REMOTE_RESOURCE_H__
#define __IOT_CONNECTIVITY_MANAGER_CLIENT_REMOTE_RESOURCE_H__

#include <stdbool.h>
#include <iotcon-constant.h>

/**
 * @file iotcon-remote-resource.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_CLIENT_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_CLIENT_REMOTE_RESOURCE_MODULE Remote Resource
 *
 * @brief Iotcon Remote Resource provides API to manage remote resource.
 *
 * @section CAPI_IOT_CONNECTIVITY_CLIENT_REMOTE_RESOURCE_MODULE_HEADER Header
 *  \#include <iotcon.h>
 *
 * @{
 */

/**
 * @brief Creates a new resource handle.
 * @details Creates a resource proxy object so that get/put/observe functionality can be used
 * without discovering the object in advance.\n
 * To use this API, you should provide all of the details required to correctly contact and
 * observe the object.\n
 * If not, you should discover the resource object manually.
 *
 * @since_tizen 3.0
 *
 * @param[in] host_address The host address of the resource
 * @param[in] connectivity_type The connectivity type
 * @param[in] uri_path The URI path of the resource.
 * @param[in] is_observable Allow observation
 * @param[in] resource_types The resource type of the resource. For example, "core.light"
 * @param[in] resource_ifaces The resource interfaces (whether it is collection etc)
 * @param[out] client_handle Generated resource handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_IOTIVITY  Iotivity errors
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_remote_resource_destroy()
 * @see iotcon_remote_resource_clone()
 */
int iotcon_remote_resource_create(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		const char *uri_path,
		bool is_observable,
		iotcon_resource_types_h resource_types,
		int resource_ifaces,
		iotcon_remote_resource_h *client_handle);

/**
 * @brief Releases a resource handle.
 * @details Decrements reference count of the source resource.\n
 * If the reference count drops to 0, releases a resource handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 *
 * @return void
 *
 * @see iotcon_remote_resource_create()
 * @see iotcon_remote_resource_clone()
 */
void iotcon_remote_resource_destroy(iotcon_remote_resource_h resource);

/**
 * @brief Makes a clone of a remote resource.
 *
 * @since_tizen 3.0
 *
 * @param[in] src The Source of resource
 * @param[out] dest The cloned resource handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_remote_resource_create()
 * @see iotcon_remote_resource_destroy()
 */
int iotcon_remote_resource_clone(iotcon_remote_resource_h src, iotcon_remote_resource_h *dest);

/**
 * @brief Specifies the type of function passed to iotcon_remote_resource_observer_start().
 * @details Called when a client receive notifications from a server. The @a response_result could be one of #iotcon_response_result_e.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[in] repr The handle of the representation
 * @param[in] options The handle of the header options
 * @param[in] response_result The response result code
 * @param[in] sequence_number The sequence of notifications from server.
 * @param[in] user_data The user data passed from the callback registration function
 *
 * @pre The callback must be registered using iotcon_remote_resource_observer_start()
 *
 * @see iotcon_remote_resource_observer_start()
 */
typedef void (*iotcon_remote_resource_observe_cb)(iotcon_remote_resource_h resource,
		iotcon_representation_h repr,
		iotcon_options_h options,
		int response_result,
		int sequence_number,
		void *user_data);

/**
 * @brief Sets observation on the resource
 * @details When server sends notification message, iotcon_remote_resource_observe_cb() will be called.
 * The @a observe_type could be one of #iotcon_observe_type_e.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 * @param[in] observe_type The type to specify how client wants to observe.
 * @param[in] query The query to send to server
 * @param[in] cb The callback function to get notifications from server
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_IOTIVITY  Iotivity errors
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the @a resource receive notification message, iotcon_remote_resource_observe_cb() will be called.
 *
 * @see iotcon_remote_resource_observe_cb()
 * @see iotcon_remote_resource_observer_stop()
 * @see iotcon_notimsg_create()
 * @see iotcon_resource_notify()
 */
int iotcon_remote_resource_observer_start(iotcon_remote_resource_h resource,
		iotcon_observe_type_e observe_type,
		iotcon_query_h query,
		iotcon_remote_resource_observe_cb cb,
		void *user_data);

/**
 * @brief Cancels the observation on the resource
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_remote_resource_observe_cb()
 * @see iotcon_remote_resource_observer_start()
 * @see iotcon_notimsg_create()
 * @see iotcon_resource_notify()
 */
int iotcon_remote_resource_observer_stop(iotcon_remote_resource_h resource);

/**
 * @brief Specifies the type of function passed to iotcon_remote_resource_get(), iotcon_remote_resource_put(), iotcon_remote_resource_post()
 * @details The @a response_result could be one of #iotcon_response_result_e.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[in] repr The handle of the representation
 * @param[in] options The handle of the header options
 * @param[in] response_result The response result code (Lesser than 0 on fail, otherwise a response result value)
 * @param[in] user_data The user data to pass to the function
 *
 * @pre The callback must be registered using iotcon_remote_resource_get(), iotcon_remote_resource_put(), iotcon_remote_resource_post()
 *
 * @see iotcon_remote_resource_get()
 * @see iotcon_remote_resource_put()
 * @see iotcon_remote_resource_post()
 */
typedef void (*iotcon_remote_resource_cru_cb)(iotcon_remote_resource_h resource,
		iotcon_representation_h repr,
		iotcon_options_h options,
		int response_result,
		void *user_data);

/**
 * @brief Gets the attributes of a resource.
 * @details When server sends response on get request, iotcon_remote_resource_cru_cb() will be called.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 * @param[in] query The query to send to server
 * @param[in] cb The callback function
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the client receive get response, iotcon_remote_resource_cru_cb() will be called.
 *
 * @see iotcon_remote_resource_cru_cb()
 * @see iotcon_remote_resource_put()
 * @see iotcon_remote_resource_post()
 */
int iotcon_remote_resource_get(iotcon_remote_resource_h resource, iotcon_query_h query,
		iotcon_remote_resource_cru_cb cb, void *user_data);

/**
 * @brief Sets the representation of a resource (via PUT)
 * @details When server sends response on put request, iotcon_remote_resource_cru_cb() will be called.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 * @param[in] repr The handle of the representation
 * @param[in] query The query to send to server
 * @param[in] cb The callback function
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the client receive put response, iotcon_remote_resource_cru_cb() will be called.
 *
 * @see iotcon_remote_resource_cru_cb()
 * @see iotcon_remote_resource_get()
 * @see iotcon_remote_resource_post()
 */
int iotcon_remote_resource_put(iotcon_remote_resource_h resource,
		iotcon_representation_h repr,
		iotcon_query_h query,
		iotcon_remote_resource_cru_cb cb,
		void *user_data);

/**
 * @brief Posts on a resource
 * @details When server sends response on post request, iotcon_remote_resource_cru_cb() will be called.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 * @param[in] repr The handle of the representation
 * @param[in] query The query to send to server
 * @param[in] cb The callback function
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the client receive post response, iotcon_remote_resource_cru_cb() will be called.
 *
 * @see iotcon_remote_resource_cru_cb()
 * @see iotcon_remote_resource_get()
 * @see iotcon_remote_resource_put()
 */
int iotcon_remote_resource_post(iotcon_remote_resource_h resource,
		iotcon_representation_h repr,
		iotcon_query_h query,
		iotcon_remote_resource_cru_cb cb,
		void *user_data);

/**
 * @brief Specifies the type of function passed to iotcon_remote_resource_delete()
 * @details The @a response_result could be one of #iotcon_response_result_e.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[in] options The handle of the header options
 * @param[in] response_result The response result code
 * @param[in] user_data The user data to pass to the function
 *
 * @pre The callback must be registered using iotcon_remote_resource_delete()
 *
 * @see iotcon_remote_resource_delete()
 */
typedef void (*iotcon_remote_resource_delete_cb)(iotcon_remote_resource_h resource,
		iotcon_options_h options,
		int response_result,
		void *user_data);

/**
 * @brief Deletes a resource.
 * @details When server sends response on delete request, iotcon_remote_resource_delete_cb() will be called.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 * @param[in] cb The callback function
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the client receive delete response, iotcon_remote_resource_delete_cb() will be called.
 *
 * @see iotcon_remote_resource_delete_cb()
 */
int iotcon_remote_resource_delete(iotcon_remote_resource_h resource,
		iotcon_remote_resource_delete_cb cb, void *user_data);

/**
 * @brief Specifies the type of function passed to iotcon_remote_resource_start_caching().
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the remote resource
 * @param[in] representation The handle of the representation
 * @param[in] user_data The user data to pass to the function
 *
 * @pre The callback must be registered using iotcon_remote_resource_start_caching()\n
 *
 * @see iotcon_remote_resource_start_caching()
 * @see iotcon_remote_resource_stop_caching()
 */
typedef void (*iotcon_remote_resource_cached_representation_changed_cb)(
		iotcon_remote_resource_h resource,
		iotcon_representation_h representation,
		void *user_data);

/**
 * @brief Start caching of a remote resource.
 * @details Use this function to start caching the resource's attribute data (state).\n
 * Default caching time interval is 10 seconds.
 * Internally, it operates GET method, periodically, and it observes the remote resource.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the remote resource to be cached
 * @param[in] caching_interval Seconds for caching time interval.\n
 * If value is 0, then it sets 10 seconds(default caching time).
 * @param[in] cb The callback function to add into callback list
 * @param[in] user_data The user data to pass to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_ALREADY Already done
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_remote_resource_stop_caching()
 * @see iotcon_remote_resource_cached_representation_changed_cb()
 */
int iotcon_remote_resource_start_caching(iotcon_remote_resource_h resource,
		int caching_interval,
		iotcon_remote_resource_cached_representation_changed_cb cb,
		void *user_data);

/**
 * @brief Stop caching of a remote resource.
 * @details Use this function to stop caching the resource's attribute data (state).\n
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the remote resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_remote_resource_start_caching()
 * @see iotcon_remote_resource_cached_representation_changed_cb()
 */
int iotcon_remote_resource_stop_caching(iotcon_remote_resource_h resource);

/**
 * @brief Specifies the type of function passed to iotcon_remote_resource_start_monitoring().
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the remote resource
 * @param[in] state The state of the remote resource
 * @param[in] user_data The user data to pass to the function
 *
 * @pre The callback must be registered using iotcon_remote_resource_start_monitoring()\n
 *
 * @see iotcon_remote_resource_start_monitoring()
 * @see iotcon_remote_resource_stop_monitoring()
 */
typedef void (*iotcon_remote_resource_state_changed_cb)(iotcon_remote_resource_h resource,
		iotcon_remote_resource_state_e state, void *user_data);

/**
 * @brief Start monitoring of a remote resource.
 * @details When remote resource's state are changed, registered callbacks will be called\n
 * in turn. Default monitoring time interval is 10 seconds.
 * Internally, it operates GET method, periodically, and it subscribes the devices's presence.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the remote resource
 * @param[in] monitoring_interval Seconds for monitoring time interval.\n
 * If value is 0, then it sets 10 seconds(default monitoring time).
 * @param[in] cb The callback function to add into callback list
 * @param[in] user_data The user data to pass to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_ALREADY  Already done
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_remote_resource_stop_monitoring()
 * @see iotcon_remote_resource_state_changed_cb()
 */
int iotcon_remote_resource_start_monitoring(iotcon_remote_resource_h resource,
		int monitoring_interval,
		iotcon_remote_resource_state_changed_cb cb,
		void *user_data);

/**
 * @brief Stop monitoring.
 * @details Use this function to stop monitoring the remote resource.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the remote resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_remote_resource_start_monitoring()
 * @see iotcon_remote_resource_state_changed_cb()
 */
int iotcon_remote_resource_stop_monitoring(iotcon_remote_resource_h resource);

/**
 * @brief Gets an URI path of the remote resource
 *
 * @since_tizen 3.0
 * @remarks @a uri_path must not be released using free().
 *
 * @param[in] resource The handle of the remote resource
 * @param[out] uri_path The URI path of the remote resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_remote_resource_get_host_address()
 * @see iotcon_remote_resource_get_connectivity_type()
 * @see iotcon_remote_resource_get_device_id()
 * @see iotcon_remote_resource_get_types()
 * @see iotcon_remote_resource_get_interfaces()
 * @see iotcon_remote_resource_is_observable()
 * @see iotcon_remote_resource_set_options()
 */
int iotcon_remote_resource_get_uri_path(iotcon_remote_resource_h resource,
		char **uri_path);

/**
 * @brief Gets connectivity type of the remote resource
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the remote resource
 * @param[out] connectivity_type The connectivity type of the remote resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter

 * @see iotcon_remote_resource_get_uri_path()
 * @see iotcon_remote_resource_get_host_address()
 * @see iotcon_remote_resource_get_connectivity_type()
 * @see iotcon_remote_resource_get_device_id()
 * @see iotcon_remote_resource_get_types()
 * @see iotcon_remote_resource_get_interfaces()
 * @see iotcon_remote_resource_is_observable()
 * @see iotcon_remote_resource_set_options()
 */
int iotcon_remote_resource_get_connectivity_type(iotcon_remote_resource_h resource,
		int *connectivity_type);

/**
 * @brief Gets an host address of the remote resource
 *
 * @since_tizen 3.0
 * @remarks @a host_address must not be released using free().
 *
 * @param[in] resource The handle of the remote resource
 * @param[out] host_address The host address of the remote resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_remote_resource_get_connectivity_type()
 * @see iotcon_remote_resource_get_uri_path()
 * @see iotcon_remote_resource_get_device_id()
 * @see iotcon_remote_resource_get_types()
 * @see iotcon_remote_resource_get_interfaces()
 * @see iotcon_remote_resource_is_observable()
 * @see iotcon_remote_resource_set_options()
 */
int iotcon_remote_resource_get_host_address(iotcon_remote_resource_h resource,
		char **host_address);

/**
 * @brief Gets an device id of the remote resource
 *
 * @since_tizen 3.0
 * @remarks @a device_id must not be released using free().
 *
 * @param[in] resource The handle of the remote resource
 * @param[out] device_id The device id of the remote resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_remote_resource_get_uri_path()
 * @see iotcon_remote_resource_get_host_address()
 * @see iotcon_remote_resource_get_connectivity_type()
 * @see iotcon_remote_resource_get_types()
 * @see iotcon_remote_resource_get_interfaces()
 * @see iotcon_remote_resource_is_observable()
 * @see iotcon_remote_resource_set_options()
 */
int iotcon_remote_resource_get_device_id(iotcon_remote_resource_h resource,
		char **device_id);

/**
 * @brief Gets resource types of the remote resource
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the remote resource
 * @param[out] types The resource types of the remote resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_remote_resource_get_uri_path()
 * @see iotcon_remote_resource_get_host_address()
 * @see iotcon_remote_resource_get_connectivity_type()
 * @see iotcon_remote_resource_get_device_id()
 * @see iotcon_remote_resource_get_interfaces()
 * @see iotcon_remote_resource_is_observable()
 * @see iotcon_remote_resource_set_options()
 */
int iotcon_remote_resource_get_types(iotcon_remote_resource_h resource,
		iotcon_resource_types_h *types);

/**
 * @brief Gets resource interfaces of the remote resource
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the remote resource
 * @param[out] ifaces The resource interfaces of the remote resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_remote_resource_get_uri_path()
 * @see iotcon_remote_resource_get_host_address()
 * @see iotcon_remote_resource_get_connectivity_type()
 * @see iotcon_remote_resource_get_device_id()
 * @see iotcon_remote_resource_get_types()
 * @see iotcon_remote_resource_is_observable()
 * @see iotcon_remote_resource_set_options()
 */
int iotcon_remote_resource_get_interfaces(iotcon_remote_resource_h resource, int *ifaces);

/**
 * @brief Checks whether the remote resource is observable or not.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[out] observable The value of observable
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_remote_resource_get_uri_path()
 * @see iotcon_remote_resource_get_host_address()
 * @see iotcon_remote_resource_get_connectivity_type()
 * @see iotcon_remote_resource_get_device_id()
 * @see iotcon_remote_resource_get_types()
 * @see iotcon_remote_resource_get_interfaces()
 * @see iotcon_remote_resource_set_options()
 */
int iotcon_remote_resource_is_observable(iotcon_remote_resource_h resource,
		bool *observable);

/**
 * @brief Sets options into the remote resource
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the remote resource
 * @param[in] options The handle of the header options
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_remote_resource_get_uri_path()
 * @see iotcon_remote_resource_get_host_address()
 * @see iotcon_remote_resource_get_connectivity_type()
 * @see iotcon_remote_resource_get_device_id()
 * @see iotcon_remote_resource_get_types()
 * @see iotcon_remote_resource_get_interfaces()
 * @see iotcon_remote_resource_is_observable()
 */
int iotcon_remote_resource_set_options(iotcon_remote_resource_h resource,
		iotcon_options_h options);

/**
 * @brief Get cached representation from the remote resource
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the remote resource
 * @param[out] representation The handle of the representation
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data
 */
int iotcon_remote_resource_get_cached_representation(
		iotcon_remote_resource_h resource,
		iotcon_representation_h *representation);

/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_MANAGER_CLIENT_REMOTE_RESOURCE_H__ */