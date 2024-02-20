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

#ifndef __GDK_QNXSCREEN_EVENTLOOP_H__
#define __GDK_QNXSCREEN_EVENTLOOP_H__

#include "gdk/gdk.h"
#include "gdk/gdkdisplayprivate.h"
#include "gdk/gdkeventsprivate.h"
#include <screen/screen.h>

typedef struct _GdkQnxScreenEventSource
{
  GSource source;
  GdkDisplay *display;
} GdkQnxScreenEventSource;

int gdk_qnxscreen_event_loop_init (GdkDisplay *display, screen_context_t context);
void gdk_qnxscreen_event_deliver_event (GdkDisplay *display, GdkEvent *event);

#endif
