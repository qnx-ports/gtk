/*
 * Copyright (C) 2023 BlackBerry Limited.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __GDK_QNXSCREEN_DEVICE_DATA_H__
#define __GDK_QNXSCREEN_DEVICE_DATA_H__

#include <screen/screen.h>
#include "gdk/gdk.h"
#include "gdk/gdkdeviceprivate.h"
#include "gdkdevice-qnxscreen-class.h"

struct _GdkQnxScreenDevice
{
    GdkDevice parent_instance;
    screen_window_t window_handle;
    GdkSurface* surface;
    int root_pos[2];
    int window_pos[2];
    int buttons;
};

struct _GdkQnxScreenDeviceClass
{
    GdkDeviceClass parent_class;
};

#endif 
