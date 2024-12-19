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

#ifndef __GDK_TOPLEVEL_QNXSCREEN_CLASS_H__
#define __GDK_TOPLEVEL_QNXSCREEN_CLASS_H__

#include "gdk/gdk.h"
#include "gdk/gdksurfaceprivate.h"

typedef struct _GdkQnxScreenToplevel GdkQnxScreenToplevel;
typedef struct _GdkQnxScreenToplevelClass GdkQnxScreenToplevelClass;

#define GDK_TYPE_QNXSCREEN_TOPLEVEL        (gdk_qnxscreen_toplevel_get_type ())
#define GDK_QNXSCREEN_TOPLEVEL(object)     (G_TYPE_CHECK_INSTANCE_CAST ((object), GDK_TYPE_QNXSCREEN_TOPLEVEL, GdkQnxScreenToplevel))
#define GDK_IS_QNXSCREEN_TOPLEVEL(object)  (G_TYPE_CHECK_INSTANCE_TYPE ((object), GDK_TYPE_QNXSCREEN_TOPLEVEL))

GType gdk_qnxscreen_toplevel_get_type (void);
void gdk_qnxscreen_toplevel_iface_init (GdkToplevelInterface *iface);

#endif
