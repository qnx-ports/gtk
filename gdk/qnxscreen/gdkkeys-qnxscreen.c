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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "gdkdisplay-qnxscreen-data.h"
#include "gdk/gdkkeysyms.h"
#include "gdkkeys-qnxscreen.h"
#include "gdkprivate-qnxscreen.h"
#include <sys/keycodes.h>
#include <errno.h>
#include <limits.h>
#include <sys/keycodes.h>

G_DEFINE_TYPE (GdkQnxScreenKeymap, gdk_qnxscreen_keymap, GDK_TYPE_KEYMAP)

static void
gdk_qnxscreen_keymap_init (GdkQnxScreenKeymap *keymap)
{
}

static void
gdk_qnxscreen_keymap_finalize (GObject *object)
{
  G_OBJECT_CLASS (gdk_qnxscreen_keymap_parent_class)->finalize (object);
}

GdkKeymap *
gdk_qnxscreen_keymap_get_keymap (GdkDisplay *display)
{
  GdkQnxScreenDisplay *qnxscreen_display = NULL;

  g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);
  qnxscreen_display = GDK_QNXSCREEN_DISPLAY (display);

  if (!qnxscreen_display->keymap)
    {
      qnxscreen_display->keymap = g_object_new (gdk_qnxscreen_keymap_get_type (), NULL);
    }

  qnxscreen_display->keymap->display = display;

  return qnxscreen_display->keymap;
}

static PangoDirection
gdk_qnxscreen_keymap_get_direction (GdkKeymap *keymap)
{
  return PANGO_DIRECTION_NEUTRAL;
}

static gboolean
gdk_qnxscreen_keymap_have_bidi_layouts (GdkKeymap *keymap)
{
  return FALSE;
}

/**
 * This function always returns false because the capslock status can only 
 * be retrieved in an keyboard event. 
 */
static gboolean
gdk_qnxscreen_keymap_get_caps_lock_state (GdkKeymap *keymap)
{
  return FALSE;
}

static gboolean
gdk_qnxscreen_keymap_get_num_lock_state (GdkKeymap *keymap)
{
  return FALSE;
}

static gboolean
gdk_qnxscreen_keymap_get_scroll_lock_state (GdkKeymap *keymap)
{
  return FALSE;
}

// TODO: Check other implementations to figure out this function. 
static gboolean
gdk_qnxscreen_keymap_get_entries_for_keyval (GdkKeymap *keymap, guint keyval, GArray *retval)
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

static gboolean
gdk_qnxscreen_keymap_get_entries_for_keycode (
    GdkKeymap *keymap,
    guint hardware_keycode,
    GdkKeymapKey **keys,
    guint **keyvals,
    int *n_entries)
{
  if (n_entries)
    *n_entries = 1;
  if (keys)
    {
      *keys = g_new0 (GdkKeymapKey, 1);
      (*keys)->keycode = hardware_keycode;
    }
  if (keyvals)
    {
      *keyvals = g_new0 (guint, 1);
      (*keyvals)[0] = hardware_keycode;
    }
  return TRUE;
}

static guint
gdk_qnxscreen_keymap_lookup_key (GdkKeymap *keymap, const GdkKeymapKey *key)
{
  return key->keycode;
}

/**
 * Translate QNX key event code to GDK conventions
 * The function will first match the corresponding keycode defined in sys/keycodes.h and gdkkeysyms.h
 * Then it will decided wether or not to consume certain keys and choose the correct symbol when shift is used.
 * 
 * @param hardware_keycode The symbol value or cap value of the key event. Use cap val when ctrl/alt is pressed
 * @param state The modifier flags translated in GDK style
 * @param group Not used in US keyboard and we don't support it at this moment. https://docs.gtk.org/gdk4/keys.html
 * @param keyval Output param for keyval
 * @param effective_group output param for group, not used
 * @param level output param for level
 * @param consumed_modifiers output param for those modifiers used to determine the group and level
 */

static gboolean
gdk_qnxscreen_keymap_translate_keyboard_state (
    GdkKeymap *keymap,
    guint hardware_keycode,
    GdkModifierType state,
    int group,
    guint *keyval,
    int *effective_group,
    int *level,
    GdkModifierType *consumed_modifiers)
{
  g_return_val_if_fail(GDK_IS_QNXSCREEN_KEYMAP(keymap), FALSE);
  GdkQnxScreenKeymap *self = GDK_QNXSCREEN_KEYMAP (keymap);
  GdkModifierType rtv_consumed_modifiers = GDK_NO_MODIFIER_MASK;
  guint rtv_keyval = hardware_keycode;
  int rtv_level = 0;
  // For most of the KEYCODE_PC_KEYS in QNX, they are 0xf00 off from the GDK ones.
  // From gdk_key_event_matches, GTK interprate <Shift><Tab> to GDK_KEY_ISO_Left_Tab
  if (hardware_keycode > KEYCODE_PC_KEYS && hardware_keycode < KEYCODE_CONSUMER_KEYS)
  {
    rtv_keyval = hardware_keycode + 0xf00;
    // Some exceptions
    if (hardware_keycode == KEYCODE_SYSREQ)
      rtv_keyval = GDK_KEY_Sys_Req;
  }

  // if num lock is not up, use the functional key instead of the number
  // QNX only has one value for all KP caps. And it is the KP number caps in GDK + 0xf00
  // Despite that they named it as functional keys.
  if (hardware_keycode == KEYCODE_KP_HOME  || hardware_keycode == KEYCODE_KP_UP   ||
      hardware_keycode == KEYCODE_KP_PG_UP || hardware_keycode == KEYCODE_KP_LEFT ||
      hardware_keycode == KEYCODE_KP_RIGHT || hardware_keycode == KEYCODE_KP_END  ||
      hardware_keycode == KEYCODE_KP_DOWN  || hardware_keycode == KEYCODE_KP_PG_DOWN )
  {
    if ((state & GDK_MOD2_MASK))
    {
      rtv_level = 1;
      rtv_consumed_modifiers |= GDK_MOD2_MASK;
    } else {
      if (hardware_keycode == KEYCODE_KP_HOME) {
        rtv_keyval = GDK_KEY_KP_Home;
      } else if (hardware_keycode == KEYCODE_KP_UP) {
        rtv_keyval = GDK_KEY_KP_Up;
      } else if (hardware_keycode == KEYCODE_KP_PG_UP) {
        rtv_keyval = GDK_KEY_Page_Up;
      } else if (hardware_keycode == KEYCODE_KP_LEFT) {
        rtv_keyval = GDK_KEY_KP_Left;
      } else if (hardware_keycode == KEYCODE_KP_RIGHT) {
        rtv_keyval = GDK_KEY_KP_Right;
      } else if (hardware_keycode == KEYCODE_KP_END) {
        rtv_keyval = GDK_KEY_KP_End;
      } else if (hardware_keycode == KEYCODE_KP_DOWN) {
        rtv_keyval = GDK_KEY_KP_Down;
      } else if (hardware_keycode == KEYCODE_KP_PG_DOWN) {
        rtv_keyval = GDK_KEY_KP_Page_Down;
      }
    }
  }

  // TODO: Essentially we are hardcoding a sticky modifier?
  // The best way is to have a qnxscreen API to query keys according to the layout instead of hardcode here

  // Now Check the modifiers and decide if we want to consume them
  // On QNX, when Ctrl/Alt is used, the sym val will become invalid, only cap val is available
  // Thus we need to check if shift modifier is on and use a different level if so.
  // And for this portion GDK KEY matches QNX KEY
  if (hardware_keycode >= KEYCODE_EXCLAM && hardware_keycode <= KEYCODE_TILDE) {
    // Lower case key or only cap value is reported
    if (hardware_keycode >= KEYCODE_A && hardware_keycode <= KEYCODE_Z) {
      if (state & (GDK_LOCK_MASK | GDK_SHIFT_MASK)) {
        rtv_consumed_modifiers |= GDK_LOCK_MASK | GDK_SHIFT_MASK;
      } else if (state & GDK_LOCK_MASK) {
        rtv_consumed_modifiers |= GDK_LOCK_MASK;
        rtv_level = 1;
        rtv_keyval -= 0x20;
      } else if (state & GDK_SHIFT_MASK) {
        rtv_consumed_modifiers |= GDK_LOCK_MASK;
        rtv_level = 1;
        rtv_keyval -= 0x20;
      }
    } 
    // Higher case key sym reported normally, no need to remapping the keycode
    else if (hardware_keycode >= KEYCODE_CAPITAL_A && hardware_keycode <= KEYCODE_CAPITAL_Z) {
      if (state & GDK_LOCK_MASK) {
        rtv_consumed_modifiers |= GDK_LOCK_MASK;
        rtv_level = 0;
      } else if (state & GDK_SHIFT_MASK) {
        rtv_consumed_modifiers |= GDK_LOCK_MASK;
        rtv_level = 0;
      }
    }
    // For all other values, no simple rules apply
    else {
      if (state & GDK_SHIFT_MASK) {
        rtv_consumed_modifiers |= GDK_SHIFT_MASK;
        rtv_level = 1;
        if (hardware_keycode == KEYCODE_GRAVE) {
          rtv_keyval = KEYCODE_TILDE;
        } else if (hardware_keycode == KEYCODE_ZERO) {
          rtv_keyval = KEYCODE_RIGHT_PAREN;
        } else if (hardware_keycode == KEYCODE_ONE) {
          rtv_keyval = KEYCODE_EXCLAM;
        } else if (hardware_keycode == KEYCODE_TWO) {
          rtv_keyval = KEYCODE_AT;
        } else if (hardware_keycode == KEYCODE_THREE) {
          rtv_keyval = KEYCODE_NUMBER;
        } else if (hardware_keycode == KEYCODE_FOUR) {
          rtv_keyval = KEYCODE_DOLLAR;
        } else if (hardware_keycode == KEYCODE_FIVE) {
          rtv_keyval = KEYCODE_PERCENT;
        } else if (hardware_keycode == KEYCODE_SIX) {
          rtv_keyval = KEYCODE_CIRCUMFLEX;
        } else if (hardware_keycode == KEYCODE_SEVEN) {
          rtv_keyval = KEYCODE_AMPERSAND;
        } else if (hardware_keycode == KEYCODE_EIGHT) {
          rtv_keyval = KEYCODE_ASTERISK;
        } else if (hardware_keycode == KEYCODE_NINE) {
          rtv_keyval = KEYCODE_LEFT_PAREN;
        } else if (hardware_keycode == KEYCODE_MINUS) {
          rtv_keyval = KEYCODE_UNDERSCORE;
        } else if (hardware_keycode == KEYCODE_EQUAL) {
          rtv_keyval = KEYCODE_PLUS;
        }  else if (hardware_keycode == KEYCODE_LEFT_BRACKET) {
          rtv_keyval = KEYCODE_LEFT_BRACE;
        } else if (hardware_keycode == KEYCODE_RIGHT_BRACKET) {
          rtv_keyval = KEYCODE_RIGHT_BRACE;
        } else if (hardware_keycode == KEYCODE_BACK_SLASH) {
          rtv_keyval = KEYCODE_BAR;
        } else if (hardware_keycode == KEYCODE_SEMICOLON) {
          rtv_keyval = KEYCODE_COLON;
        } else if (hardware_keycode == KEYCODE_APOSTROPHE) {
          rtv_keyval = KEYCODE_QUOTE;
        } else if (hardware_keycode == KEYCODE_COMMA) {
          rtv_keyval = KEYCODE_LESS_THAN;
        } else if (hardware_keycode == KEYCODE_PERIOD) {
          rtv_keyval = KEYCODE_GREATER_THAN;
        } else if (hardware_keycode == KEYCODE_SLASH) {
          rtv_keyval = KEYCODE_QUESTION;
        }
      }
    }
  }
   
  if (keyval) {*keyval = rtv_keyval; }
  if (level) {*level = rtv_level; }
  if (consumed_modifiers) {*consumed_modifiers=rtv_consumed_modifiers; }
  return TRUE;
}

static void
gdk_qnxscreen_keymap_class_init (GdkQnxScreenKeymapClass *klass)
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

/**
 * Translate from QNX modifier flags to GDK modifier Type
 * 
 * @param mods QNX flags property associated with keyboard event
 * @return The equivalent Gdk Modifier
 */
GdkModifierType _gdk_qnxscreen_keymap_get_gdk_modifiers (GdkKeymap *keymap, guint32 mods)
{
  GdkModifierType state = GDK_NO_MODIFIER_MASK;
  if ( mods & KEYMOD_SHIFT ) {
    state |= GDK_SHIFT_MASK;
  }
  if ( mods & KEYMOD_CTRL ) {
    state |= GDK_CONTROL_MASK;
  }
  if ( mods & KEYMOD_ALT ) {
    state |= GDK_ALT_MASK;
  }
  if ( mods & KEYMOD_CAPS_LOCK ) {
    state |= GDK_LOCK_MASK;
  }
  if ( mods & KEYMOD_NUM_LOCK ) {
    state |= GDK_MOD2_MASK;
  }

  return state;
}