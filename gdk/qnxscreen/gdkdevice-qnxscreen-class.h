/*
 * Copyright (C) 2024 BlackBerry Limited.
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

#ifndef __GDK_QNXSCREEN_DEVICE_CLASS_H__
#define __GDK_QNXSCREEN_DEVICE_CLASS_H__

#include "gdk/gdk.h"

typedef struct _GdkQnxScreenDevice GdkQnxScreenDevice;
typedef struct _GdkQnxScreenDeviceClass GdkQnxScreenDeviceClass;

#define GDK_TYPE_QNXSCREEN_DEVICE (gdk_qnxscreen_device_get_type ())
#define GDK_QNXSCREEN_DEVICE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GDK_TYPE_QNXSCREEN_DEVICE, GdkQnxScreenDevice))
#define GDK_QNXSCREEN_DEVICE_CLASS(c) (G_TYPE_CHECK_CLASS_CAST ((c), GDK_TYPE_QNXSCREEN_DEVICE, GdkQnxScreenDeviceClass))
#define GDK_IS_QNXSCREEN_DEVICE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GDK_TYPE_QNXSCREEN_DEVICE))
#define GDK_IS_QNXSCREEN_DEVICE_CLASS(c) (G_TYPE_CHECK_CLASS_TYPE ((c), GDK_TYPE_QNXSCREEN_DEVICE))
#define GDK_QNXSCREEN_DEVICE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GDK_TYPE_QNXSCREEN_DEVICE, GdkQnxScreenDeviceClass))

GType gdk_qnxscreen_device_get_type (void);

int gdk_qnxscreen_device_init_seat (GdkDisplay *display);
void gdk_qnxscreen_device_pointer_event (GdkDisplay *display);
void gdk_qnxscreen_device_keyboard_event (GdkDisplay *display);
void gdk_qnxscreen_device_touch_event (GdkDisplay *display, int type);
void gdk_qnxscreen_inputcontrol_event (GdkDisplay *display, int subtype);
void gdk_qnxscreen_device_query_state (GdkDevice *device, GdkSurface *surface, double *win_x, double *win_y, GdkModifierType *mask);
void gdk_qnxscreen_device_seat_surface_destroyed (GdkDisplay *display, GdkSurface *surface);

#endif
