/*
 * Copyright (C) 2017 La Ode Muh. Fadlun Akbar <fadlun.net@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "device.h"


static void populate_usb_devices(void);
int total_usb_device(void);
static void cleanup_device_usage(void);
GSList* usb_devices_list(void);
static UsbDevice* get_devices(void);

struct udev* udev;
struct udev_enumerate* enumerate;
struct udev_list_entry *devices, *dev_list_entry;

static void
populate_usb_devices(void)
{
    udev = udev_new();
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "usb");
    udev_enumerate_add_nomatch_sysattr(enumerate, "bDeviceClass", "09");
    udev_enumerate_add_nomatch_sysattr(enumerate, "bInterfaceNumber", NULL);
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
}

int
total_usb_device(void)
{
    int total = 0;

    populate_usb_devices();
    udev_list_entry_foreach(dev_list_entry, devices) {
        total++;
    }

    cleanup_device_usage();
    return total;
}

static void
cleanup_device_usage(void)
{
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}

GSList*
usb_devices_list(void)
{
    GSList* usb_dev_list = NULL;
    UsbDevice* usb_list = NULL;
    int total = 0;

    total = total_usb_device();
    usb_list = get_devices();

    for (int i = 0; i < total; i++) {
        usb_dev_list = g_slist_append(usb_dev_list, (usb_list + i));
    }

    return usb_dev_list;
}

static UsbDevice*
get_devices(void)
{
    int index = 0;
    int total = 0;
    struct udev_device* dev;

    total = total_usb_device();
    populate_usb_devices();
    UsbDevice* USBDevInfo = (UsbDevice*)malloc(total * sizeof(UsbDevice));
    udev_list_entry_foreach(dev_list_entry, devices)
    {
        USBDevInfo[index].path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, USBDevInfo[index].path);

        USBDevInfo[index].idVendor = udev_device_get_sysattr_value(dev, "idVendor");
        USBDevInfo[index].idProduct = udev_device_get_sysattr_value(dev, "idProduct");
        USBDevInfo[index].bConfValue =
          udev_device_get_sysattr_value(dev, "bConfigurationValue");
        USBDevInfo[index].bNumIntfs =
          udev_device_get_sysattr_value(dev, "bNumInterfaces");
        USBDevInfo[index].busid = udev_device_get_sysname(dev);
        USBDevInfo[index].manufact =
          udev_device_get_sysattr_value(dev, "manufacturer");
        USBDevInfo[index].product_usb = udev_device_get_sysattr_value(dev, "product");

        if (!USBDevInfo[index].idVendor || !USBDevInfo[index].idProduct ||
            !USBDevInfo[index].bConfValue || !USBDevInfo[index].bNumIntfs) {
            err("problem getting device attributes: %s", strerror(errno));
            goto err_out;
        }

        index++;
    }

    cleanup_device_usage();
    return USBDevInfo;

err_out:
    cleanup_device_usage();
    return NULL;
}