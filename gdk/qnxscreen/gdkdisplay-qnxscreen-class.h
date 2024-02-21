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

#ifndef __GDK_QNXSCREEN_DISPLAY_CLASS_H__
#define __GDK_QNXSCREEN_DISPLAY_CLASS_H__

#include "gdk/gdk.h"

typedef struct _GdkQnxScreenDisplay GdkQnxScreenDisplay;
typedef struct _GdkQnxScreenDisplayClass GdkQnxScreenDisplayClass;

#define GDK_TYPE_QNXSCREEN_DISPLAY (gdk_qnxscreen_display_get_type ())
#define GDK_QNXSCREEN_DISPLAY(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), GDK_TYPE_QNXSCREEN_DISPLAY, GdkQnxScreenDisplay))
#define GDK_QNXSCREEN_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GDK_TYPE_QNXSCREEN_DISPLAY, GdkQnxScreenDisplayClass))
#define GDK_IS_QNXSCREEN_DISPLAY(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), GDK_TYPE_QNXSCREEN_DISPLAY))
#define GDK_IS_QNXSCREEN_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GDK_TYPE_QNXSCREEN_DISPLAY))
#define GDK_QNXSCREEN_DISPLAY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GDK_TYPE_QNXSCREEN_DISPLAY, GdkQnxScreenDisplayClass))

GType gdk_qnxscreen_display_get_type (void);
void gdk_qnxscreen_display_management_event (GdkDisplay *display);
void gdk_qnxscreen_display_surface_destroyed (GdkDisplay *display, GdkSurface *surface);

#endif
