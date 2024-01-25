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

#ifndef __GDK_SURFACE_QNXSCREEN_CLASS_H__
#define __GDK_SURFACE_QNXSCREEN_CLASS_H__

#include "gdk/gdk.h"
#include "gdk/gdksurfaceprivate.h"

typedef struct _GdkQnxScreenSurface GdkQnxScreenSurface;
typedef struct _GdkQnxScreenSurfaceClass GdkQnxScreenSurfaceClass;

#define GDK_TYPE_QNXSCREEN_SURFACE              (gdk_qnxscreen_surface_get_type ())
#define GDK_QNXSCREEN_SURFACE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), GDK_TYPE_QNXSCREEN_SURFACE, GdkQnxScreenSurface))
#define GDK_QNXSCREEN_SURFACE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GDK_TYPE_QNXSCREEN_SURFACE, GdkQnxScreenSurfaceClass))
#define GDK_IS_QNXSCREEN_SURFACE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), GDK_TYPE_QNXSCREEN_SURFACE))
#define GDK_IS_QNXSCREEN_SURFACE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GDK_TYPE_QNXSCREEN_SURFACE))
#define GDK_QNXSCREEN_SURFACE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GDK_TYPE_QNXSCREEN_SURFACE, GdkQnxScreenSurfaceClass))

GType gdk_qnxscreen_surface_get_type (void);
void gdk_qnxscreen_surface_post_screen(GdkSurface* surface, int num_rects, cairo_rectangle_int_t* damaged_rects);

typedef struct _GdkQnxScreenToplevel GdkQnxScreenToplevel;
typedef struct _GdkQnxScreenToplevelClass GdkQnxScreenToplevelClass;

#define GDK_TYPE_QNXSCREEN_TOPLEVEL (gdk_qnxscreen_toplevel_get_type ())

GType gdk_qnxscreen_toplevel_get_type (void);
void gdk_qnxscreen_toplevel_iface_init(GdkToplevelInterface* iface);

typedef struct _GdkQnxScreenPopup GdkQnxScreenPopup;
typedef struct _GdkQnxScreenPopupClass GdkQnxScreenPopupClass;

#define GDK_TYPE_QNXSCREEN_POPUP (gdk_qnxscreen_popup_get_type ())

GType gdk_qnxscreen_popup_get_type (void);
void gdk_qnxscreen_popup_iface_init (GdkPopupInterface* iface);

#endif
