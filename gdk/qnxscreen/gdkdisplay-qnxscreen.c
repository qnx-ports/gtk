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
#include "gdkcairocontext-qnxscreen-class.h"
#include "gdkdevice-qnxscreen-class.h"
#include "gdkdisplay-qnxscreen-data.h"
#include "gdkeventloop-qnxscreen.h"
#include "gdkkeys-qnxscreen.h"
#include "gdkmonitor-qnxscreen-class.h"
#include "gdkprivate-qnxscreen.h"
#include "gdksurface-qnxscreen-class.h"

G_DEFINE_TYPE (GdkQnxScreenDisplay, gdk_qnxscreen_display, GDK_TYPE_DISPLAY)

static const char *
gdk_qnxscreen_display_get_name (GdkDisplay *display)
{
  g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);
  return (char *) "QNX Screen";
}

static gboolean
gdk_qnxscreen_display_get_setting (GdkDisplay *display, const char *name, GValue *value)
{
  GDK_DEBUG (SETTINGS, "%s get setting - %s", QNX_SCREEN, name);

  return FALSE;
}

static GListModel *
gdk_qnxscreen_display_get_monitors (GdkDisplay *display)
{

  GdkQnxScreenDisplay *qnx_screen_display = GDK_QNXSCREEN_DISPLAY (display);
  return G_LIST_MODEL (qnx_screen_display->monitors);
}

static GdkMonitor *
gdk_qnxscreen_display_get_monitor_at_surface (GdkDisplay *display, GdkSurface *surface)
{
  GDK_DEBUG (MISC, "%s getting monitor for surface", QNX_SCREEN);

  // TODO - IMPLEMENT!
  GdkQnxScreenDisplay *qnx_screen_display = GDK_QNXSCREEN_DISPLAY (display);
  GdkMonitor *monitor = g_list_model_get_item (G_LIST_MODEL (qnx_screen_display->monitors), 0);
  g_object_unref (monitor);
  return monitor;
}

static gboolean
gdk_qnxscreen_display_has_pending (GdkDisplay *display)
{
  GDK_DEBUG (MISC, "%s has pending", QNX_SCREEN);
  return FALSE;
}

static gulong
gdk_qnxscreen_display_get_next_serial (GdkDisplay *display)
{
  static gulong serial = 0;
  return ++serial;
}

static void
gdk_qnxscreen_display_notify_startup_complete (GdkDisplay *display, const char *startup_id)
{
  GDK_DEBUG (MISC, "%s startup complete", QNX_SCREEN);
}

static void
gdk_qnxscreen_display_beep (GdkDisplay *display)
{
  GDK_DEBUG (MISC, "%s BEEP!", QNX_SCREEN);
}

static void
gdk_qnxscreen_display_dispose (GObject *object)
{
  GDK_DEBUG (MISC, "%s disposing GdkQnxScreenDisplay", QNX_SCREEN);

  GdkQnxScreenDisplay *qnx_screen_display = GDK_QNXSCREEN_DISPLAY (object);

  if (qnx_screen_display->monitors != NULL)
    {
      g_list_store_remove_all (qnx_screen_display->monitors);
      g_clear_object (&qnx_screen_display->monitors);
    }

  if (qnx_screen_display->qnxscreen_context != NULL)
    {
      screen_destroy_context (qnx_screen_display->qnxscreen_context);
      qnx_screen_display->qnxscreen_context = NULL;
    }

  if (qnx_screen_display->zorders)
    {
      g_queue_free (qnx_screen_display->zorders);
      qnx_screen_display->zorders = NULL;
    }

  // TODO DISPOSE SEAT & DEVICES!
  // TODO cleanup lookup table

  G_OBJECT_CLASS (gdk_qnxscreen_display_parent_class)->dispose (object);
}

static void
gdk_qnxscreen_display_init (GdkQnxScreenDisplay *display)
{
  GDK_DEBUG (MISC, "%s initializing GdkQnxScreenDisplay", QNX_SCREEN);

  display->qnxscreen_context = NULL;
  display->event = NULL;
  display->monitors = NULL;
  display->core_pointer = NULL;
  display->core_keyboard = NULL;
  display->phys_touchscreen = NULL;
  display->phys_pointer = NULL;
  display->phys_keyboard = NULL;
  display->seat = NULL;
  display->event_source = NULL;
  display->keymap = NULL;
  display->surface_window_table = NULL;

  display->zorders = g_queue_new ();
  /* Initialize to 1. Treat 0 as invalid in surface::active_touch_sequence map */
  display->next_touch_sequence = 1;
}

static void
gdk_qnxscreen_display_class_init (GdkQnxScreenDisplayClass *class)
{
  GDK_DEBUG (MISC, "%s initializing GdkQnxScreenDisplayClass", QNX_SCREEN);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GdkDisplayClass *display_class = GDK_DISPLAY_CLASS (class);

  object_class->dispose = gdk_qnxscreen_display_dispose;
  display_class->get_name = gdk_qnxscreen_display_get_name;
  display_class->get_setting = gdk_qnxscreen_display_get_setting;
  display_class->get_monitors = gdk_qnxscreen_display_get_monitors;
  display_class->has_pending = gdk_qnxscreen_display_has_pending;
  display_class->get_next_serial = gdk_qnxscreen_display_get_next_serial;
  display_class->notify_startup_complete = gdk_qnxscreen_display_notify_startup_complete;
  display_class->get_monitor_at_surface = gdk_qnxscreen_display_get_monitor_at_surface;
  display_class->get_keymap = gdk_qnxscreen_keymap_get_keymap;
  display_class->beep = gdk_qnxscreen_display_beep;
  display_class->cairo_context_type = GDK_TYPE_QNXSCREEN_CAIRO_CONTEXT;
  display_class->toplevel_type = GDK_TYPE_QNXSCREEN_TOPLEVEL;
  display_class->popup_type = GDK_TYPE_QNXSCREEN_POPUP;
}

GdkDisplay *
_gdk_qnxscreen_display_open (const gchar *display_name)
{
  GDK_DEBUG (MISC, "%s initializing GDK Display", QNX_SCREEN);

  int ret = 0;
  int num_screens = 0;
  GdkDisplay *display = NULL;
  GdkQnxScreenDisplay *qnx_screen_display = NULL;
  GdkMonitor *monitor = NULL;
  screen_display_t *qnx_screen_display_handles = NULL;

  /* create our QnxScreenDisplay class */
  display = g_object_new (GDK_TYPE_QNXSCREEN_DISPLAY, NULL);
  qnx_screen_display = GDK_QNXSCREEN_DISPLAY (display);
  if (display == NULL)
    {
      g_critical (G_STRLOC ": failed to create GDK_TYPE_QNXSCREEN_DISPLAY: %s", strerror (errno));
      ret = -1;
    }
  else
    {
      GDK_DEBUG (MISC, "%s created GdkQnxScreenDisplay context", QNX_SCREEN);
    }

  /* create QNX Screen Window to GDKSurface lookup table */
  if (ret == 0)
    {
      qnx_screen_display->surface_window_table = g_hash_table_new (g_direct_hash, g_direct_equal);
      if (qnx_screen_display->surface_window_table == NULL)
        {
          g_critical (G_STRLOC ": failed to create surface_window_table: %s", strerror (errno));
          ret = -1;
        }
      else
        {
          GDK_DEBUG (MISC, "%s created surface_window_table", QNX_SCREEN);
        }
    }

  /* initialize seat with all the input devices */
  if (ret == 0)
    {
      ret = gdk_qnxscreen_device_init_seat (display);
      if (ret == -1)
        {
          g_critical (G_STRLOC ": failed to initialize GDKSeat: %s", strerror (errno));
        }
      else
        {
          GDK_DEBUG (MISC, "%s initialized GDKSeat", QNX_SCREEN);
        }
    }

  /* create the qnx screen context for this application */
  if (ret == 0)
    {
      ret = screen_create_context (&qnx_screen_display->qnxscreen_context, SCREEN_APPLICATION_CONTEXT);
      if (ret == -1)
        {
          g_critical (G_STRLOC ": Please ensure that the QNX Screen server (screen) is running. screen_create_context(): %s", strerror (errno));
        }
      else
        {
          if (display_name)
            {
              /* Set the name for this context */
              printf ("Setting display name! %s\n", display_name);
              ret = screen_set_context_property_cv (qnx_screen_display->qnxscreen_context, SCREEN_PROPERTY_ID_STRING, strlen (display_name), display_name);
              if (ret)
                {
                  g_critical (G_STRLOC ": screen_set_context_property_cv(SCREEN_PROPERTY_ID_STRING): %s", strerror (errno));
                }
            }

          GDK_DEBUG (MISC, "%s created screen context", QNX_SCREEN);
        }
    }

  /* initialize the event loop */
  if (ret == 0)
    {
      ret = gdk_qnxscreen_event_loop_init (display, qnx_screen_display->qnxscreen_context);
      if (ret == -1)
        {
          g_critical (G_STRLOC ": failed to create event loop: %s", strerror (errno));
        }
      else
        {
          GDK_DEBUG (MISC, "%s initialized event loop", QNX_SCREEN);
        }
    }

  /* query qnx screen to determine how many monitors (screen displays) are available */
  if (ret == 0)
    {
      ret = screen_get_context_property_iv (qnx_screen_display->qnxscreen_context, SCREEN_PROPERTY_DISPLAY_COUNT, &num_screens);
      if (ret == -1)
        {
          g_critical (G_STRLOC ": failed to determine display count: %s", strerror (errno));
        }
      else
        {
          GDK_DEBUG (MISC, "%s successfully retrieved monitor count: %d", QNX_SCREEN, num_screens);
        }
    }

  /* get the qnx screen display handles */
  if (ret == 0)
    {
      qnx_screen_display_handles = (screen_display_t *) calloc (num_screens, sizeof (screen_display_t));
      ret = screen_get_context_property_pv (qnx_screen_display->qnxscreen_context, SCREEN_PROPERTY_DISPLAYS, (void **) qnx_screen_display_handles);
      if (ret == -1)
        {
          g_critical (G_STRLOC ": failed to get display array: %s", strerror (errno));
        }
      else
        {
          GDK_DEBUG (MISC, "%s successfully retrieved screen display handles", QNX_SCREEN);
        }
    }

  /* initialize GdkMonitors collection */
  if (ret == 0)
    {
      qnx_screen_display->monitors = g_list_store_new (GDK_TYPE_MONITOR);
      for (int i = 0; i < num_screens && ret == 0; i++)
        {
          GDK_DEBUG (MISC, "%s initializing monitor %d", QNX_SCREEN, i);
          monitor = g_object_new (GDK_TYPE_QNXSCREEN_MONITOR, "display", display, NULL);
          g_list_store_append (qnx_screen_display->monitors, monitor);
          ret = gdk_qnxscreen_monitor_init_from_qnxscreen (monitor, qnx_screen_display_handles[i]);
        }
    }

  /* release temp memory for screen displays */
  if (qnx_screen_display_handles != NULL)
    {
      free (qnx_screen_display_handles);
    }

  /* pulse display opened msg */
  if (ret == 0)
    {
      g_signal_emit_by_name (display, "opened");
    }

  /* check for failures and release display memory if necessary */
  if (ret != 0 && display != NULL)
    {
      g_clear_object (&display);
      display = NULL;
    }

  return display;
}

void
gdk_qnxscreen_display_management_event (GdkDisplay *display)
{
  GDK_DEBUG (EVENTS, "%s manager event", QNX_SCREEN);

  int ret = 0;
  int subtype = 0;
  GdkQnxScreenDisplay *qnx_screen_display = GDK_QNXSCREEN_DISPLAY (display);
  GdkSurface *surface = NULL;
  GdkEvent *event = NULL;
  screen_window_t window_handle = NULL;

  /* find the window over which the event occurred */
  if (ret == 0)
    {
      ret = screen_get_event_property_pv (qnx_screen_display->event, SCREEN_PROPERTY_WINDOW, (void **) &window_handle);
      if (ret == -1)
        {
          g_critical (G_STRLOC ": failed to get manager event window: %s", strerror (errno));
        }
    }

  /* translate the native window to a GDKSurface */
  if (ret == 0)
    {
      surface = g_hash_table_lookup (qnx_screen_display->surface_window_table, window_handle);
    }

  /* get the management event type */
  if (ret == 0)
    {
      ret = screen_get_event_property_iv (qnx_screen_display->event, SCREEN_PROPERTY_SUBTYPE, &subtype);
      if (ret == -1)
        {
          g_critical (G_STRLOC ": failed to get manager event subtype: %s", strerror (errno));
        }
    }

  /* create event and deliver */
  if (ret == 0)
    {
      switch (subtype)
        {
        case SCREEN_EVENT_CLOSE:
          event = gdk_delete_event_new (surface);
          GDK_DEBUG (EVENTS, "%s delete event on surface %p", QNX_SCREEN, surface);
          gdk_qnxscreen_event_deliver_event (display, event);
          break;

        default:
          GDK_DEBUG (EVENTS, "%s unknown management event type %d", QNX_SCREEN, subtype);
          break;
        }
    }
}

void
gdk_qnxscreen_display_surface_destroyed (GdkDisplay *display, GdkSurface *surface)
{
  GDK_DEBUG (MISC, "%s surface destroyed event: %p", QNX_SCREEN, surface);
  gdk_qnxscreen_device_seat_surface_destroyed (display, surface);
}
