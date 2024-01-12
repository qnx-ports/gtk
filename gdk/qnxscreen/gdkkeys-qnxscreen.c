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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <limits.h>
#include <errno.h>
#include <sys/keycodes.h>
#include "gdkkeys-qnxscreen.h"
#include "gdkdisplay-qnxscreen-data.h"

G_DEFINE_TYPE (GdkQnxScreenKeymap, gdk_qnxscreen_keymap, GDK_TYPE_KEYMAP)

static void gdk_qnxscreen_keymap_init (GdkQnxScreenKeymap* keymap)
{
}

static void gdk_qnxscreen_keymap_finalize (GObject* object)
{
    G_OBJECT_CLASS (gdk_qnxscreen_keymap_parent_class)->finalize (object);
}

GdkKeymap* gdk_qnxscreen_keymap_get_keymap(GdkDisplay* display)
{
    GdkQnxScreenDisplay *qnxscreen_display = NULL;

    g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);
    qnxscreen_display = GDK_QNXSCREEN_DISPLAY (display);

    if (!qnxscreen_display->keymap) {
        qnxscreen_display->keymap = g_object_new (gdk_qnxscreen_keymap_get_type (), NULL);
    }

    qnxscreen_display->keymap->display = display;

    return qnxscreen_display->keymap;
}

static PangoDirection gdk_qnxscreen_keymap_get_direction (GdkKeymap* keymap)
{
    return PANGO_DIRECTION_NEUTRAL;
}

static gboolean gdk_qnxscreen_keymap_have_bidi_layouts (GdkKeymap* keymap)
{
    return FALSE;
}

static gboolean gdk_qnxscreen_keymap_get_caps_lock_state (GdkKeymap* keymap)
{
    return FALSE;
}

static gboolean gdk_qnxscreen_keymap_get_num_lock_state (GdkKeymap* keymap)
{
    return FALSE;
}

static gboolean gdk_qnxscreen_keymap_get_scroll_lock_state (GdkKeymap* keymap)
{
    return FALSE;
}

static gboolean gdk_qnxscreen_keymap_get_entries_for_keyval (GdkKeymap* keymap, guint keyval, GArray* retval)
{
    GdkKeymapKey key;
    key.group = 0;
    key.level = 0;

    switch (keyval)
    {
        case GDK_KEY_BackSpace:
            key.keycode = KEYCODE_BACKSPACE;
            break;

        case GDK_KEY_uparrow:
            key.keycode = KEYCODE_UP;
            break;

        case GDK_KEY_downarrow:
            key.keycode = KEYCODE_DOWN;
            break;

        case GDK_KEY_leftarrow:
            key.keycode = KEYCODE_LEFT;
            break;

        case GDK_KEY_rightarrow:
            key.keycode = KEYCODE_RIGHT;
            break;

        default:
            key.keycode = keyval;
            break;
    }

    g_array_append_val (retval, key);
    
    return TRUE;
}

static gboolean gdk_qnxscreen_keymap_get_entries_for_keycode (
    GdkKeymap     *keymap,
	guint          hardware_keycode,
	GdkKeymapKey **keys,
	guint        **keyvals,
	int           *n_entries)
{
    if (n_entries)
        *n_entries = 1;
    if (keys) {
        *keys = g_new0 (GdkKeymapKey, 1);
        (*keys)->keycode = hardware_keycode;
    }
    if (keyvals) {
        *keyvals = g_new0 (guint, 1);
        (*keyvals)[0] = hardware_keycode;
    }
    return TRUE;
}

static guint gdk_qnxscreen_keymap_lookup_key (GdkKeymap* keymap, const GdkKeymapKey* key)
{
    return key->keycode;
}

static gboolean gdk_qnxscreen_keymap_translate_keyboard_state (
    GdkKeymap       *keymap,
	guint            hardware_keycode,
	GdkModifierType  state,
	int              group,
	guint           *keyval,
	int             *effective_group,
	int             *level,
	GdkModifierType *consumed_modifiers)
{
    switch (hardware_keycode)
    {
        case KEYCODE_BACKSPACE:
            *keyval = GDK_KEY_BackSpace;
            break;

        case KEYCODE_UP:
            *keyval = GDK_KEY_uparrow;
            break;

        case KEYCODE_DOWN:
            *keyval = GDK_KEY_downarrow;
            break;

        case KEYCODE_LEFT:
            *keyval = GDK_KEY_leftarrow;
            break;

        case KEYCODE_RIGHT:
            *keyval = GDK_KEY_rightarrow;
            break;

        default:
            *keyval = hardware_keycode;
            break;
    }

    return TRUE;
}

static void gdk_qnxscreen_keymap_class_init (GdkQnxScreenKeymapClass* klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GdkKeymapClass *keymap_class = GDK_KEYMAP_CLASS (klass);

  object_class->finalize = gdk_qnxscreen_keymap_finalize;

  keymap_class->get_direction = gdk_qnxscreen_keymap_get_direction;
  keymap_class->have_bidi_layouts = gdk_qnxscreen_keymap_have_bidi_layouts;
  keymap_class->get_caps_lock_state = gdk_qnxscreen_keymap_get_caps_lock_state;
  keymap_class->get_num_lock_state = gdk_qnxscreen_keymap_get_num_lock_state;
  keymap_class->get_scroll_lock_state = gdk_qnxscreen_keymap_get_scroll_lock_state;
  keymap_class->get_entries_for_keyval = gdk_qnxscreen_keymap_get_entries_for_keyval;
  keymap_class->get_entries_for_keycode = gdk_qnxscreen_keymap_get_entries_for_keycode;
  keymap_class->lookup_key = gdk_qnxscreen_keymap_lookup_key;
  keymap_class->translate_keyboard_state = gdk_qnxscreen_keymap_translate_keyboard_state;
}

