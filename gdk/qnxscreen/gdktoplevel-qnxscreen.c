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
#include "errno.h"
#include "gdk_qnxscreen_common.h"
#include "gdkdevice-qnxscreen-class.h"
#include "gdkdisplay-qnxscreen-data.h"
#include "gdksurface-qnxscreen-data.h"
#include "gdktoplevel-qnxscreen-data.h"
#include <screen/screen.h>


G_DEFINE_TYPE_WITH_CODE (GdkQnxScreenToplevel, gdk_qnxscreen_toplevel, GDK_TYPE_QNXSCREEN_SURFACE, G_IMPLEMENT_INTERFACE (GDK_TYPE_TOPLEVEL, gdk_qnxscreen_toplevel_iface_init))

/*
Attached to parent window "transient_for"
If it doesn't exist, join the default QNX ancestor window group instead
*/
static void
_gdk_qnxscreen_toplevel_surface_attach_to_parent   (GdkQnxScreenToplevel *self)
{
  g_return_if_fail (GDK_IS_QNXSCREEN_TOPLEVEL (self));

  GdkSurface *surface = GDK_SURFACE (self);
  GdkDisplay *display = gdk_surface_get_display (surface);
  GdkQnxScreenSurface *impl = GDK_QNXSCREEN_SURFACE (surface);
  GdkQnxScreenDisplay *display_impl = GDK_QNXSCREEN_DISPLAY (display);

  int ret;

  if (GDK_SURFACE_DESTROYED (surface))
    return;

  screen_window_t parent_window_handle;
  char* parent_group_name;
  if (surface->transient_for != NULL &&
    !GDK_SURFACE_DESTROYED (surface->transient_for))
  {
    gdk_qnxscreen_surface_create_group (surface->transient_for);

    GdkQnxScreenSurface *transient_impl = GDK_QNXSCREEN_SURFACE (surface->transient_for);
    GDK_DEBUG (MISC, "%s toplevel present: create and join group for transient window.", QNX_SCREEN);
    parent_window_handle = transient_impl->window_handle;
    parent_group_name = transient_impl->group_name;
  } else {
    GDK_DEBUG (MISC, "%s toplevel present: create and join group for ancestor window.", QNX_SCREEN);
    parent_window_handle =  display_impl ->qnxscreen_ancestor_window;
    parent_group_name = display_impl -> qnxscreen_group_name;
  }

  if (NULL == parent_window_handle || strlen (parent_group_name) <= 0)
  {
    g_critical (G_STRLOC ": toplevel window has invalid qnxscreen window group");
  }
  else
  {
    ret = screen_join_window_group (impl->window_handle, parent_group_name);
    if (ret)
    {
      g_critical (G_STRLOC ": toplevel window failed join window group: %s", strerror (errno));
    }
    else
    {
      ret = screen_flush_context (impl->context_handle, 0);
      if (ret)
      {
        g_critical (G_STRLOC ": toplevel window failed flush context: %s", strerror (errno));
      }
      else
      {
        GDK_DEBUG (MISC, "%s, toplevel window joined qnxscreen window group: %s", QNX_SCREEN, parent_group_name);
      }
    }
  }

  // TODO: modal?
  // NSWindow *parent = _gdk_macos_surface_get_native (GDK_MACOS_SURFACE (surface->transient_for));
  // NSWindow *window = _gdk_macos_surface_get_native (GDK_MACOS_SURFACE (self));
  int x, y;

  surface->x = 0;
  surface->y = 0;

  // _gdk_macos_display_clear_sorting (GDK_MACOS_DISPLAY (surface->display));
  _gdk_qnxscreen_display_position_surface(display, surface, &x, &y);
  gdk_qnxscreen_surface_move_resize (surface, TRUE, x, y, -1, -1);

}

static void
_gdk_qnxscreen_toplevel_surface_detach_from_parent (GdkQnxScreenToplevel *self)
{

}

static void
_gdk_qnxscreen_toplevel_surface_set_transient_for (GdkQnxScreenToplevel *self,
                                                   GdkQnxScreenSurface  *parent)
{
  g_assert (GDK_IS_QNXSCREEN_TOPLEVEL (self));
  g_assert (!parent || GDK_IS_QNXSCREEN_TOPLEVEL (parent));

  GdkSurface *surface = (GdkSurface *)self;

  if (GDK_SURFACE_DESTROYED (surface))
    return;

  if (surface->transient_for != NULL &&
      !GDK_SURFACE_DESTROYED (surface->transient_for))
    {
      // TODO: remove from the window group
      GdkQnxScreenSurface *surface_impl = GDK_QNXSCREEN_SURFACE(self);
      if (screen_leave_window_group(surface_impl->window_handle)) {
        g_critical (G_STRLOC ": toplevel window failed to leave window group: %s", strerror (errno));
      }
      // _gdk_macos_display_clear_sorting (GDK_MACOS_DISPLAY (surface->display));
    }
  g_clear_object (&GDK_SURFACE (self)->transient_for);

  if (g_set_object (&GDK_SURFACE (self)->transient_for, GDK_SURFACE (parent)))
  {
    _gdk_qnxscreen_toplevel_surface_attach_to_parent(self);
  }
}

enum {
  PROP_0,
  LAST_PROP
};

static void
gdk_qnxscreen_toplevel_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GdkSurface *surface = GDK_SURFACE (object);
  GdkQnxScreenSurface *base = GDK_QNXSCREEN_SURFACE (surface);
  GdkQnxScreenToplevel *toplevel = GDK_QNXSCREEN_TOPLEVEL (base);

  switch (prop_id)
  {
  case LAST_PROP + GDK_TOPLEVEL_PROP_TITLE:
    break;

  case LAST_PROP + GDK_TOPLEVEL_PROP_STARTUP_ID:
    break;

  case LAST_PROP + GDK_TOPLEVEL_PROP_TRANSIENT_FOR:
    _gdk_qnxscreen_toplevel_surface_set_transient_for (toplevel, g_value_get_object (value));
    // g_object_notify_by_pspec (G_OBJECT (surface), pspec);
    //TODO: IMPLEMENT
    break;

  case LAST_PROP + GDK_TOPLEVEL_PROP_MODAL:
    break;

  case LAST_PROP + GDK_TOPLEVEL_PROP_ICON_LIST:
    break;

  case LAST_PROP + GDK_TOPLEVEL_PROP_DECORATED:
    break;

  case LAST_PROP + GDK_TOPLEVEL_PROP_DELETABLE:
    break;

  case LAST_PROP + GDK_TOPLEVEL_PROP_FULLSCREEN_MODE:
    break;

  case LAST_PROP + GDK_TOPLEVEL_PROP_SHORTCUTS_INHIBITED:
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
  GDK_DEBUG (SETTINGS, "%s toplevel set property %d", QNX_SCREEN, prop_id);
}

static void
gdk_qnxscreen_toplevel_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GDK_DEBUG (SETTINGS, "%s toplevel get property %d", QNX_SCREEN, prop_id);
}

static gboolean
gdk_qnxscreen_toplevel_lower (GdkToplevel *toplevel)
{
  gdk_qnxscreen_surface_lower (GDK_SURFACE (toplevel));

  return TRUE;
}

static void
gdk_qnxscreen_toplevel_present (GdkToplevel *toplevel, GdkToplevelLayout *layout)
{
  GdkSurface *surface = GDK_SURFACE (toplevel);
  GdkDisplay *display = gdk_surface_get_display (surface);
  GdkMonitor *monitor;
  GdkToplevelSize size;
  int bounds_width, bounds_height;
  int width, height;
  GdkGeometry geometry;
  GdkSurfaceHints mask;
  gboolean fullscreen;
  gboolean maximize;

  // If has transient_for, use it as the parent
  // Otherwise, use the display window as the parent. 
  int ret;
  screen_window_t parent_window_handle;
  char* parent_group_name;
  GdkQnxScreenSurface *impl = GDK_QNXSCREEN_SURFACE (surface);
  GdkQnxScreenDisplay *display_impl = GDK_QNXSCREEN_DISPLAY (display);
  if (surface->transient_for)
  {
    gdk_qnxscreen_surface_create_group (surface->transient_for);

    GdkQnxScreenSurface *transient_impl = GDK_QNXSCREEN_SURFACE (surface->transient_for);
    GDK_DEBUG (MISC, "%s toplevel present: create and join group for transient window.", QNX_SCREEN);
    parent_window_handle = transient_impl->window_handle;
    parent_group_name = transient_impl->group_name;
  }
  else
  {  
    GDK_DEBUG (MISC, "%s toplevel present: create and join group for ancestor window.", QNX_SCREEN);
    parent_window_handle =  display_impl ->qnxscreen_ancestor_window;
    parent_group_name = display_impl -> qnxscreen_group_name;
  }
  if (NULL == parent_window_handle || strlen (parent_group_name) <= 0)
  {
    g_critical (G_STRLOC ": toplevel window has invalid qnxscreen window group");
  }
  else
  {
    ret = screen_join_window_group (impl->window_handle, parent_group_name);
    if (ret)
    {
      g_critical (G_STRLOC ": toplevel window failed join window group: %s", strerror (errno));
    }
    else
    {
      ret = screen_flush_context (impl->context_handle, 0);
      if (ret)
      {
        g_critical (G_STRLOC ": toplevel window failed flush context: %s", strerror (errno));
      }
      else
      {
        GDK_DEBUG (MISC, "%s, toplevel window joined qnxscreen window group: %s", QNX_SCREEN, parent_group_name);
      }
    }
  }
  
  gdk_qnxscreen_surface_raise (surface);
  monitor = gdk_display_get_monitor_at_surface (display, surface);
  if (monitor)
    {
      GdkRectangle monitor_geometry;
      gdk_monitor_get_geometry (monitor, &monitor_geometry);
      bounds_width = monitor_geometry.width;
      bounds_height = monitor_geometry.height;
      GDK_DEBUG (MISC, "%s monitor bounds - size %dx%d", QNX_SCREEN, bounds_width, bounds_height);

      /* Move the surface to this monitor internally */
      gdk_qnxscreen_surface_move_to_monitor (surface, monitor);
    }
  else
    {
      bounds_width = G_MAXINT;
      bounds_height = G_MAXINT;
    }

  gdk_toplevel_size_init (&size, bounds_width, bounds_height);
  gdk_toplevel_notify_compute_size (GDK_TOPLEVEL (surface), &size);
  g_warn_if_fail (size.width > 0);
  g_warn_if_fail (size.height > 0);
  width = size.width;
  height = size.height;

  if (gdk_toplevel_layout_get_resizable (layout))
    {
      geometry.min_width = size.min_width;
      geometry.min_height = size.min_height;
      mask = GDK_HINT_MIN_SIZE;
    }
  else
    {
      geometry.max_width = geometry.min_width = width;
      geometry.max_height = geometry.min_height = height;
      mask = GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE;
    }

  gdk_surface_constrain_size (&geometry, mask, width, height, &width, &height);
  gdk_qnxscreen_surface_move_resize (surface, FALSE, -1, -1, width, height);

  /* On QNX, just make maximize the same as fullscreen */
  if (gdk_toplevel_layout_get_maximized (layout, &maximize))
    {
      if (maximize)
        gdk_qnxscreen_surface_fullscreen (surface);
      else
        gdk_qnxscreen_surface_unfullscreen (surface);
    }

  if (gdk_toplevel_layout_get_fullscreen (layout, &fullscreen))
    {
      if (fullscreen)
        {
          GdkMonitor *fullscreen_monitor =
              gdk_toplevel_layout_get_fullscreen_monitor (layout);

          if (fullscreen_monitor)
            gdk_qnxscreen_surface_fullscreen_on_monitor (surface, fullscreen_monitor);
          else
            gdk_qnxscreen_surface_fullscreen (surface);
        }
      else
        {
          gdk_qnxscreen_surface_unfullscreen (surface);
        }
    }

  /* TODO: Do something with shadows internally in Screen? */
  if (surface->transient_for) {
    int x = -1, y = -1;
    gboolean position_retrieved = FALSE;
    _gdk_qnxscreen_display_position_surface(display, surface, &x, &y);
    gdk_qnxscreen_surface_move_resize (surface, position_retrieved, x, y, -1, -1);
  }

  int was_mapped = GDK_SURFACE_IS_MAPPED (surface);

  if (!was_mapped)
    gdk_surface_set_is_mapped (surface, TRUE);

  gdk_qnxscreen_surface_show (surface);

  if (!was_mapped)
    gdk_surface_invalidate_rect (surface, NULL);
}

static void
gdk_qnxscreen_toplevel_begin_move (GdkToplevel *toplevel,
                                   GdkDevice *device,
                                   int button,
                                   double x,
                                   double y,
                                   guint32 timestamp)
{
  // TODO: actually implement this
  //       QNX Screen has no native drag implementation -- need to emulate
}

static void
gdk_qnxscreen_toplevel_begin_resize (GdkToplevel *toplevel,
                                     GdkSurfaceEdge edge,
                                     GdkDevice *device,
                                     int button,
                                     double x,
                                     double y,
                                     guint32 timestamp)
{
  // TODO: Actually implement this
}

static void
gdk_qnxscreen_toplevel_init (GdkQnxScreenToplevel *toplevel)
{
  GDK_DEBUG (MISC, "%s initializing GdkQnxScreenToplevel: %p", QNX_SCREEN, toplevel);
}

static void
gdk_qnxscreen_toplevel_constructed (GObject *object)
{
  GdkQnxScreenSurface *qnx_screen_surface = GDK_QNXSCREEN_SURFACE (object);
  GdkSurface *surface = GDK_SURFACE (qnx_screen_surface);
  GDK_DEBUG (MISC, "%s toplevel object constructed: %p", QNX_SCREEN, object);

  /* frame clock: a toplevel surface does not have a parent, so use the parent frame clock */
  gdk_surface_set_frame_clock (surface, _gdk_frame_clock_idle_new ());

  /* create QnxScreen window of type SCREEN_CHILD_WINDOW */
  gdk_qnxscreen_create_window (qnx_screen_surface, SCREEN_CHILD_WINDOW);

  /* call parent */
  G_OBJECT_CLASS (gdk_qnxscreen_toplevel_parent_class)->constructed (object);
}

static void
gdk_qnxscreen_toplevel_class_init (GdkQnxScreenToplevelClass *class)
{
  GDK_DEBUG (MISC, "%s initializing GdkQnxScreenToplevelClass: %p", QNX_SCREEN, class);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->constructed = gdk_qnxscreen_toplevel_constructed;
  object_class->get_property = gdk_qnxscreen_toplevel_get_property;
  object_class->set_property = gdk_qnxscreen_toplevel_set_property;
  gdk_toplevel_install_properties (object_class, 1);
}

void
gdk_qnxscreen_toplevel_iface_init (GdkToplevelInterface *iface)
{
  GDK_DEBUG (MISC, "%s initializing toplevel interface: %p", QNX_SCREEN, iface);

  iface->present = gdk_qnxscreen_toplevel_present;
  iface->lower = gdk_qnxscreen_toplevel_lower;
  iface->begin_move = gdk_qnxscreen_toplevel_begin_move;
  iface->begin_resize = gdk_qnxscreen_toplevel_begin_resize;
}
