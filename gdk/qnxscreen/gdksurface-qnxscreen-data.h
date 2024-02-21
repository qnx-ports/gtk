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

#ifndef __GDK_SURFACE_QNXSCREEN_DATA_H__
#define __GDK_SURFACE_QNXSCREEN_DATA_H__

#include "gdk/gdk.h"
#include "gdk/gdkdebugprivate.h"
#include "gdk/gdkframeclockidleprivate.h"
#include "gdk/gdkpopupprivate.h"
#include "gdk/gdksurfaceprivate.h"
#include "gdk/gdktoplevelprivate.h"
#include "gdksurface-qnxscreen-class.h"
#include <screen/screen.h>

#define MAX_QNXSCREEN_NAME_LEN (128)
#define QNXSCREEN_NUM_BUFFERS 2

struct _GdkQnxScreenSurface
{
  GdkSurface parent_instance;

  screen_context_t context_handle;
  screen_window_t window_handle;
  screen_buffer_t buffer_handles[QNXSCREEN_NUM_BUFFERS];
  screen_session_t session_handle;
  cairo_surface_t *cairo_surfaces[QNXSCREEN_NUM_BUFFERS];
  gboolean visible;
  /* Window */
  int win_width;
  int win_height;
  int win_x;
  int win_y;
  /* Buffer */
  int buf_width;
  int buf_height;
  int buf_stride;
  void *buf_ptrs[QNXSCREEN_NUM_BUFFERS];

  /* Current zorder of this window */
  int zorder;

  /* Group owned by this window that child windows can join */
  screen_group_t group_handle;
  char group_name[MAX_QNXSCREEN_NAME_LEN];

  /* A queue of signed integers. Used to track zorders
   * of all child windows */
  GQueue *zorders;

  /* Monitor (Screen Display) that this surface is currently mapped to */
  GdkMonitor *mapped_monitor;

  /* Information for handling fullscreen operations */
  gboolean fullscreened;
  int pre_fullscreen_x;
  int pre_fullscreen_y;
  int pre_fullscreen_width;
  int pre_fullscreen_height;

  /* Active touch sequences on this surface
   * (key,value) = (touch_id, sequence)
   * NULL sequence indicates that touch_id is not active.
   */
  GHashTable *active_touch_table;

  /* Refocussed touch seqeunces.
   * (key,value) = (touch_id, [0 or 1 or 2])
   * 1 indicates that an mtouch may have been started without a begin.
   */
  GHashTable *refocussed_touch_table;
};

struct _GdkQnxScreenSurfaceClass
{
  GdkSurfaceClass parent_class;
};

struct _GdkQnxScreenToplevel
{
  struct _GdkQnxScreenSurface parent_instance;
};

struct _GdkQnxScreenToplevelClass
{
  struct _GdkQnxScreenSurfaceClass parent_class;
};

struct _GdkQnxScreenPopup
{
  struct _GdkQnxScreenSurface parent_instance;
};

struct _GdkQnxScreenPopupClass
{
  struct _GdkQnxScreenSurfaceClass parent_class;
};

#endif
