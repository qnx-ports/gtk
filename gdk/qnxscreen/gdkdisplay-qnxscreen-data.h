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

#ifndef __GDK_QNXSCREEN_DISPLAY_DATA_H__
#define __GDK_QNXSCREEN_DISPLAY_DATA_H__

#include "gdk/gdkdebugprivate.h"
#include "gdk/gdkdisplayprivate.h"
#include "gdk/gdkkeysprivate.h"
#include "gdk/gdkseatdefaultprivate.h"
#include "gdk/gdksurfaceprivate.h"
#include "gdkdisplay-qnxscreen-class.h"
#include "gio/gio.h"
#include "glib.h"
#include <screen/screen.h>

struct _GdkQnxScreenDisplay
{
  GdkDisplay parent_instance;
  GListStore *monitors;
  /* Logical devices */
  GdkDevice *core_pointer;
  GdkDevice *core_keyboard;
  /* Physical devices */
  GdkDevice *phys_pointer;
  GdkDevice *phys_keyboard;
  GdkDevice *phys_touchscreen;
  GdkSeat *seat;
  GSource *event_source;
  GdkKeymap *keymap;
  GdkSurface *focused_surface;
  screen_context_t qnxscreen_context;
  screen_event_t event;
  GHashTable *surface_window_table;

  /* A queue of signed integers. Used to track zorders
   * of all top-level windows in this GdkDisplay */
  GQueue *zorders;

  /* Screen does not provide a unique ID for individual touch sequences
   * (i.e. BEGIN/UPDATE/END sequences). It provides TOUCH_ID, the ID of the
   * contact point in multi-touch situations, and SEQUENCE_ID, an always
   * incrementing number for a given TOUCH_ID. Generate our own touch
   * sequence numbers unique to gestures on a given surface.
   */
  int next_touch_sequence;
};

struct _GdkQnxScreenDisplayClass
{
  GdkDisplayClass parent_class;
};

#endif