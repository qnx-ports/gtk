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

#include "config.h"
#include "gdk_qnxscreen_common.h"
#include "gdkcairocontext-qnxscreen-data.h"
#include "gdksurface-qnxscreen-data.h"

G_DEFINE_TYPE (GdkQnxScreenCairoContext, gdk_qnxscreen_cairo_context, GDK_TYPE_CAIRO_CONTEXT)

static void
gdk_qnxscreen_cairo_context_dispose (GObject *object)
{
  GDK_DEBUG (MISC, "%s dispose GdkQnxScreenCairoContext", QNX_SCREEN);
  G_OBJECT_CLASS (gdk_qnxscreen_cairo_context_parent_class)->dispose (object);
}

static void
gdk_qnxscreen_cairo_context_begin_frame (GdkDrawContext *draw_context, GdkMemoryDepth depth, cairo_region_t *region)
{
  GDK_DEBUG (MISC, "%s begin_frame GdkQnxScreenCairoContext", QNX_SCREEN);
}

static void
gdk_qnxscreen_cairo_context_end_frame (GdkDrawContext *draw_context, cairo_region_t *painted)
{
  GDK_DEBUG (MISC, "%s end_frame GdkQnxScreenCairoContext", QNX_SCREEN);
  int num_rects = 0;
  cairo_rectangle_int_t *damage_rects = NULL;
  GdkQnxScreenCairoContext *qnx_screen_cairo_context = GDK_QNXSCREEN_CAIRO_CONTEXT (draw_context);
  GdkSurface *surface = gdk_draw_context_get_surface (draw_context);
  GdkQnxScreenSurface *qnx_screen_surface = GDK_QNXSCREEN_SURFACE (surface);

  /* determine number of damaged areas that require repaint */
  num_rects = cairo_region_num_rectangles (painted);
  damage_rects = alloca (sizeof (cairo_rectangle_int_t) * num_rects);

  if (damage_rects != NULL)
    {
      /* copy all the damaged rects out of the region into an array */
      for (int i = 0; i < num_rects; i++)
        {
          cairo_region_get_rectangle (painted, i, &damage_rects[i]);
          GDK_DEBUG (FRAMES, "%s damaged rectangle %d,%d,%d,%d",
                     QNX_SCREEN,
                     damage_rects[i].x,
                     damage_rects[i].y,
                     damage_rects[i].width,
                     damage_rects[i].height);
        }

      /* draw the cairo buffer to the screen */
      gdk_qnxscreen_surface_post_screen (surface, qnx_screen_cairo_context->active_buffer_idx, num_rects, damage_rects);

      /* copy all drawn regions to the next rendering buffer
       * this is required as per QNX screen handbook, since it does not guarantee
       * only the dirty regions are flushed to screen
       */
      int new_buffer_idx = (qnx_screen_cairo_context->active_buffer_idx + 1) % QNXSCREEN_NUM_BUFFERS;
      cairo_t *cr = cairo_create (qnx_screen_surface->cairo_surfaces[new_buffer_idx]);
      cairo_set_source_surface (cr, qnx_screen_surface->cairo_surfaces[qnx_screen_cairo_context->active_buffer_idx], 0, 0);
      /* only copy dirty regions for performance reasons */
      gdk_cairo_region (cr, painted);
      cairo_clip (cr);
      cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
      cairo_paint (cr);
      cairo_destroy (cr);
      qnx_screen_cairo_context->active_buffer_idx = new_buffer_idx;
    }
  else
    {
      g_critical (G_STRLOC ": failed to alloca memory: %s", strerror (errno));
    }
}

static cairo_t *
gdk_qnxscreen_cairo_context_cairo_create (GdkCairoContext *context)
{
  GDK_DEBUG (MISC, "%s creating cairo context", QNX_SCREEN);

  GdkQnxScreenCairoContext *qnx_screen_cairo_context = GDK_QNXSCREEN_CAIRO_CONTEXT (context);
  GdkSurface *surface = gdk_draw_context_get_surface (GDK_DRAW_CONTEXT (qnx_screen_cairo_context));
  GdkQnxScreenSurface *qnx_screen_surface = GDK_QNXSCREEN_SURFACE (surface);
  return cairo_create (qnx_screen_surface->cairo_surfaces[qnx_screen_cairo_context->active_buffer_idx]);
}

static void
gdk_qnxscreen_cairo_context_class_init (GdkQnxScreenCairoContextClass *class)
{
  GDK_DEBUG (MISC, "%s initializing GdkQnxScreenCairoContextClass", QNX_SCREEN);

  GObjectClass *gobject_class = G_OBJECT_CLASS (class);
  GdkDrawContextClass *draw_context_class = GDK_DRAW_CONTEXT_CLASS (class);
  GdkCairoContextClass *cairo_context_class = GDK_CAIRO_CONTEXT_CLASS (class);

  gobject_class->dispose = gdk_qnxscreen_cairo_context_dispose;
  draw_context_class->begin_frame = gdk_qnxscreen_cairo_context_begin_frame;
  draw_context_class->end_frame = gdk_qnxscreen_cairo_context_end_frame;
  cairo_context_class->cairo_create = gdk_qnxscreen_cairo_context_cairo_create;
}

static void
gdk_qnxscreen_cairo_context_init (GdkQnxScreenCairoContext *self)
{
  GDK_DEBUG (MISC, "%s initializing GdkQnxScreenCairoContext", QNX_SCREEN);
  self->active_buffer_idx = 0;
}
