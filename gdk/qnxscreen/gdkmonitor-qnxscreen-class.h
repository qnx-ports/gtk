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

#ifndef __GDK_QNXSCREEN_MONITOR_CLASS_H__
#define __GDK_QNXSCREEN_MONITOR_CLASS_H__

#include "gdk/gdk.h"
#include <screen/screen.h>

typedef struct _GdkQnxScreenMonitor GdkQnxScreenMonitor;
typedef struct _GdkQnxScreenMonitorClass GdkQnxScreenMonitorClass;

#define GDK_TYPE_QNXSCREEN_MONITOR              (gdk_qnxscreen_monitor_get_type ())
#define GDK_QNXSCREEN_MONITOR(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), GDK_TYPE_QNXSCREEN_MONITOR, GdkQnxScreenMonitor))
#define GDK_QNXSCREEN_MONITOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GDK_TYPE_QNXSCREEN_MONITOR, GdkQnxScreenMonitorClass))
#define GDK_IS_QNXSCREEN_MONITOR(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), GDK_TYPE_QNXSCREEN_MONITOR))
#define GDK_IS_QNXSCREEN_MONITOR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GDK_TYPE_QNXSCREEN_MONITOR))
#define GDK_QNXSCREEN_MONITOR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GDK_TYPE_QNXSCREEN_MONITOR, GdkQnxScreenMonitorClass))

GType gdk_qnxscreen_monitor_get_type (void);
int gdk_qnxscreen_monitor_init_from_qnxscreen(GdkMonitor* monitor, screen_display_t qnx_screen_display);

#endif