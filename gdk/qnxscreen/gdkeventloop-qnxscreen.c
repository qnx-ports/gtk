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

#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include "config.h"
#include "glib.h"
#include <screen/screen.h>
#include "gdk/gdk.h"
#include "gdk/gdkeventsprivate.h"
#include "gdkdisplay-qnxscreen-data.h"
#include "gdkdevice-qnxscreen-class.h"
#include "gdkeventloop-qnxscreen.h"
#include "gdk_qnxscreen_common.h"

static int event_chid;
static int event_coid;
static GPollFD event_poll_fd;
static unsigned events_pending;
static pthread_mutex_t event_mutex = PTHREAD_MUTEX_INITIALIZER;

static inline void lock_events (void)
{
    if (pthread_mutex_lock (&event_mutex) != 0)
    {
        g_critical (G_STRLOC ": failed to lock event mutex: %s", strerror(errno));
    }
}

static inline void unlock_events (void)
{
    pthread_mutex_unlock (&event_mutex);
}

static void gdk_qnxscreen_event_queue_events(GdkDisplay* display)
{
    GDK_DEBUG(EVENTS, "%s reading QNX Screen events", QNX_SCREEN);

    GdkQnxScreenDisplay* qnx_screen_display = GDK_QNXSCREEN_DISPLAY(display);
    gboolean available_events = TRUE;
    int type = 0;
    int subtype = 0;

    do
    {
        if (screen_get_event(qnx_screen_display->qnxscreen_context, qnx_screen_display->event, 0) == -1) {
            g_critical (G_STRLOC ": failed to get screen event: %s", strerror(errno));
        }

        if (screen_get_event_property_iv(qnx_screen_display->event, SCREEN_PROPERTY_TYPE, &type) == -1) {
            g_critical (G_STRLOC ": failed to get event type: %s", strerror(errno));
        }

        switch (type)
        {
            case SCREEN_EVENT_POINTER:
                gdk_qnxscreen_device_pointer_event(display);
                break;

            case SCREEN_EVENT_KEYBOARD:
                gdk_qnxscreen_device_keyboard_event(display);
                break;

            case SCREEN_EVENT_MTOUCH_TOUCH:
            case SCREEN_EVENT_MTOUCH_MOVE:
            case SCREEN_EVENT_MTOUCH_RELEASE:
                gdk_qnxscreen_device_touch_event(display, type);
                break;

            case SCREEN_EVENT_MANAGER:                
                gdk_qnxscreen_display_management_event(display);
                break;

            case SCREEN_EVENT_INPUT_CONTROL:
                if (-1 == screen_get_event_property_iv(qnx_screen_display->event, SCREEN_PROPERTY_NAME, &subtype)) {
                    g_critical (G_STRLOC ": screen_get_event_property_iv(SCREEN_PROPERTY_SUBTYPE): %s", strerror(errno));
                } else {
                    gdk_qnxscreen_inputcontrol_event(display, subtype);
                }
                break;

            case SCREEN_EVENT_NONE:
                available_events = FALSE;
                GDK_DEBUG(EVENTS, "%s SCREEN_EVENT_NONE detected - available_events=FALSE", QNX_SCREEN);
                break;

            /* Silently ignore SCREEN_EVENT_PROPERTY */
            case SCREEN_EVENT_PROPERTY:
                break;
            default:
                GDK_DEBUG(EVENTS, "%s unhandled event type: %d", QNX_SCREEN, type);
                break;
        }
    } while (available_events == TRUE);
}

void gdk_qnxscreen_event_deliver_event(GdkDisplay* display, GdkEvent* event)
{
    GDK_DEBUG (EVENTS, "%s queue event %p type %d surface %p", QNX_SCREEN, event, event->event_type, event->surface);

    GList* node = _gdk_event_queue_append (display, event);
    _gdk_windowing_got_event (display, node, event, _gdk_display_get_next_serial(display));
}

static gboolean gdk_qnxscreen_event_check(GSource* source)
{
    GdkDisplay* display = ((GdkQnxScreenEventSource*) source)->display;
    gboolean retval = FALSE;

    if ((display->event_pause_count > 0) || (event_poll_fd.revents & G_IO_IN)) {
        retval = (_gdk_event_queue_find_first (display) != NULL);
    } else {
        retval = FALSE;
    }

    return retval;
}

static gboolean gdk_qnxscreen_event_dispatch (GSource* source, GSourceFunc callback, gpointer user_data)
{
    GdkDisplay* display = ((GdkQnxScreenEventSource*)source)->display;
    GdkEvent* event = NULL;

    // convert ALL pending QNX Screen events to GDKEvent events and to be added to queue _gdk_event_queue_append (display, event);
    gdk_qnxscreen_event_queue_events(display); 

    for (;;)
    {
        event = _gdk_event_unqueue (display);
        if (event == NULL) {
            break;
        }

        GDK_DEBUG (EVENTS, "%s dequeue event %p type %d surface %p", QNX_SCREEN, event, event->event_type, event->surface);

        _gdk_event_emit (event);
        gdk_event_unref (event);
    }

    return TRUE;
}

static gboolean gdk_qnxscreen_event_prepare (GSource* source, gint* timeout)
{
    gboolean has_events = FALSE;

    lock_events ();

    if (events_pending != 0) {
        events_pending--;
        has_events = TRUE;
    } else {
        has_events = FALSE;
    }

    unlock_events ();

    return has_events;
}

static GSourceFuncs event_funcs = {
    gdk_qnxscreen_event_prepare,
    gdk_qnxscreen_event_check,
    gdk_qnxscreen_event_dispatch,
    NULL
};

static void* screen_event_thread(void* arg)
{
    GDK_DEBUG(MISC, "%s event thread created", QNX_SCREEN);

    union {
        struct _pulse pulse;
        io_notify_t notify;
    } msg;

    int rcvid = 0;
    int notify_rcvid = -1;
    struct sigevent notify_event;

    for (;;)
    {
        rcvid = MsgReceive (event_chid, &msg, sizeof (msg), NULL);
        if (rcvid == -1) {
            g_critical (G_STRLOC ": failed to receive message: %s", strerror(errno));
        } else if (rcvid == 0) {
            if (msg.pulse.code == _PULSE_CODE_MINAVAIL) {
                /*
                * Screen event ready.
                */
                lock_events ();
                events_pending++;
                unlock_events ();

                if (notify_rcvid != -1) {
                    if (MsgDeliverEvent (notify_rcvid, &notify_event) == -1) {
                        g_critical (G_STRLOC ": failed to deliver event: %s",strerror(errno));
                    }
                }
            }
        } else if (msg.notify.i.type == _IO_NOTIFY) {
            /*
            * Query from poll().
            */
            lock_events ();
            if (events_pending > 0) {
                /*
                * Pending events, let poll() know.
                */
                notify_rcvid = -1;
                msg.notify.o.flags = _NOTIFY_COND_INPUT;
            } else {
                /*
                * No pending events.
                * Remember the notification information so it can be
                * delivered later.
                */
                notify_rcvid = rcvid;
                notify_event = *(struct sigevent *)&msg.notify.i.event;
                msg.notify.o.flags = 0;
            }

            unlock_events ();
            MsgReply(rcvid, 0, &msg.notify.o, sizeof (msg.notify.o));
        } else {
            MsgError(rcvid, ENOSYS);
        }
    }

    return NULL;
}

int gdk_qnxscreen_event_loop_init(GdkDisplay* display, screen_context_t context)
{
    GDK_DEBUG(MISC, "%s initializing event handler loop", QNX_SCREEN);

    int rc = 0;
    struct sigevent screen_event;
    GSource* source = NULL;
    GdkQnxScreenEventSource* event_source = NULL;
    GdkQnxScreenDisplay* qnx_screen_display = GDK_QNXSCREEN_DISPLAY(display);

    /* initialize the screen event data structure */
    screen_create_event(&qnx_screen_display->event);

    /* create a private channel for event notifications */
    event_chid = ChannelCreate(_NTO_CHF_PRIVATE);
    if (event_chid == -1) {
        g_critical (G_STRLOC ": failed to create event channel: %s", strerror(errno));
    }

    /* connect to the channel */
    event_coid = ConnectAttach(0, 0, event_chid, 0, 0);
    if (event_coid == -1) {
        g_critical (G_STRLOC ": failed to connect to event channel: %s", strerror(errno));
    }

    /* register for screen notifications */
    SIGEV_PULSE_INIT(&screen_event, event_coid, -1, _PULSE_CODE_MINAVAIL, 0);
    if (screen_register_event (context, &screen_event) == -1) {
        g_critical (G_STRLOC ": failed to register screen event: %s", strerror(errno));
    }

    if (screen_notify (context, SCREEN_NOTIFY_INPUT, NULL, &screen_event) == -1) {
        g_critical (G_STRLOC ": failed to register screen event: %s", strerror(errno));
    }

    /* add a fake screen fd to the list of fds to poll */
    event_poll_fd.events = G_IO_IN;
    event_poll_fd.fd = event_coid;

    /* create the event source */
    source = g_source_new (&event_funcs, sizeof (GdkQnxScreenEventSource));
    g_source_set_name (source, "GDK QNX Screen event source");
    g_source_add_poll (source, &event_poll_fd);
    g_source_set_priority (source, GDK_PRIORITY_EVENTS);
    g_source_set_can_recurse (source, TRUE);
    g_source_attach (source, NULL);
    event_source = (GdkQnxScreenEventSource *)source;
    event_source->display = display;

    /* start polling thread */
    rc = pthread_create (NULL, NULL, screen_event_thread, NULL);
    if (rc != 0) {
        g_critical (G_STRLOC ": failed to create polling thread: %s", strerror(rc));
    }

    return 0;
}
