#include <glib.h>
#include <iotcon.h>
#include "test.h"

int main()
{
	int ret;
	GMainLoop *loop;

	iotcon_platform_info_s platform_info = {0};

	platform_info.platform_id = "platform_id";
	platform_info.manuf_name = "manuf_name";
	platform_info.manuf_url = "manuf_url";
	platform_info.model_number = "model_number";
	platform_info.date_of_manufacture = "date_of_manufacture";
	platform_info.platform_ver = "platform_ver";
	platform_info.os_ver = "os_ver";
	platform_info.hardware_ver = "hardware_ver";
	platform_info.firmware_ver = "firmware_ver";
	platform_info.support_url = "support_url";
	platform_info.system_time = "system_time";

	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon initialize */
	iotcon_initialize();

	ret = iotcon_register_platform_info(platform_info);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_register_platform_info() Fail(%d)", ret);
		return -1;
	}

	g_main_loop_run(loop);

	g_main_loop_unref(loop);

	/* iotcon deinitialize */
	iotcon_deinitialize();

	return 0;
}




