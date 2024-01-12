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
#include "gdkmonitor-qnxscreen-data.h"
#include "gdk_qnxscreen_common.h"

G_DEFINE_TYPE (GdkQnxScreenMonitor, gdk_qnxscreen_monitor, GDK_TYPE_MONITOR)

static void gdk_qnxscreen_monitor_init (GdkQnxScreenMonitor* monitor)
{
    GDK_DEBUG(MISC, "%s initializing GdkQnxScreenMonitor", QNX_SCREEN);
    GdkQnxScreenMonitor* impl = GDK_QNXSCREEN_MONITOR(monitor);
    impl->display_handle = NULL;
    impl->display_id = 0;
}

static void gdk_qnxscreen_monitor_class_init (GdkQnxScreenMonitorClass* class)
{
    GDK_DEBUG(MISC, "%s initializing GdkQnxScreenMonitorClass", QNX_SCREEN);
}

int gdk_qnxscreen_monitor_init_from_qnxscreen(GdkMonitor* monitor, screen_display_t qnx_screen_display)
{
    GDK_DEBUG(MISC, "%s initializing GdkQnxScreenMonitor", QNX_SCREEN);

    int ret = 0;
    int display_id;
    int size[2] = {0};
    int pos[2] = {0};
    int refresh_rate = 0;

    /* Note: GDK Monitor == QNX Screen display */

    /* get the display ID */
    ret = screen_get_display_property_iv(qnx_screen_display, SCREEN_PROPERTY_ID, &display_id);
    if (ret == -1) {
        g_critical (G_STRLOC ": screen_get_display_property_iv(SCREEN_PROPERTY_ID): %s", strerror(errno));
    }

    /* get the size of the display */
    ret = screen_get_display_property_iv(qnx_screen_display, SCREEN_PROPERTY_SIZE, size);
    if (ret == -1) {
        g_critical (G_STRLOC ": screen_get_display_property_iv(SCREEN_PROPERTY_SIZE): %s", strerror(errno));
    }

    /* TODO: QNX Screen versions <= 3.x.x do not have position-awareness for displays (monitors).
     *  This feature may become available in future QNX Screen releases.
    */
    /* get the position of this display */
    ret = screen_get_display_property_iv(qnx_screen_display, SCREEN_PROPERTY_POSITION, pos);
    if (ret) {
        g_message ("QNX Screen does not support SCREEN_PROPERTY_POSITION for display (monitor). Setting monitor position to (0,0).");
        pos[0] = 0;
        pos[1] = 0;
    }

    /* get the mode of this display (for refresh rate) */
    screen_display_mode_t display_mode;
    ret = screen_get_display_property_pv(qnx_screen_display, SCREEN_PROPERTY_MODE, (void**)&display_mode);
    if (ret == -1) {
        g_critical (G_STRLOC ": screen_get_display_property_iv(SCREEN_PROPERTY_MODE): %s", strerror(errno));
    }
    refresh_rate = display_mode.refresh;

    if (!refresh_rate) {
        refresh_rate = 60;
    }

    /* set GdkMonitor properties */
    if (ret == 0) {
        GdkQnxScreenMonitor* impl = GDK_QNXSCREEN_MONITOR(monitor);
        impl->display_handle = qnx_screen_display;
        impl->display_id = display_id;
        gdk_monitor_set_geometry(monitor, &(GdkRectangle) { pos[0], pos[1], size[0], size[1] });
        /* Refresh rate set in milli-Hz */
        gdk_monitor_set_refresh_rate(monitor, refresh_rate*1000);
        /* Scale factor not supported in all qnxscreen modules */
        gdk_monitor_set_scale_factor(monitor, 1);

        GDK_DEBUG(MISC, "%s Initialized QNX Screen Display (GdkMonitor)- ID: %d, size: %dx%d, position: %dx%x, refresh: %d Hz",
            QNX_SCREEN, display_id, size[0], size[1], pos[0], pos[1], refresh_rate);
    }

    return ret;
}
