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
#ifndef __IOT_CONNECTIVITY_MANAGER_SERVER_RESOURCE_H__
#define __IOT_CONNECTIVITY_MANAGER_SERVER_RESOURCE_H__

#include <iotcon-types.h>

/**
 * @file iotcon-resource.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_SERVER_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_SERVER_RESOURCE_MODULE Resource
 *
 * @brief Iotcon Resource provides API to manage resource.
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_RESOURCE_MODULE_HEADER Required Header
 *  \#include <iotcon.h>
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_RESOURCE_MODULE_OVERVIEW Overview
 * The iotcon resource API provides methods for managing handle and resource information.
 *
 * Example :
 * @code
#include <iotcon.h>

static iotcon_resource_h _resource_room;

static void _room_request_handler(iotcon_resource_h resource, iotcon_request_h request, void *user_data)
{
	// handle request
	...
}

static void _door_request_handler(iotcon_resource_h resource, iotcon_request_h request, void *user_data)
{
	// handle request
	...
}

static void _create_resource()
{
	int ret;
	int ifaces;
	int properties;
	iotcon_resource_types_h resource_types = NULL;
	iotcon_resource_h resource_door = NULL;

	// 1. create room resource
	ifaces = IOTCON_INTERFACE_DEFAULT | IOTCON_INTERFACE_LINK | IOTCON_INTERFACE_BATCH;
	properties = IOTCON_RESOURCE_DISCOVERABLE | IOTCON_RESOURCE_OBSERVABLE;

	ret = iotcon_resource_types_create(&resource_types);
	if (IOTCON_ERROR_NONE != ret)
		return;

	ret = iotcon_resource_types_add(resource_types, "org.tizen.room");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	ret = iotcon_resource_create("/room/1", resource_types, ifaces,
			properties, _room_request_handler, NULL, &_resource_room);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		return;
	}
	iotcon_resource_types_destroy(resource_types);

	// 2. create door resource
	ifaces = IOTCON_INTERFACE_DEFAULT;
	properties = IOTCON_RESOURCE_OBSERVABLE;

	ret = iotcon_resource_types_create(&resource_types);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_destroy(_resource_room);
		_resource_room = NULL;
		return;
	}

	ret = iotcon_resource_types_add(resource_types, "org.tizen.door");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		iotcon_resource_destroy(_resource_room);
		_resource_room = NULL;
		return;
	}

	ret = iotcon_resource_create("/door/1", resource_types, ifaces,
			properties, _door_request_handler, NULL, &resource_door);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		iotcon_resource_destroy(_resource_room);
		_resource_room = NULL;
		return;
	}
	iotcon_resource_types_destroy(resource_types);

	// 3. bind door resouce to room resource
	ret = iotcon_resource_bind_child_resource(_resource_room, resource_door);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_destroy(resource_door);
		iotcon_resource_destroy(_resource_room);
		_resource_room = NULL;
		return;
	}
}
 * @endcode
 *
 * @{
 */

/**
 * @brief Specifies the type of function passed to iotcon_resource_create() and
 * iotcon_resource_set_request_handler()
 * @details Called when server receive request from the client.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The resource requested
 * @param[in] request The request from client
 * @param[in] user_data The user data to pass to the function
 *
 * @pre The callback must be registered using iotcon_resource_create()
 *
 * @see iotcon_resource_create()
 * @see iotcon_resource_set_request_handler()
 */
typedef void (*iotcon_request_handler_cb)(iotcon_resource_h resource,
		iotcon_request_h request, void *user_data);

/**
 * @brief Creates a resource handle and registers the resource in server
 * @details Registers a resource specified by @a uri_path, @a res_types, @a ifaces which have
 * @a properties in Iotcon server.\n
 * When client find the registered resource, iotcon_request_handler_cb() will be called automatically.\n
 * @a uri_path format would be relative URI path like '/a/light'\n
 * @a res_types is a list of resource types. Create a iotcon_resource_types_h handle and
 * add types string to it.\n
 * @a ifaces can contain multiple interfaces like
 * IOTCON_INTERFACE_LINK | IOTCON_INTERFACE_BATCH.\n
 * @a properties also can contain multiple properties like
 * IOTCON_RESOURCE_ACTIVE | IOTCON_RESOURCE_DISCOVERABLE.\n
 * iotcon_request_handler_cb() will be called when receive CRUD request to the registered
 * resource.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @remarks @a uri_path length must be less than or equal 36.\n
 * You must destroy @a resource by calling iotcon_resource_destroy()
 * if @a resource is no longer needed.
 *
 * @param[in] uri_path The URI path of the resource.
 * @param[in] res_types The list of type of the resource.
 * @param[in] ifaces The interfaces of the resource.
 * @param[in] properties The property of the resource.
 * @param[in] cb The request handler callback function
 * @param[in] user_data The user data to pass to the callback function
 * @param[out] resource_handle The handle of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_IOTIVITY  Iotivity errors
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the resource receive CRUD request, iotcon_request_handler_cb() will be called.
 *
 * @see iotcon_resource_destroy()
 * @see iotcon_resource_bind_interface()
 * @see iotcon_resource_bind_type()
 * @see iotcon_resource_set_request_handler()
 * @see iotcon_resource_bind_child_resource()
 * @see iotcon_resource_unbind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_resource_create(const char *uri_path,
		iotcon_resource_types_h res_types,
		int ifaces,
		int properties,
		iotcon_request_handler_cb cb,
		void *user_data,
		iotcon_resource_h *resource_handle);

/**
 * @brief Destroys the resource and releases its data.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @remarks When a normal variable is used, there are only dbus error and permission\n
 * denied error. If the errors of this API are not handled, then you must check\n
 * whether dbus is running and an application have the privileges for the API.
 *
 * @param[in] resource_handle The handle of the resource to be unregistered
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_resource_create()
 * @see iotcon_resource_bind_interface()
 * @see iotcon_resource_bind_type()
 * @see iotcon_resource_set_request_handler()
 * @see iotcon_resource_bind_child_resource()
 * @see iotcon_resource_unbind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_resource_destroy(iotcon_resource_h resource_handle);

/**
 * @brief Binds an interface to the resource
 *
 * @details The @a iface could be one of #iotcon_interface_e.
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @remarks Sets only one interface to @a iface. If not, @a iface will be ignored.
 *
 * @param[in] resource The handle of the resource
 * @param[in] iface The interface to be bound to the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_resource_create()
 * @see iotcon_resource_destroy()
 * @see iotcon_resource_bind_type()
 * @see iotcon_resource_set_request_handler()
 * @see iotcon_resource_bind_child_resource()
 * @see iotcon_resource_unbind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_resource_bind_interface(iotcon_resource_h resource, iotcon_interface_e iface);

/**
 * @brief Binds a type to the resource
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @remarks The length of @a resource_type should be less than or equal to 61.\n
 * The @a resource_type must start with a lowercase alphabetic character, followed by a sequence
 * of lowercase alphabetic, numeric, ".", or "-" characters, and contains no white space.\n
 *
 * @param[in] resource_handle The handle of the resource
 * @param[in] resource_type The type to be bound to the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_resource_create()
 * @see iotcon_resource_destroy()
 * @see iotcon_resource_bind_interface()
 * @see iotcon_resource_set_request_handler()
 * @see iotcon_resource_bind_child_resource()
 * @see iotcon_resource_unbind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_resource_bind_type(iotcon_resource_h resource_handle,
		const char *resource_type);

/**
 * @brief Binds a request handler to the resource
 * @details When the resource receive CRUD request, iotcon_request_handler_cb() will be
 * called.
 *
 * @since_tizen 3.0
 *
 * @remarks Registered callback function will be replaced with the new @a cb.\n
 *
 * @param[in] resource The handle of the resource
 * @param[in] cb The request handler to be bound to the resource
 * @param[in] user_data The user data to pass to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_create()
 * @see iotcon_resource_destroy()
 * @see iotcon_resource_bind_interface()
 * @see iotcon_resource_bind_type()
 * @see iotcon_resource_bind_child_resource()
 * @see iotcon_resource_unbind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_resource_set_request_handler(iotcon_resource_h resource,
		iotcon_request_handler_cb cb, void *user_data);

/**
 * @brief Binds a child resource into the parent resource.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] parent The handle of the parent resource
 * @param[in] child The handle of the child resource to be added to the parent resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_ALREADY  Already done
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_resource_create()
 * @see iotcon_resource_destroy()
 * @see iotcon_resource_bind_interface()
 * @see iotcon_resource_bind_type()
 * @see iotcon_resource_set_request_handler()
 * @see iotcon_resource_unbind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_resource_bind_child_resource(iotcon_resource_h parent,
		iotcon_resource_h child);

/**
 * @brief Unbinds a child resource from the parent resource.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] parent The handle of the parent resource
 * @param[in] child The handle of the child resource to be unbound from the parent resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_resource_create()
 * @see iotcon_resource_destroy()
 * @see iotcon_resource_bind_interface()
 * @see iotcon_resource_bind_type()
 * @see iotcon_resource_set_request_handler()
 * @see iotcon_resource_bind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_resource_unbind_child_resource(iotcon_resource_h parent,
		iotcon_resource_h child);

/**
 * @brief Notifies specific clients that resource's attributes have changed.
 * @details If @a observers is @c NULL, the @a msg will notify to all observers.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 * @param[in] repr The handle of the representation
 * @param[in] observers The handle of the observers.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_REPRESENTATION  Representation error
 * @retval #IOTCON_ERROR_SYSTEM  System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_remote_resource_response_cb()
 * @see iotcon_remote_resource_observe_register()
 * @see iotcon_remote_resource_observe_deregister()
 * @see iotcon_observers_create()
 * @see iotcon_observers_destroy()
 * @see iotcon_observers_add()
 * @see iotcon_observers_remove()
 */
int iotcon_resource_notify(iotcon_resource_h resource, iotcon_representation_h repr,
		iotcon_observers_h observers);

/**
 * @brief Gets the number of children resources of the resource
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[out] number The number of children resources
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_get_nth_child()
 * @see iotcon_resource_get_uri_path()
 * @see iotcon_resource_get_types()
 * @see iotcon_resource_get_interfaces()
 * @see iotcon_resource_get_properties()
 */
int iotcon_resource_get_number_of_children(iotcon_resource_h resource, int *number);

/**
 * @brief Gets the child resource at the given index in the parent resource
 *
 * @since_tizen 3.0
 *
 * @remarks @a child must not be released using iotcon_resource_destroy().
 *
 * @param[in] parent The handle of the parent resource
 * @param[in] index The index of the child resource
 * @param[out] child The child resource at the index
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_get_number_of_children()
 * @see iotcon_resource_get_uri_path()
 * @see iotcon_resource_get_types()
 * @see iotcon_resource_get_interfaces()
 * @see iotcon_resource_get_properties()
 */
int iotcon_resource_get_nth_child(iotcon_resource_h parent, int index,
		iotcon_resource_h *child);

/**
 * @brief Gets an URI path of the resource
 *
 * @since_tizen 3.0
 *
 * @remarks @a uri_path must not be released using free().
 *
 * @param[in] resource The handle of the resource
 * @param[out] uri_path The URI path of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_get_number_of_children()
 * @see iotcon_resource_get_nth_child()
 * @see iotcon_resource_get_types()
 * @see iotcon_resource_get_interfaces()
 * @see iotcon_resource_get_properties()
 */
int iotcon_resource_get_uri_path(iotcon_resource_h resource, char **uri_path);

/**
 * @brief Gets the list of types in the resource
 *
 * @since_tizen 3.0
 *
 * @remarks @a types must not be released using iotcon_resource_types_destroy().
 *
 * @param[in] resource The handle of the resource
 * @param[out] types The types of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_get_number_of_children()
 * @see iotcon_resource_get_nth_child()
 * @see iotcon_resource_get_uri_path()
 * @see iotcon_resource_get_interfaces()
 * @see iotcon_resource_get_properties()
 */
int iotcon_resource_get_types(iotcon_resource_h resource, iotcon_resource_types_h *types);

/**
 * @brief Gets the interfaces of the resource
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[out] ifaces The interfaces of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_get_number_of_children()
 * @see iotcon_resource_get_nth_child()
 * @see iotcon_resource_get_uri_path()
 * @see iotcon_resource_get_types()
 * @see iotcon_resource_get_properties()
 */
int iotcon_resource_get_interfaces(iotcon_resource_h resource, int *ifaces);

/**
 * @brief Checks whether the resource is observable or not.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[out] properties The properties of resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_get_number_of_children()
 * @see iotcon_resource_get_nth_child()
 * @see iotcon_resource_get_uri_path()
 * @see iotcon_resource_get_types()
 * @see iotcon_resource_get_interfaces()
 */
int iotcon_resource_get_properties(iotcon_resource_h resource, int *properties);

/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_MANAGER_SERVER_RESOURCE_H__ */
