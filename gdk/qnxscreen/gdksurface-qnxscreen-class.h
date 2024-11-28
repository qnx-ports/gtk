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

#ifndef __GDK_SURFACE_QNXSCREEN_CLASS_H__
#define __GDK_SURFACE_QNXSCREEN_CLASS_H__

#include "gdk/gdk.h"
#include "gdk/gdksurfaceprivate.h"

G_BEGIN_DECLS

typedef struct _GdkQnxScreenSurface GdkQnxScreenSurface;
typedef struct _GdkQnxScreenSurfaceClass GdkQnxScreenSurfaceClass;

#define GDK_TYPE_QNXSCREEN_SURFACE (gdk_qnxscreen_surface_get_type ())
#define GDK_QNXSCREEN_SURFACE(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), GDK_TYPE_QNXSCREEN_SURFACE, GdkQnxScreenSurface))
#define GDK_QNXSCREEN_SURFACE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GDK_TYPE_QNXSCREEN_SURFACE, GdkQnxScreenSurfaceClass))
#define GDK_IS_QNXSCREEN_SURFACE(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), GDK_TYPE_QNXSCREEN_SURFACE))
#define GDK_IS_QNXSCREEN_SURFACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GDK_TYPE_QNXSCREEN_SURFACE))
#define GDK_QNXSCREEN_SURFACE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GDK_TYPE_QNXSCREEN_SURFACE, GdkQnxScreenSurfaceClass))

GType gdk_qnxscreen_surface_get_type (void);

void gdk_qnxscreen_surface_post_screen (GdkSurface *surface, int buffer_idx, int num_rects, cairo_rectangle_int_t *damaged_rects);
gboolean gdk_qnxscreen_surface_get_device_state (GdkSurface *surface, GdkDevice *device, double *x, double *y, GdkModifierType *mask);
void gdk_qnxscreen_surface_get_root_coords (GdkSurface *surface, gint x, gint y, gint *root_x, gint *root_y);
int gdk_qnxscreen_surface_create_buffers (GdkQnxScreenSurface *qnx_screen_surface, int *buf_size);
void gdk_qnxscreen_surface_destroy_buffers (GdkQnxScreenSurface *impl);
int gdk_qnxscreen_surface_recreate_buffers (GdkQnxScreenSurface *impl, int *buf_size);
void gdk_qnxscreen_surface_set_input_region (GdkSurface *surface, cairo_region_t *shape_region);
void gdk_qnxscreen_surface_show (GdkSurface *surface);
void gdk_qnxscreen_surface_hide (GdkSurface *surface);
void gdk_qnxscreen_surface_move_to_monitor (GdkSurface *surface, GdkMonitor *monitor);
void gdk_qnxscreen_surface_move_resize (GdkSurface *surface, gboolean with_move, int x, int y, int width, int height);
void gdk_qnxscreen_surface_fullscreen (GdkSurface *surface);
void gdk_qnxscreen_surface_fullscreen_on_monitor (GdkSurface *surface, GdkMonitor *monitor);
void gdk_qnxscreen_surface_unfullscreen (GdkSurface *surface);
void gdk_qnxscreen_surface_destroy (GdkSurface *surface, gboolean foreign_destroy);
GdkDrag * gdk_qnxscreen_surface_drag_begin (GdkSurface *surface, GdkDevice *device, GdkContentProvider *content, GdkDragAction actions, double dx, double dy);
/*
Create a QNX window group for the surface if it doesn't have group handle already.
*/
void gdk_qnxscreen_surface_create_group (GdkSurface *surface);
void gdk_qnxscreen_create_window (GdkQnxScreenSurface *qnx_screen_surface, int qnxscreen_window_type);
void gdk_qnxscreen_surface_raise (GdkSurface *surface);
void gdk_qnxscreen_surface_lower (GdkSurface *surface);

typedef struct _GdkQnxScreenPopup GdkQnxScreenPopup;
typedef struct _GdkQnxScreenPopupClass GdkQnxScreenPopupClass;

#define GDK_TYPE_QNXSCREEN_POPUP (gdk_qnxscreen_popup_get_type ())

GType gdk_qnxscreen_popup_get_type (void);
void gdk_qnxscreen_popup_iface_init (GdkPopupInterface *iface);

G_END_DECLS

#endif
