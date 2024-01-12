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
#include <sys/keycodes.h>
#include <screen/screen.h>
#include "gdkdevice-qnxscreen-data.h"
#include "gdkdisplay-qnxscreen-data.h"
#include "gdksurface-qnxscreen-data.h"
#include "gdkeventloop-qnxscreen.h"
#include "gdk_qnxscreen_common.h"

G_DEFINE_TYPE (GdkQnxScreenDevice, gdk_qnxscreen_device, GDK_TYPE_DEVICE)

static void copy_qnxscreen_device(const GdkQnxScreenDevice* src, GdkQnxScreenDevice* dest)
{
    dest->buttons = src->buttons;
    dest->surface = src->surface;
    dest->window_handle = src->window_handle;
    memcpy(dest->root_pos, src->root_pos, sizeof(dest->root_pos));
    memcpy(dest->window_pos, src->window_pos, sizeof(dest->window_pos));
}

static void gdk_qnxscreen_device_set_surface_cursor(GdkDevice* device, GdkSurface* surface, GdkCursor* cursor)
{
}

static GdkSurface* gdk_qnxscreen_device_surface_at_position(GdkDevice* device, double* win_x, double* win_y, GdkModifierType* mask)
{
    GdkQnxScreenDevice* impl = GDK_QNXSCREEN_DEVICE(device);

    if (win_x) {
      *win_x = impl->window_pos[0];
    }

    if (win_y) {
      *win_y = impl->window_pos[1];
    }

    return impl->surface;
}

static GdkGrabStatus gdk_qnxscreen_device_grab(GdkDevice* device, GdkSurface* surface, gboolean owner_events, GdkEventMask event_mask, GdkSurface* confine_to, GdkCursor* cursor, guint32 time_)
{
    return GDK_GRAB_SUCCESS;
}

static void gdk_qnxscreen_device_ungrab(GdkDevice* device, guint32 time_)
{
}

static void gdk_qnxscreen_device_init(GdkQnxScreenDevice* device)
{
    GDK_DEBUG(MISC, "%s initializing GdkQnxScreenDevice", QNX_SCREEN);
    device->window_handle = NULL;
    memset(device->root_pos, 0, sizeof(device->root_pos));
    memset(device->window_pos, 0, sizeof(device->window_pos));
    device->buttons = 0;
    device->surface = NULL;
}

static void gdk_qnxscreen_device_class_init(GdkQnxScreenDeviceClass* class)
{
    GDK_DEBUG(MISC, "%s initializing GdkQnxScreenDeviceClass", QNX_SCREEN);
    GdkDeviceClass* device_class = GDK_DEVICE_CLASS(class);
    device_class->surface_at_position = gdk_qnxscreen_device_surface_at_position;
    device_class->set_surface_cursor = gdk_qnxscreen_device_set_surface_cursor;
    device_class->grab = gdk_qnxscreen_device_grab;
    device_class->ungrab = gdk_qnxscreen_device_ungrab;
}

int gdk_qnxscreen_device_init_seat(GdkDisplay* display)
{
    GDK_DEBUG(MISC, "%s initializing GdkSeat and devices", QNX_SCREEN);

    int ret = 0;
    GdkQnxScreenDisplay* qnx_screen_display = GDK_QNXSCREEN_DISPLAY(display);

    /* Initialize core pointer logical device */
    if (ret == 0) {
        qnx_screen_display->core_pointer = g_object_new(
            GDK_TYPE_QNXSCREEN_DEVICE,
            "name", "Core Pointer",
            "source", GDK_SOURCE_MOUSE,
            "has-cursor", TRUE,
            "display", display,
            NULL);

        if (qnx_screen_display->core_pointer == NULL) {
            g_critical(G_STRLOC ": failed to create GDK_SOURCE_MOUSE: %s", strerror(errno));
            ret = -1;
        } else {
            GDK_DEBUG(MISC, "%s created Core Pointer device", QNX_SCREEN);
        }
    }

    /* Initialize core keyboard logical device */
    if (ret == 0) {
        qnx_screen_display->core_keyboard = g_object_new(
            GDK_TYPE_QNXSCREEN_DEVICE,
            "name", "Core Keyboard",
            "source", GDK_SOURCE_KEYBOARD,
            "has-cursor", FALSE,
            "display", display,
            NULL);

        if (qnx_screen_display->core_keyboard == NULL) {
            g_critical(G_STRLOC ": failed to create GDK_SOURCE_KEYBOARD: %s", strerror(errno));
            ret = -1;
        } else {
            GDK_DEBUG(MISC, "%s created Core Keyboard device", QNX_SCREEN);
        }
    }

    /* initialize mouse pointer  */
    if (ret == 0) {
        qnx_screen_display->phys_pointer = g_object_new(
            GDK_TYPE_QNXSCREEN_DEVICE,
            "name", "Pointer",
            "source", GDK_SOURCE_MOUSE,
            "has-cursor", TRUE,
            "display", display,
            NULL);
        
        if (qnx_screen_display->phys_pointer == NULL) {
            g_critical(G_STRLOC ": failed to create GDK_SOURCE_MOUSE: %s", strerror(errno));
            ret = -1;
        } else {
            GDK_DEBUG(MISC, "%s created GDK_SOURCE_MOUSE", QNX_SCREEN);
        }
    }

    /* initialize keyboard */
    if (ret == 0) {
        qnx_screen_display->phys_keyboard = g_object_new(
            GDK_TYPE_QNXSCREEN_DEVICE,
            "name", "Keyboard",
            "source", GDK_SOURCE_KEYBOARD,
            "has-cursor", FALSE,
            "display", display,
            NULL);

        if (qnx_screen_display->phys_keyboard == NULL) {
            g_critical(G_STRLOC ": failed to create GDK_SOURCE_KEYBOARD: %s", strerror(errno));
            ret = -1;
        } else {
            GDK_DEBUG(MISC, "%s created GDK_SOURCE_KEYBOARD", QNX_SCREEN);
        }
    }

    /* initialize touchscreen */
    if (ret == 0) {
        qnx_screen_display->phys_touchscreen = g_object_new(
            GDK_TYPE_QNXSCREEN_DEVICE,
            "name", "Touchscreen",
            "source", GDK_SOURCE_TOUCHSCREEN,
            "has-cursor", FALSE,
            "display", display,
            NULL);

        if (qnx_screen_display->phys_touchscreen == NULL) {
            g_critical(G_STRLOC ": failed to create GDK_SOURCE_TOUCHSCREEN: %s", strerror(errno));
            ret = -1;
        } else {
            GDK_DEBUG(MISC, "%s created GDK_SOURCE_TOUCHSCREEN", QNX_SCREEN);
        }
    }

    /* associate devices */
    if (ret == 0) {
        _gdk_device_set_associated_device (qnx_screen_display->core_pointer, qnx_screen_display->core_keyboard);
        _gdk_device_set_associated_device (qnx_screen_display->core_keyboard, qnx_screen_display->core_pointer);
        _gdk_device_set_associated_device (qnx_screen_display->phys_pointer, qnx_screen_display->core_pointer);
        _gdk_device_set_associated_device (qnx_screen_display->phys_keyboard, qnx_screen_display->core_keyboard);
        _gdk_device_set_associated_device (qnx_screen_display->phys_touchscreen, qnx_screen_display->core_pointer);
        _gdk_device_add_physical_device (qnx_screen_display->core_pointer, qnx_screen_display->phys_touchscreen);
    }

    /* create GDKSeat */
    if (ret == 0) {
        qnx_screen_display->seat = gdk_seat_default_new_for_logical_pair(qnx_screen_display->core_pointer, qnx_screen_display->core_keyboard);
        if (qnx_screen_display->seat == NULL) {
            g_critical(G_STRLOC ": failed to create GDKSeat: %s", strerror(errno));
            ret = -1;
        } else {
            GDK_DEBUG(MISC, "%s created GDKSeat", QNX_SCREEN);
        }
    }

    /* initialize GDKSeat */
    if (ret == 0) {
        gdk_display_add_seat(display, qnx_screen_display->seat);
        gdk_seat_default_add_physical_device(GDK_SEAT_DEFAULT(qnx_screen_display->seat), qnx_screen_display->phys_pointer);
        gdk_seat_default_add_physical_device(GDK_SEAT_DEFAULT(qnx_screen_display->seat), qnx_screen_display->phys_keyboard);
        gdk_seat_default_add_physical_device(GDK_SEAT_DEFAULT(qnx_screen_display->seat), qnx_screen_display->phys_touchscreen);
    }

    return ret;
}

static void emit_button_events(GdkDisplay* display, GdkDevice* device, GdkQnxScreenDevice* prev_pointer)
{
    GDK_DEBUG(EVENTS, "%s handling pointer button event", QNX_SCREEN);

    static int screen_buttons[] = { 
        SCREEN_LEFT_MOUSE_BUTTON,
        SCREEN_MIDDLE_MOUSE_BUTTON,
        SCREEN_RIGHT_MOUSE_BUTTON ,
    };

    static int gdk_buttons[] = { 
        GDK_BUTTON_PRIMARY,
        GDK_BUTTON_MIDDLE,
        GDK_BUTTON_SECONDARY,
    };

    static int gdk_button_masks[] = {
        GDK_BUTTON1_MASK,
        GDK_BUTTON2_MASK,
        GDK_BUTTON3_MASK,
    };

    GdkQnxScreenDevice* pointer = GDK_QNXSCREEN_DEVICE(device);
    GdkEvent* event = NULL;
    int event_type = 0;
    int button_mask = 0;

    for (int i=0; i<3; i++)
    {
        if (((pointer->buttons ^ prev_pointer->buttons) & screen_buttons[i]) == 0) {
            /* no change in button state */
            continue;
        }

        event_type = (pointer->buttons & screen_buttons[i]) == 0 ? GDK_BUTTON_RELEASE : GDK_BUTTON_PRESS;
        if (event_type == GDK_BUTTON_RELEASE) {
            button_mask = gdk_button_masks[i];
        }        

        /* create button event and deliver */
        event = gdk_button_event_new (
            event_type,
            pointer->surface,
            device,
            NULL,
            GDK_QNXSCREEN_TIME(),
            button_mask,                              // todo Record and set key modifiers (CTRL, ALT, etc.) here.
            gdk_buttons[i],
            pointer->window_pos[0],
            pointer->window_pos[1],
            NULL);        

        GdkButtonEvent* button_event = (GdkButtonEvent*) event;
        GDK_DEBUG (
            EVENTS, 
            "%s button %d %s, state %d pos %d,%d surface %p",
            QNX_SCREEN,
            button_event->button,
            event_type == GDK_BUTTON_PRESS ? "press" : "release",
            button_event->state,
            pointer->window_pos[0], pointer->window_pos[1],
            pointer->surface);

        gdk_qnxscreen_event_deliver_event(display, event);
    }
}

static void emit_motion_event(GdkDisplay* display, GdkDevice* device)
{
    GDK_DEBUG(EVENTS, "%s handling pointer motion event", QNX_SCREEN);

    GdkQnxScreenDevice* pointer = GDK_QNXSCREEN_DEVICE(device);

    GdkEvent* event = gdk_motion_event_new (
        pointer->surface,
        device,
        NULL,
        GDK_QNXSCREEN_TIME(),
        0,                              // todo Record and set key modifiers (CTRL, ALT, etc.) here.
        pointer->window_pos[0],
        pointer->window_pos[1],
        NULL);

    GDK_DEBUG(
        EVENTS,
        "%s motion pos %d,%d surface %p",
        QNX_SCREEN,
        pointer->window_pos[0], 
        pointer->window_pos[1],
        pointer->surface);

    gdk_qnxscreen_event_deliver_event (display, event);
}

static void emit_surface_crossing_event(GdkDisplay* display, GdkQnxScreenDevice* pointer, GdkQnxScreenDevice* prev_pointer)
{
    GDK_DEBUG(EVENTS, "%s handling pointer surface crossing event", QNX_SCREEN);
    
    GdkQnxScreenDisplay* qnx_screen_display = GDK_QNXSCREEN_DISPLAY(display);
    GdkEvent* event = NULL;
    
    /* first send a leave notify event for the previous surface (if one exists) */
    if (prev_pointer != NULL && NULL != prev_pointer->surface) {
        event = gdk_crossing_event_new (
            GDK_LEAVE_NOTIFY,
            prev_pointer->surface,
            qnx_screen_display->core_pointer,
            GDK_QNXSCREEN_TIME(),
            0,                          // todo Record and set key modifiers (CTRL, ALT, etc.) here.
            prev_pointer->window_pos[0],
            prev_pointer->window_pos[1],
            GDK_CROSSING_NORMAL,
            GDK_NOTIFY_ANCESTOR);
        gdk_qnxscreen_event_deliver_event(display, event);
        GDK_DEBUG(EVENTS, "%s LEAVE_NOTIFY surface %p", QNX_SCREEN, prev_pointer->surface);
    }

    /* second send a entry notify event for the new surface */
    event = gdk_crossing_event_new (
        GDK_ENTER_NOTIFY,
        pointer->surface,
        qnx_screen_display->core_pointer,
        GDK_QNXSCREEN_TIME(),
        0,                          // todo Record and set key modifiers (CTRL, ALT, etc.) here.
        pointer->window_pos[0], 
        pointer->window_pos[1],
        GDK_CROSSING_NORMAL,
        GDK_NOTIFY_ANCESTOR);
    gdk_qnxscreen_event_deliver_event(display, event);
    GDK_DEBUG(EVENTS, "%s ENTER_NOTIFY surface %p", QNX_SCREEN, pointer->surface);

    /* third send a lost focus event for the old surface */
    if (prev_pointer != NULL && NULL != prev_pointer->surface) {
        event = gdk_focus_event_new(prev_pointer->surface, qnx_screen_display->core_keyboard, FALSE);
        gdk_qnxscreen_event_deliver_event(display, event);
        GDK_DEBUG(EVENTS, "%s LOST focus %p", QNX_SCREEN, prev_pointer->surface);
    }

    /* finally send a focus event for the new surface */
    event = gdk_focus_event_new(pointer->surface, qnx_screen_display->core_keyboard, TRUE);
    gdk_qnxscreen_event_deliver_event(display, event);
    GDK_DEBUG(EVENTS, "%s GAINED focus %p", QNX_SCREEN, pointer->surface);
}

void gdk_qnxscreen_device_pointer_event(GdkDisplay* display)
{
    GDK_DEBUG(EVENTS, "%s handling pointer event", QNX_SCREEN);

    int ret = 0;
    GdkQnxScreenDisplay* qnx_screen_display = GDK_QNXSCREEN_DISPLAY(display);
    GdkQnxScreenDevice* pointer_state = GDK_QNXSCREEN_DEVICE(qnx_screen_display->core_pointer);
    GdkQnxScreenDevice prev_pointer_state;
    screen_window_t window_handle = NULL;

    /* copy previous pointer data */
    copy_qnxscreen_device(pointer_state, &prev_pointer_state);

    /* get the new button state */
    if (ret == 0) {
        ret = screen_get_event_property_iv(qnx_screen_display->event, SCREEN_PROPERTY_BUTTONS, &pointer_state->buttons);
        if (ret == -1) {
            g_critical (G_STRLOC "failed to get pointer buttons property: %s", strerror(errno));
        }
    }

    /* get the event's position */
    if (ret == 0) {
        ret = screen_get_event_property_iv(qnx_screen_display->event, SCREEN_PROPERTY_POSITION, pointer_state->root_pos);
        if (ret == -1) {
            g_critical (G_STRLOC "failed to get pointer position property: %s", strerror(errno));
        }
    }

    /* get the event's source position */
    if (ret == 0) {
        ret = screen_get_event_property_iv (qnx_screen_display->event, SCREEN_PROPERTY_SOURCE_POSITION, pointer_state->window_pos);
        if (ret == -1) {
            g_critical (G_STRLOC "failed to get pointer source position property: %s", strerror(errno));
        }
    }

    /* find the window over which the event occurred */
    if (ret == 0) {
        ret = screen_get_event_property_pv(qnx_screen_display->event, SCREEN_PROPERTY_WINDOW, (void **)&window_handle);
        if (ret == -1) {
            g_critical (G_STRLOC "failed to get pointer window property: %s", strerror(errno));
        }
    }

    /* Translate the native window to a GDKSurface */
    if (ret == 0) {
        if (window_handle != pointer_state->window_handle) {
            pointer_state->window_handle = window_handle;
            pointer_state->surface = g_hash_table_lookup(qnx_screen_display->surface_window_table, window_handle);
        }
    }

    /* send surface change event */
    if (ret == 0) {
        if (pointer_state->surface != prev_pointer_state.surface) {
            emit_surface_crossing_event(display, pointer_state, &prev_pointer_state);
        }
    }

    /* sent button events */
    if (ret == 0) {
        if (pointer_state->buttons != prev_pointer_state.buttons) {
            emit_button_events(display, qnx_screen_display->core_pointer, &prev_pointer_state);
        }
    }

    /* send motion event */
    if (ret == 0) {
        if ((pointer_state->root_pos[0] != prev_pointer_state.root_pos[0]) || (pointer_state->root_pos[1] != prev_pointer_state.root_pos[1])) {
            emit_motion_event(display, qnx_screen_display->core_pointer);
        }
    }
}

static void emit_touch_event(GdkDisplay* display, GdkQnxScreenDevice* touch_state, int touch_id, GdkEventType touch_type)
{
    int ret = 0;
    GdkQnxScreenDisplay *display_impl = GDK_QNXSCREEN_DISPLAY(display);
    GdkQnxScreenSurface *surface_impl = GDK_QNXSCREEN_SURFACE(touch_state->surface);
    /* Look for an active touch seqeuence for this touch_id on this surface */
    int touch_sequence = GPOINTER_TO_INT(g_hash_table_lookup(surface_impl->active_touch_table, GINT_TO_POINTER(touch_id)));
    int buttons = GDK_BUTTON1_MASK;

    switch ((int)touch_type) {
        case GDK_TOUCH_BEGIN:
            /* If there is an active touch sequence for this touch_id */
            if (touch_sequence) {
                /* This should not happen. Input control should handle this */
                g_warning (G_STRLOC " Got TOUCH_BEGIN, but touch_id %d already has an active sequence on the surface: 0x%p.", touch_id, touch_state->surface);
            } else {
                /* Remove this touch_id from the refocussed touch_table, regardless of whether or not it was there */
                g_hash_table_remove(surface_impl->refocussed_touch_table, GINT_TO_POINTER(touch_id));
                /* Allocate and register a new touch sequence for this touch_id on this surface */
                touch_sequence = display_impl->next_touch_sequence;
                display_impl->next_touch_sequence++;
                g_hash_table_insert(surface_impl->active_touch_table, GINT_TO_POINTER(touch_id), GINT_TO_POINTER(touch_sequence));
            }
            break;
        case GDK_TOUCH_UPDATE:
            /* If there's no active touch sequence for this touch_id */
            if (!touch_sequence) {
                if (g_hash_table_lookup(surface_impl->refocussed_touch_table, GINT_TO_POINTER(touch_id))) {
                    /* Ignore silently and return if this is from a refocussed touch (started on another surface) */
                    GDK_DEBUG(EVENTS, "%s Ignoring GDK_TOUCH_UPDATE for a refocussed touch.", QNX_SCREEN);
                    return;
                } else {
                    /* This should not happen. Input control should handle this */
                    g_warning (G_STRLOC " Got TOUCH_UPDATE, but there is no touch sequence active on this surface: 0x%p", touch_state->surface);
                }
            }
            break;
        case GDK_TOUCH_END:
        case GDK_TOUCH_CANCEL:
            /* zero button_mask for cancel/end */
            buttons = 0;
            /* If there's no active touch sequence for this touch_id */
            if (!touch_sequence) {
                if (g_hash_table_lookup(surface_impl->refocussed_touch_table, GINT_TO_POINTER(touch_id))) {
                    /* Remove from refocussed touch table, and ignore this event. Return silently. */
                    g_hash_table_remove(surface_impl->refocussed_touch_table, GINT_TO_POINTER(touch_id));
                    /* Ignore silently and return */
                    GDK_DEBUG (EVENTS, "%s Ignoring GDK_TOUCH_END/CANCEL for a refocussed touch.", QNX_SCREEN);
                    return;
                } else {
                    /* This should not happen. Input control should handle this */
                    g_warning (G_STRLOC " Got TOUCH_END or TOUCH_CANCEL, but there is no touch sequence active on this surface: 0x%p", touch_state->surface);
                }
            } else {
                g_hash_table_remove(surface_impl->active_touch_table, GINT_TO_POINTER(touch_id));
            }
            break;
        default:
            g_critical (G_STRLOC "GdkEvenType (touch_type) unrecognized: %d", touch_type);
            ret = -1;
            break;
    }

    if (0 == ret) {
        /* Set the button mask, based on the event type */
        touch_state->buttons = buttons;

        GdkEvent* event = gdk_touch_event_new (
            touch_type,
            GINT_TO_POINTER(0),
            touch_state->surface,
            display_impl->core_pointer,
            GDK_QNXSCREEN_TIME(),
            touch_state->buttons,
            touch_state->window_pos[0],
            touch_state->window_pos[1],
            NULL,
            /* Indicates that these touch events are emulating the pointer */
            TRUE);

        GDK_DEBUG(
            EVENTS,
            "%s touch-event: %s, sequence: %d, pos: (%d,%d), buttons: 0x%x, surface: %p",
            QNX_SCREEN,
            (touch_type == GDK_TOUCH_BEGIN ? "GDK_TOUCH_BEGIN" :
            touch_type == GDK_TOUCH_UPDATE ? "GDK_TOUCH_UPDATE" :
            touch_type == GDK_TOUCH_END ? "GDK_TOUCH_END" : "GDK_TOUCH_CANCEL"),
            touch_sequence,
            touch_state->window_pos[0],
            touch_state->window_pos[1],
            touch_state->buttons,
            touch_state->surface);

        gdk_qnxscreen_event_deliver_event (display, event);
    }
}

void gdk_qnxscreen_device_touch_event(GdkDisplay* display, int type)
{
    GDK_DEBUG(EVENTS, "%s handling touch event", QNX_SCREEN);

    int ret = 0;
    GdkQnxScreenDisplay* qnx_screen_display = GDK_QNXSCREEN_DISPLAY(display);
    GdkQnxScreenDevice* touch_state = GDK_QNXSCREEN_DEVICE(qnx_screen_display->core_pointer);
    GdkQnxScreenDevice prev_touch_state;
    screen_window_t window_handle = NULL;
    int touch_id = 0;

    GdkEventType touch_type = GDK_TOUCH_BEGIN;

    switch (type) {
        case SCREEN_EVENT_MTOUCH_TOUCH:
            touch_type = GDK_TOUCH_BEGIN;
            break;
        case SCREEN_EVENT_MTOUCH_MOVE:
            touch_type = GDK_TOUCH_UPDATE;
            break;
        case SCREEN_EVENT_MTOUCH_RELEASE:
            touch_type = GDK_TOUCH_END;
            break;
        default:
            g_critical (G_STRLOC "Screen event type unrecognized: %d", type);
            ret = -1;
            break;
    }

    /* copy previous pointer data */
    copy_qnxscreen_device(touch_state, &prev_touch_state);

    /* get the touch ID */
    if (ret == 0) {
        ret = screen_get_event_property_iv(qnx_screen_display->event, SCREEN_PROPERTY_TOUCH_ID, &touch_id);
        if (ret == -1) {
            g_critical (G_STRLOC "failed to get touch id property: %s", strerror(errno));
        }
    }

    /* get the event's source position. Gives the position relative to this window over which the event occurred */
    if (ret == 0) {
        ret = screen_get_event_property_iv (qnx_screen_display->event, SCREEN_PROPERTY_SOURCE_POSITION, touch_state->window_pos);
        if (ret == -1) {
            g_critical (G_STRLOC "failed to get touch source position property: %s", strerror(errno));
        }
    }

    /* get the event's absolute position */
    if (ret == 0) {
        ret = screen_get_event_property_iv(qnx_screen_display->event, SCREEN_PROPERTY_POSITION, touch_state->root_pos);
        if (ret == -1) {
            g_critical (G_STRLOC "screen_get_event_property_iv(SCREEN_PROPERTY_POSITION): %s", strerror(errno));
        }
    }

    /* find the window over which the event occurred */
    if (ret == 0) {
        ret = screen_get_event_property_pv(qnx_screen_display->event, SCREEN_PROPERTY_WINDOW, (void **)&window_handle);
        if (ret == -1) {
            g_critical (G_STRLOC "failed to get touch window property: %s", strerror(errno));
        }
    }

    /* translate the native window to a GDKSurface */
    if (ret == 0) {
        touch_state->window_handle = window_handle;
        touch_state->surface = g_hash_table_lookup(qnx_screen_display->surface_window_table, window_handle);
        if (!GDK_IS_SURFACE(touch_state->surface)) {
            g_critical (G_STRLOC " surface 0x%p mapped to window_handle 0x%p is not valid", touch_state->surface, window_handle);
            ret = -1;
        }
    }

    if (ret == 0) {
        /* If the touch surface has changed (ignore initial transition) */
        if (touch_state->surface != prev_touch_state.surface) {
            emit_surface_crossing_event(display, touch_state, &prev_touch_state);
        }
    }

    if (ret == 0) {
        /* Always emit touch events, whether or not position has changed */
        emit_touch_event(display, GDK_QNXSCREEN_DEVICE(touch_state), touch_id, touch_type);
    }
}

void gdk_qnxscreen_inputcontrol_event(GdkDisplay* display, int subtype)
{
    GdkQnxScreenDisplay* display_impl = GDK_QNXSCREEN_DISPLAY(display);
    GdkSurface* surface = NULL;
    screen_window_t window_handle;
    int touch_id;
    int ret = 0;

    /* Input control events always refer to a window */
    if (ret == 0) {
        ret = screen_get_event_property_pv(display_impl->event, SCREEN_PROPERTY_WINDOW, (void **)&window_handle);
        if (ret == -1) {
            g_critical (G_STRLOC "screen_get_event_property_pv(SCREEN_PROPERTY_WINDOW): %s", strerror(errno));
        }
    }

    /* translate the native window to a GDKSurface */
    if (ret == 0) {
        surface = g_hash_table_lookup(display_impl->surface_window_table, window_handle);
        if (!GDK_IS_SURFACE(surface)) {
            g_critical (G_STRLOC " surface 0x%p mapped to window_handle 0x%p is not valid", surface, window_handle);
            ret = -1;
        }
    }

    switch(subtype) {
    case SCREEN_INPUT_CONTROL_MTOUCH_START:
    case SCREEN_INPUT_CONTROL_MTOUCH_STOP:
    {
        if (ret == 0) {
            ret = screen_get_event_property_iv(display_impl->event, SCREEN_PROPERTY_TOUCH_ID, &touch_id);
            if (ret == -1) {
                g_critical (G_STRLOC "screen_get_event_property_iv(SCREEN_PROPERTY_TOUCH_ID): %s", strerror(errno));
            }
        }

        GdkQnxScreenSurface* surface_impl = GDK_QNXSCREEN_SURFACE(surface);
        if (SCREEN_INPUT_CONTROL_MTOUCH_STOP == subtype) {
             /* TODO: Cancel the current gesture (w/ a TOUCH_CANCEL). The catch, however, is that this is only received upon hiding the surface.
              * Need a way to emit TOUCH_CANCEL before GDK marks things as hidden internally. This may be a bug in the higher-level GdkSurface.
              */
            /* This indicates this gesture sequence is ending without getting any further events.
             * For now, remove all active touches on this surface so gestures at a later time are not deemed invalid
             */
            GDK_DEBUG(EVENTS, "%s MTOUCH_STOP for surface 0x%p - Removing valid touch sequence for touch_id %d.", QNX_SCREEN, surface, touch_id);
            g_hash_table_remove(surface_impl->active_touch_table, GINT_TO_POINTER(touch_id));
        } else if (SCREEN_INPUT_CONTROL_MTOUCH_START == subtype) {
            /* This indicates the surface may get an MTOUCH_MOVE or MTOUCH_RELEASE before getting an MTOUCH_BEGIN.
             * Don't do anything right away, but mark the surface as having a refocussed touch.
             */
            GDK_DEBUG(EVENTS, "%s MTOUCH_START for surface 0x%p - adding valid refocussed touch for touch_id %d ", QNX_SCREEN, surface, touch_id);
            g_hash_table_insert(surface_impl->refocussed_touch_table, GINT_TO_POINTER(touch_id), GINT_TO_POINTER(1));
        }
    }
    break;
    case SCREEN_INPUT_CONTROL_POINTER_START:
    case SCREEN_INPUT_CONTROL_POINTER_STOP:
        GDK_DEBUG(EVENTS, "%s SCREEN_INPUT_CONTROL_POINTER_* events are not handled", QNX_SCREEN);
        break;
    default:
        g_critical (G_STRLOC " Unhandled input control subtype: %d", subtype);
        break;
    } /* switch */
}

void gdk_qnxscreen_device_keyboard_event(GdkDisplay* display)
{
    GDK_DEBUG(EVENTS, "%s handling keyboard event", QNX_SCREEN);

    int ret = 0;
    GdkQnxScreenDisplay* qnx_screen_display = GDK_QNXSCREEN_DISPLAY(display);
    GdkEvent* event = NULL;
    GdkSurface* surface = NULL;
    screen_window_t window_handle = NULL;
    int sym = 0;
    int cap = 0;
    int scan = 0;
    int flags = 0;
    int modifiers = 0;

    /* get the QNX window where the keyboard event occurred */
    if (ret == 0) {
        ret = screen_get_event_property_pv(qnx_screen_display->event, SCREEN_PROPERTY_WINDOW, (void **)&window_handle);
        if (ret == -1)  { 
            g_critical (G_STRLOC "failed to get key event window: %s", strerror(errno));
        }
    }

    /* get the key symbol */
    if (ret == 0) {
        ret = screen_get_event_property_iv(qnx_screen_display->event, SCREEN_PROPERTY_SYM, &sym);
        if (ret == -1) {
            g_critical (G_STRLOC "failed to get key event symbol: %s", strerror(errno));
        }
    }

    /* get the caps/shift property */
    if (ret == 0) {
        ret = screen_get_event_property_iv(qnx_screen_display->event, SCREEN_PROPERTY_KEY_CAP, &cap);
        if (ret == -1) {
            g_critical (G_STRLOC "failed to get key event cap: %s", strerror(errno));
        }
    }

    /* get the physical key position */
    if (ret == 0) {
        ret = screen_get_event_property_iv(qnx_screen_display->event, SCREEN_PROPERTY_SCAN, &scan);
        if (ret == -1) {
            g_critical (G_STRLOC "failed to get key event scan code: %s", strerror(errno));
        }
    }

    /* get the key property flags (key down, repeat, etc) */
    if (ret == 0) {
        ret = screen_get_event_property_iv(qnx_screen_display->event, SCREEN_PROPERTY_FLAGS, &flags);
        if (ret == -1) {
            g_critical (G_STRLOC "failed to get key event flags: %s", strerror(errno));
        }
    }

    /* get the key modifiers */
    if (ret == 0) {
        ret = screen_get_event_property_iv(qnx_screen_display->event, SCREEN_PROPERTY_MODIFIERS, &modifiers);
        if (ret == -1) {
            g_critical (G_STRLOC "failed to get key event modifiers: %s", strerror(errno));
        }
    }

    /* translate the native window to a GDKSurface */
    if (ret == 0) {
        surface = g_hash_table_lookup(qnx_screen_display->surface_window_table, window_handle);
    } 

    /* create event and deliver */
    GdkTranslatedKey translated;
    translated.keyval = sym;
    translated.consumed = 0;
    translated.layout = 0;
    translated.level = 0;
    event = gdk_key_event_new (
        flags & KEY_DOWN ? GDK_KEY_PRESS : GDK_KEY_RELEASE,
        surface,
        qnx_screen_display->core_keyboard,
        GDK_QNXSCREEN_TIME(),
        sym,
        0,  // todo need to convert modifiers to GdkModifierType type
        FALSE,
        &translated,
        &translated,
        NULL);

    GDK_DEBUG(
        EVENTS,
        "%s key %s sym %x cap %d scan %x flags %d mods %d",
        QNX_SCREEN,
        flags & KEY_DOWN ? "press" : "release",
        sym, cap, scan, flags, modifiers);

    gdk_qnxscreen_event_deliver_event (display, event);
}

void gdk_qnxscreen_device_query_state(GdkDevice* device, GdkSurface* surface, double* win_x, double* win_y, GdkModifierType* mask)
{
    int origin_x, origin_y;

    if (gdk_device_get_source (device) != GDK_SOURCE_MOUSE)
        return;

    GdkQnxScreenDevice* impl = GDK_QNXSCREEN_DEVICE(device);
    gdk_surface_get_origin (surface, &origin_x, &origin_y);

    if (win_x)
        *win_x = impl->root_pos[0] - origin_x;
    if (win_y)
        *win_y = impl->root_pos[1] - origin_y;
    if (mask)
        *mask = 0;

    GDK_DEBUG (
        MISC, 
        "%s %s: %f, %f",
        QNX_SCREEN,
        __func__,
        win_x ? *win_x : -1.0,
        win_y ? *win_y : -1.0);
}