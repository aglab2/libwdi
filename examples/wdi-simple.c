/*
* wdi-simple.c: Console Driver Installer for a single USB device
* Copyright (c) 2010-2018 Pete Batard <pete@akeo.ie>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 3 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libwdi.h"

#if defined(_PREFAST_)
/* Disable "Banned API Usage:" errors when using WDK's OACR/Prefast */
#pragma warning(disable:28719)
/* Disable "Consider using 'GetTickCount64' instead of 'GetTickCount'" when using WDK's OACR/Prefast */
#pragma warning(disable:28159)
#endif

#define oprintf(...) do {if (!opt_silent) printf(__VA_ARGS__);} while(0)

/*
 * Change these values according to your device if
 * you don't want to provide parameters
 */
#define DESC        "WUP-028"
#define VID         0x057e
#define PID         0x0337
#define INF_NAME    "usb_device.inf"
#define DEFAULT_DIR NULL

int __cdecl main(int argc, char** argv)
{
	static struct wdi_device_info *ldev, dev = {NULL, VID, PID, FALSE, 0, DESC, NULL, NULL, NULL};
	static struct wdi_options_create_list ocl = { 0 };
	static struct wdi_options_prepare_driver opd = { 0 };
	static struct wdi_options_install_driver oid = { 0 };
	static struct wdi_options_install_cert oic = { 0 };

#ifdef DEBUG
    static int log_level = WDI_LOG_LEVEL_DEBUG;
#else
    static int log_level = WDI_LOG_LEVEL_WARNING;
#endif
    static int opt_silent = 0, opt_extract = 0;
	static BOOL matching_device_found;
	int r;
	char *inf_name = INF_NAME;
	char *ext_dir = DEFAULT_DIR;
	char *cert_name = NULL;

	ocl.list_all = TRUE;
	ocl.list_hubs = TRUE;
	ocl.trim_whitespaces = TRUE;
	opd.driver_type = WDI_WINUSB;

	wdi_set_log_level(log_level);

	oprintf("Extracting driver files...\n");
	r = wdi_prepare_driver(&dev, ext_dir, inf_name, &opd);
	oprintf("  %s\n", wdi_strerror(r));
	if ((r != WDI_SUCCESS) || (opt_extract))
		return r;

	if (cert_name != NULL) {
		oprintf("Installing certificate '%s' as a Trusted Publisher...\n", cert_name);
		r = wdi_install_trusted_certificate(cert_name, &oic);
		oprintf("  %s\n", wdi_strerror(r));
	}

	oprintf("Installing driver(s)...\n");

	// Try to match against a plugged device to avoid device manager prompts
	matching_device_found = FALSE;
	if (wdi_create_list(&ldev, &ocl) == WDI_SUCCESS)
    {
		r = WDI_SUCCESS;
		for (; (ldev != NULL) && (r == WDI_SUCCESS); ldev = ldev->next)
        {
			if ( (ldev->vid == dev.vid) && (ldev->pid == dev.pid) && (ldev->mi == dev.mi) &&(ldev->is_composite == dev.is_composite) )
            {
				
				dev.hardware_id = ldev->hardware_id;
				dev.device_id = ldev->device_id;
				matching_device_found = TRUE;
				oprintf("WiiU Adapter FOUND!\n");
                oprintf("Driver is getting installed...\n");
				fflush(stdout);
				r = wdi_install_driver(&dev, ext_dir, inf_name, &oid);
				oprintf("%s: %s\n", dev.hardware_id, wdi_strerror(r));
			}
		}
	}

	// No plugged USB device matches this one -> install driver
	if (!matching_device_found) {
		r = wdi_install_driver(&dev, ext_dir, inf_name, &oid);
		oprintf("  %s\n", wdi_strerror(r));
	}

	return r;
}
