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

#include "config.h"
#include "gdk_qnxscreen_common.h"
#include "gdkdisplay-qnxscreen-class.h"
#include "gdkglcontext-qnxscreen-data.h"

G_DEFINE_TYPE (GdkQnxScreenGLContext, gdk_qnxscreen_gl_context, GDK_TYPE_GL_CONTEXT)

static void
gdk_qnxscreen_gl_context_begin_frame (GdkDrawContext *draw_context, gboolean prefers_high_depth, cairo_region_t *region)
{
  //   gdk_wayland_surface_ensure_wl_egl_window (gdk_draw_context_get_surface (draw_context));

  //   GDK_DRAW_CONTEXT_CLASS (gdk_wayland_gl_context_parent_class)->begin_frame (draw_context, prefers_high_depth, region);
}

static void
gdk_qnxscreen_gl_context_end_frame (GdkDrawContext *draw_context, cairo_region_t *painted)
{
  //   GdkSurface *surface = gdk_draw_context_get_surface (draw_context);

  //   gdk_wayland_surface_sync (surface);
  //   gdk_wayland_surface_request_frame (surface);

  //   GDK_DRAW_CONTEXT_CLASS (gdk_wayland_gl_context_parent_class)->end_frame (draw_context, painted);

  //   gdk_wayland_surface_notify_committed (surface);
}

static void
gdk_qnxscreen_gl_context_class_init (GdkQnxScreenGLContextClass *class)
{
  GdkDrawContextClass *draw_context_class = GDK_DRAW_CONTEXT_CLASS (class);
  GdkGLContextClass *context_class = GDK_GL_CONTEXT_CLASS (class);

  draw_context_class->begin_frame = gdk_qnxscreen_gl_context_begin_frame;
  draw_context_class->end_frame = gdk_qnxscreen_gl_context_end_frame;

  context_class->backend_type = GDK_GL_EGL;
}

static void
gdk_qnxscreen_gl_context_init (GdkQnxScreenGLContext *self)
{
}

gpointer
gdk_qnxscreen_display_get_egl_display (GdkDisplay *display)
{
  g_return_val_if_fail (GDK_IS_QNXSCREEN_DISPLAY (display), NULL);
  return gdk_display_get_egl_display (display);
}

GdkGLContext *
gdk_qnxscreen_display_init_gl (GdkDisplay *display, GError **error)
{
  GdkQnxScreenDisplay *self = GDK_QNXSCREEN_DISPLAY (display);

  // if (!gdk_display_init_egl (display,
  //                             EGL_PLATFORM_WAYLAND_EXT,
  //                             self->wl_display,
  //                             TRUE,
  //                             error)) {
  //     return NULL;
  // }

  return g_object_new (GDK_TYPE_QNXSCREEN_GL_CONTEXT,
                       "display", display,
                       NULL);
}
