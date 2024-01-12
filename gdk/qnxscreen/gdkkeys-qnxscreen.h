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

#ifndef __GDK_QNXSCREEN_KEYS_H__
#define __GDK_QNXSCREEN_KEYS_H__

#include "gdk/gdk.h"
#include "gdk/gdkdisplayprivate.h"
#include "gdk/gdkkeysprivate.h"
#include "gdk/gdkkeysyms.h"

typedef struct 
{
    GdkKeymap parent_instance;
} GdkQnxScreenKeymap;

typedef struct
{
    GdkKeymapClass keymap_class;
} GdkQnxScreenKeymapClass;

#define GDK_TYPE_QNXSCREEN_KEYMAP          (gdk_qnxscreen_keymap_get_type ())
#define GDK_QNXSCREEN_KEYMAP(object)       (G_TYPE_CHECK_INSTANCE_CAST ((object), GDK_TYPE_QNXSCREEN_KEYMAP, GdkQnxScreenKeymap))
#define GDK_IS_QNXSCREEN_KEYMAP(object)    (G_TYPE_CHECK_INSTANCE_TYPE ((object), GDK_TYPE_QNXSCREEN_KEYMAP))

GType gdk_qnxscreen_keymap_get_type (void);
GdkKeymap* gdk_qnxscreen_keymap_get_keymap(GdkDisplay* display);

#endif
