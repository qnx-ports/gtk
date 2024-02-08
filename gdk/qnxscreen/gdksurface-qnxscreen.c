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
#include "errno.h"
#include "gdksurface-qnxscreen-data.h"
#include "gdkdisplay-qnxscreen-data.h"
#include "gdkmonitor-qnxscreen-data.h"
#include "gdkdevice-qnxscreen-class.h"
#include "gdk_qnxscreen_common.h"

G_DEFINE_TYPE (GdkQnxScreenSurface, gdk_qnxscreen_surface, GDK_TYPE_SURFACE)

void gdk_qnxscreen_surface_post_screen(GdkSurface* surface, int buffer_idx, int num_rects, cairo_rectangle_int_t* damaged_rects)
{
    GdkQnxScreenSurface* qnx_screen_surface = GDK_QNXSCREEN_SURFACE(surface);
    if (screen_post_window (qnx_screen_surface->window_handle, qnx_screen_surface->buffer_handles[buffer_idx], num_rects, (int*)damaged_rects, 0) == -1)
    {
        g_critical (G_STRLOC ": failed to post window: %s", strerror(errno));
    }
}

static gboolean gdk_qnxscreen_surface_get_device_state(GdkSurface* surface, GdkDevice* device, double* x, double* y, GdkModifierType* mask)
{
    GDK_DEBUG(MISC, "%s surface get device state: surface %p - device %p", QNX_SCREEN, surface, device);
    
    g_return_val_if_fail (surface == NULL || GDK_IS_SURFACE (surface), FALSE);

    if (GDK_SURFACE_DESTROYED (surface))
        return FALSE;

    gdk_qnxscreen_device_query_state(device, surface, x, y, mask);

    gboolean result = *x >= 0 && *y >= 0 && *x < surface->width && *y < surface->height;
    GDK_DEBUG(MISC, "%s surface get device state result: %d", QNX_SCREEN, result);

    return result;
}

static void gdk_qnxscreen_surface_get_root_coords(GdkSurface* surface, gint x, gint y, gint* root_x, gint* root_y)
{
    int out_x = 0;
    int out_y = 0;

    for (; surface; surface = surface->parent)
    {
        out_x += surface->x;
        out_y += surface->y;
    }

    out_x += x;
    out_y += y;

    if (root_x)
        *root_x = out_x;

    if (root_y)
        *root_y = out_y;

    GDK_DEBUG (MISC, "%s %s: %d,%d => %d,%d", QNX_SCREEN, __func__, x, y, out_x, out_y);
}

static void gdk_qnxscreen_surface_finalize (GObject* object)
{
    GDK_DEBUG(MISC, "%s finalize GdkQnxScreenSurface: %p", QNX_SCREEN, object);

    GdkSurface *surface = GDK_SURFACE(object);
    GdkQnxScreenSurface* impl = GDK_QNXSCREEN_SURFACE(surface);
    GdkDisplay* display = gdk_surface_get_display(surface);
    GdkQnxScreenDisplay* display_impl = GDK_QNXSCREEN_DISPLAY(display);
    
    // Notify the display that this surface is about to be destroyed
    gdk_qnxscreen_display_surface_destroyed(display, surface);

    if (impl->session_handle != NULL) {
        screen_destroy_session(impl->session_handle);
        impl->session_handle = NULL;
    }

    for (int i = 0; i < QNXSCREEN_NUM_BUFFERS; i++) {
        if (impl->cairo_surfaces[i] != NULL) {
            cairo_surface_destroy(impl->cairo_surfaces[i]);
            impl->cairo_surfaces[i] = NULL;
        }
    }

    if (impl->window_handle != NULL) {
        screen_destroy_window_buffers(impl->window_handle);
        screen_destroy_window(impl->window_handle);
        impl->window_handle = NULL;
        
        for (int i = 0; i < QNXSCREEN_NUM_BUFFERS; i++) {
            impl->buffer_handles[i] = NULL;
        }
    }

    unsigned long long qval = impl->zorder;
    if (qval != 0) { // TODO: This is INCORRECT because 0 can be a valid, non-default z-order. We should NOT track z-orders using a queue.
        if (GDK_IS_TOPLEVEL(surface)) {
            /* GdkDisplay zorder queue should have 1:1 z-order allocation for all windows */
            g_warn_if_fail(g_queue_remove(display_impl->zorders, (void*)qval));
        } else if (surface->parent) {
            GdkQnxScreenSurface *parent = GDK_QNXSCREEN_SURFACE(surface->parent);
            /* parent's GdkSurface zorder queue should have 1:1 z-order allocation for all windows */
            g_warn_if_fail(g_queue_remove(parent->zorders, (void*)qval));
        }
    }

    impl->zorder = 0;

    if (impl->zorders) {
        g_queue_free(impl->zorders);
        impl->zorders = NULL;
    }
    if (impl->active_touch_table) {
        g_hash_table_destroy(impl->active_touch_table);
    }
    if (impl->refocussed_touch_table) {
        g_hash_table_destroy(impl->refocussed_touch_table);
    }
    G_OBJECT_CLASS (gdk_qnxscreen_surface_parent_class)->dispose(object);
}

static void gdk_qnxscreen_surface_set_input_region(GdkSurface* surface, cairo_region_t* shape_region)
{
    GDK_DEBUG(MISC, "%s surface set input region: %p", QNX_SCREEN, surface);
}

static void
gdk_qnxscreen_surface_show (GdkSurface *surface)
{
    GdkQnxScreenSurface *impl = GDK_QNXSCREEN_SURFACE(surface);

    GDK_DEBUG(MISC, "%s showing GdkQnxScreenSurface: %p", QNX_SCREEN, impl);

    impl->visible = TRUE;
    int ret = screen_set_window_property_iv(impl->window_handle, SCREEN_PROPERTY_VISIBLE, &impl->visible);
    if (ret) {
        g_critical (G_STRLOC ": screen_set_window_property_iv(SCREEN_PROPERTY_VISIBLE)");
    } else {
        ret = screen_flush_context(impl->context_handle, 0);
        if (ret) {
            g_critical (G_STRLOC ": failed to SCREEN_PROPERTY_VISIBLE: %s", strerror(errno));
        } else {
            GDK_DEBUG(MISC, "%s successfully set SCREEN_PROPERTY_VISIBLE=TRUE", QNX_SCREEN);
        }
    }
}

static void gdk_qnxscreen_surface_hide(GdkSurface *surface)
{
    GdkQnxScreenSurface *impl = GDK_QNXSCREEN_SURFACE(surface);
    GDK_DEBUG(MISC, "%s hiding GdkQnxScreenSurface: %p", QNX_SCREEN, impl);

    impl->visible = FALSE;
    int ret = screen_set_window_property_iv(impl->window_handle, SCREEN_PROPERTY_VISIBLE, &impl->visible);
    if (ret) {
        g_critical (G_STRLOC ": screen_set_window_property_iv(SCREEN_PROPERTY_VISIBLE)");
    } else {
        ret = screen_flush_context(impl->context_handle, 0);
        if (ret) {
            g_critical (G_STRLOC ": failed to SCREEN_PROPERTY_VISIBLE: %s", strerror(errno));
        } else {
            GDK_DEBUG(MISC, "%s successfully set SCREEN_PROPERTY_VISIBLE=FALSE", QNX_SCREEN);
        }
    }

    _gdk_surface_clear_update_area (surface);
}

static void gdk_qnxscreen_surface_move_to_monitor (GdkSurface *surface, GdkMonitor *monitor)
{
    GdkQnxScreenSurface *impl = GDK_QNXSCREEN_SURFACE(surface);
    GdkQnxScreenMonitor *monitor_impl = GDK_QNXSCREEN_MONITOR(monitor);

    if (GDK_SURFACE_DESTROYED (surface))
        return;

    /* If surface is already mapped to a monitor */
    if (impl->mapped_monitor == monitor)
        return;

    if (NULL == monitor_impl->display_handle)
        g_critical (G_STRLOC ": GdkQnxScreenMonitor does not have valid QnxScreen display handle");

    /* Put this window on the Screen display (GdkMonitor) */
    int ret = screen_set_window_property_pv(impl->window_handle, SCREEN_PROPERTY_DISPLAY, (void**)&monitor_impl->display_handle);
    if (ret) {
        g_critical (G_STRLOC ": screen_set_window_property_iv(SCREEN_PROPERTY_DISPLAY): %s", strerror(errno));
    } else {
        ret = screen_flush_context(impl->context_handle, 0);
        if (ret) {
            g_critical (G_STRLOC ": Could not set SCREEN_PROPERTY_DISPLAY for window: %s", strerror(errno));
        } else {
            GDK_DEBUG(MISC, "%s successfully moved window to QnxScreen display %d", QNX_SCREEN, monitor_impl->display_id);
        }
    }

    /* For top-level surfaces, the surface is considered mapped once it is placed on a monitor */
    gdk_surface_set_is_mapped (surface, TRUE);
    impl->mapped_monitor = monitor;
}

static void
gdk_qnxscreen_surface_move_resize(GdkSurface *surface,
                               gboolean with_move,
                               int x,
                               int y,
                               int width,
                               int height)
{
    GdkQnxScreenSurface *impl = GDK_QNXSCREEN_SURFACE (surface);
    int ret = 0;

    if (with_move) {
        /* set window position */
        int pos[] = { x, y };
        ret = screen_set_window_property_iv(impl->window_handle, SCREEN_PROPERTY_POSITION, pos);
        if (ret == -1) {
            g_critical (G_STRLOC ": screen_set_window_property_iv(SCREEN_PROPERTY_POSITION): %s", strerror(errno));
        } else {
            surface->x = x;
            surface->y = y;
            impl->win_x = x;
            impl->win_y = y;
            GDK_DEBUG(MISC, "%s window position set to: (%d,%d)", QNX_SCREEN, x, y);
        }
    }

    if (width >= 0 && height >= 0) {
        /* Make non-zero */
        width = (width) ? width : 1;
        height = (height) ? height : 1;
        /* Set the window size */
        int new_size[] = { width, height };
        ret = screen_set_window_property_iv(impl->window_handle, SCREEN_PROPERTY_SIZE, new_size);
        if (ret == -1) {
            g_critical (G_STRLOC ": screen_set_window_property_iv(SCREEN_PROPERTY_SIZE): %s", strerror(errno));
        } else {
            /* Note: Need to also set source size because the window buffer (and cairo surface backed by
             * this buffer) is persistent. Want to resize the window, but also resize the source size that
             * window presents from */
            /* Set the source size (window buffer source size) */
            ret = screen_set_window_property_iv(impl->window_handle, SCREEN_PROPERTY_SOURCE_SIZE, new_size);
            if (ret == -1) {
                g_critical (G_STRLOC ": screen_set_window_property_iv(SCREEN_PROPERTY_SOURCE_SIZE): %s", strerror(errno));
            } else {
                impl->win_width = width;
                impl->win_height = height;
                surface->width = width;
                surface->height = height;
                GDK_DEBUG(MISC, "%s qnxscreen surface resized to: %dx%d", QNX_SCREEN, width, height);
                _gdk_surface_update_size (surface);
            }
        }
    }

    ret = screen_flush_context(impl->context_handle, 0);
    if (0 != ret) {
        g_critical (G_STRLOC ": screen_flush_context(): %s", strerror(errno));
    }
}

static void
gdk_qnxscreen_surface_fullscreen (GdkSurface *surface)
{
    if (GDK_SURFACE_DESTROYED (surface))
        return;

    GdkQnxScreenSurface *impl = GDK_QNXSCREEN_SURFACE (surface);

    if (!impl->fullscreened) {
        impl->pre_fullscreen_x = surface->x;
        impl->pre_fullscreen_y = surface->y;
        impl->pre_fullscreen_width = surface->width;
        impl->pre_fullscreen_height = surface->height;
    }

    GdkMonitor *monitor = impl->mapped_monitor;

    if (NULL == monitor) {
        monitor = gdk_display_get_monitor_at_surface(gdk_surface_get_display(surface), surface);
        gdk_qnxscreen_surface_move_to_monitor(surface, monitor);
    }

    impl->fullscreened = TRUE;

    GdkRectangle geom;
    gdk_monitor_get_geometry (monitor, &geom);
    /* Resize/move to be fullscreen */
    gdk_qnxscreen_surface_move_resize (surface, TRUE, geom.x, geom.y, geom.width, geom.height);

    GDK_DEBUG(MISC, "%s surface window set to fullscreen", QNX_SCREEN);
}

static void
gdk_qnxscreen_surface_fullscreen_on_monitor (GdkSurface  *surface,
                                      GdkMonitor *monitor)
{
    if (GDK_SURFACE_DESTROYED (surface))
        return;

    gdk_qnxscreen_surface_move_to_monitor(surface, monitor);

    surface->fullscreen_mode = GDK_FULLSCREEN_ON_CURRENT_MONITOR;
    g_object_notify (G_OBJECT (surface), "fullscreen-mode");

    gdk_qnxscreen_surface_fullscreen (surface);
}

static void
gdk_qnxscreen_surface_unfullscreen (GdkSurface *surface)
{
    if (GDK_SURFACE_DESTROYED (surface))
        return;

    GdkQnxScreenSurface *impl = GDK_QNXSCREEN_SURFACE (surface);

    if (!impl->fullscreened)
        return;

    impl->fullscreened = FALSE;

    /* Resize/move back to original state */
    gdk_qnxscreen_surface_move_resize (surface,
        TRUE,
        impl->pre_fullscreen_x,
        impl->pre_fullscreen_y,
        impl->pre_fullscreen_width,
        impl->pre_fullscreen_height);
}

static void
gdk_qnxscreen_surface_layout_popup (GdkSurface     *surface,
                                   int             width,
                                   int             height,
                                   GdkPopupLayout *layout)
{
    GdkMonitor *monitor;
    GdkRectangle bounds;
    GdkRectangle final_rect;

    monitor = gdk_surface_get_layout_monitor (surface, layout,
                                                gdk_monitor_get_geometry);
    gdk_monitor_get_geometry (monitor, &bounds);

    gdk_surface_layout_popup_helper (surface,
                                        width,
                                        height,
                                        0,
                                        0,
                                        0,
                                        0,
                                        monitor,
                                        &bounds,
                                        layout,
                                        &final_rect);

    gdk_qnxscreen_surface_move_resize (surface,
                                        TRUE,
                                        final_rect.x,
                                        final_rect.y,
                                        final_rect.width,
                                        final_rect.height);
}

static void gdk_qnxscreen_surface_destroy(GdkSurface* surface, gboolean foreign_destroy)
{
    GDK_DEBUG(MISC, "%s destroy surface: %p", QNX_SCREEN, surface);

    g_return_if_fail (GDK_IS_SURFACE (surface));

    GdkDisplay* display = gdk_surface_get_display(surface);
    GdkQnxScreenDisplay* qnx_screen_display = GDK_QNXSCREEN_DISPLAY(display);
    GdkQnxScreenSurface* qnx_screen_surface = GDK_QNXSCREEN_SURFACE(surface);

    g_hash_table_remove (qnx_screen_display->surface_window_table, GINT_TO_POINTER (qnx_screen_surface->window_handle));
}

static void gdk_qnxscreen_surface_init(GdkQnxScreenSurface* surface)
{
    GDK_DEBUG(MISC, "%s initializing GdkQnxScreenSurface: %p", QNX_SCREEN, surface);

    surface->window_handle = NULL;
    surface->session_handle = NULL;
    surface->visible = FALSE;
    surface->win_width = surface->win_height = 0;
    surface->buf_width = surface->buf_height = 0;
    surface->zorder = 0;

    surface->zorders = g_queue_new();
    surface->active_touch_table = g_hash_table_new(NULL, NULL);
    surface->refocussed_touch_table = g_hash_table_new(NULL, NULL);
    
    for (int i = 0; i < QNXSCREEN_NUM_BUFFERS; i++) {
        surface->buffer_handles[i] = NULL;
        surface->cairo_surfaces[i] = NULL;
        surface->buf_ptrs[i] = NULL;
    }
}

static void gdk_qnxscreen_surface_class_init(GdkQnxScreenSurfaceClass* class)
{
    GDK_DEBUG(MISC, "%s initializing GdkQnxScreenSurfaceClass: %p", QNX_SCREEN, class);

    GObjectClass* object_class = G_OBJECT_CLASS(class);
    GdkSurfaceClass* surface_class = GDK_SURFACE_CLASS(class);

    object_class->finalize = gdk_qnxscreen_surface_finalize;

    surface_class->get_device_state = gdk_qnxscreen_surface_get_device_state;
    surface_class->get_root_coords = gdk_qnxscreen_surface_get_root_coords;
    surface_class->set_input_region = gdk_qnxscreen_surface_set_input_region;
    surface_class->hide = gdk_qnxscreen_surface_hide;
    surface_class->destroy = gdk_qnxscreen_surface_destroy;
}

static void gdk_qnxscreen_surface_create_group (GdkSurface* surface)
{
    GdkQnxScreenSurface *impl = GDK_QNXSCREEN_SURFACE(surface);

    if (NULL == impl->group_handle) {
        int ret = screen_create_window_group(impl->window_handle, NULL);
        if (ret) {
            g_critical (G_STRLOC ": screen_create_window_group(window, NULL): %s", strerror(errno));
        } else {
            ret = screen_get_window_property_pv(impl->window_handle, SCREEN_PROPERTY_GROUP, (void**)&impl->group_handle);
            if (ret) {
                g_critical (G_STRLOC ": screen_get_window_property_pv(SCREEN_PROPERTY_GROUP): %s", strerror(errno));
            } else {
                ret = screen_get_window_property_cv(impl->window_handle, SCREEN_PROPERTY_GROUP, sizeof(impl->group_name), impl->group_name);
                if (ret) {
                    g_critical (G_STRLOC ": screen_get_window_property_cv(SCREEN_PROPERTY_GROUP): %s", strerror(errno));
                } else {
                    GDK_DEBUG(MISC, "%s Create Window group for toplevel: %s", QNX_SCREEN, impl->group_name);
                }
            }
        }
    }
}

static void gdk_qnxscreen_create_window (GdkQnxScreenSurface* qnx_screen_surface, int qnxscreen_window_type)
{
    GdkSurface* surface = GDK_SURFACE(qnx_screen_surface);
    GdkDisplay *display = gdk_surface_get_display(surface);
    GdkQnxScreenDisplay* qnx_screen_display = GDK_QNXSCREEN_DISPLAY(display);
    const int usage = SCREEN_USAGE_WRITE | SCREEN_USAGE_NATIVE;
    const int format = SCREEN_FORMAT_RGBA8888;
    int ret = 0;
    
    qnx_screen_surface->context_handle = qnx_screen_display->qnxscreen_context;
    qnx_screen_surface->mapped_monitor = NULL;
    
    /* create qnx screen window */
    ret = screen_create_window_type(&qnx_screen_surface->window_handle, qnx_screen_surface->context_handle, qnxscreen_window_type);
    if (ret < 0) {
        g_critical (G_STRLOC ": failed to create screen window: %s", strerror(errno));
        return;
    }
    
    /* Get the window's size */
    int win_size[2];
    ret = screen_get_window_property_iv(qnx_screen_surface->window_handle, SCREEN_PROPERTY_SIZE, win_size);
    if (ret < 0) {
        g_critical (G_STRLOC ": screen_get_window_property_iv(SCREEN_PROPERTY_SIZE): %s", strerror(errno));
        return;
    }

    qnx_screen_surface->win_width = win_size[0];
    qnx_screen_surface->win_height = win_size[1];
    GDK_DEBUG(MISC, "%s successfully created screen window, size: %dx%d (default)",
        QNX_SCREEN, qnx_screen_surface->win_width, qnx_screen_surface->win_height);

    /* Get the window's position */
    int pos[2];
    ret = screen_get_window_property_iv(qnx_screen_surface->window_handle, SCREEN_PROPERTY_POSITION, pos);
    if (ret < 0) {
        g_critical (G_STRLOC ": screen_get_window_property_iv(SCREEN_PROPERTY_SIZE): %s", strerror(errno));
        return;
    }

    qnx_screen_surface->win_x = pos[0];
    qnx_screen_surface->win_y = pos[1];
    GDK_DEBUG(MISC, "%s window position: %d,%d (default)",
        QNX_SCREEN, qnx_screen_surface->win_width, qnx_screen_surface->win_height);

    /* set screen usage */
    ret = screen_set_window_property_iv (qnx_screen_surface->window_handle, SCREEN_PROPERTY_USAGE, &usage);
    if (ret < 0) {
        g_critical (G_STRLOC ": failed to set window usage property: %s", strerror(errno));
        return;
    }

    GDK_DEBUG(MISC, "%s successfully set screen usage to 0x%04X", QNX_SCREEN, SCREEN_PROPERTY_USAGE);

    /* set screen format */
    ret = screen_set_window_property_iv(qnx_screen_surface->window_handle, SCREEN_PROPERTY_FORMAT, &format);
    if (ret < 0) {
        g_critical (G_STRLOC ": failed to set window format property: %s", strerror(errno));
        return;
    }

    GDK_DEBUG(MISC, "%s successfully set screen format to 0x%04X", QNX_SCREEN, SCREEN_PROPERTY_FORMAT);

    /* create the window buffer */
    GDK_DEBUG(MISC, "%s creating %d window buffers (default size)", QNX_SCREEN, QNXSCREEN_NUM_BUFFERS);
    ret = screen_create_window_buffers(qnx_screen_surface->window_handle, QNXSCREEN_NUM_BUFFERS);
    if (ret < 0) {
        g_critical (G_STRLOC ": failed to create window buffers: %s", strerror(errno));
    }

    GDK_DEBUG(MISC, "%s successfully created screen buffer", QNX_SCREEN);

    /* get screen buffer */
    ret = screen_get_window_property_pv(qnx_screen_surface->window_handle, SCREEN_PROPERTY_BUFFERS, (void **) qnx_screen_surface->buffer_handles);
    if (ret < 0) {
        g_critical (G_STRLOC ": failed to get window buffer: %s", strerror(errno));
        return;
    }

    GDK_DEBUG(MISC, "%s successfully retrievd the screen buffer", QNX_SCREEN);

    /* Get the buffer size */
    int buf_size[2];
    ret = screen_get_buffer_property_iv(qnx_screen_surface->buffer_handles[0], SCREEN_PROPERTY_BUFFER_SIZE, buf_size);
    if (ret < 0) {
        g_critical (G_STRLOC ": screen_get_buffer_property_iv(SCREEN_PROPERTY_BUFFER_SIZE): %s", strerror(errno));
        return;
    }

    /* Note: The window buffers were created with the default size (in QNX Screen).
     *  In most cases this will be the size of the displays framebuffer. The buffer
     *  size does not have to match the window size. Resizing the GDK surface can
     *  still be done after initializing with window size, while the buffer size
     *  remains constant.
     */
    qnx_screen_surface->buf_width = buf_size[0];
    qnx_screen_surface->buf_height = buf_size[1];
    GDK_DEBUG(MISC, "%s successfully created window buffers, size: %dx%d (default)",
        QNX_SCREEN, qnx_screen_surface->buf_width, qnx_screen_surface->buf_height);

    /* get buffer stride */
    ret = screen_get_buffer_property_iv(qnx_screen_surface->buffer_handles[0], SCREEN_PROPERTY_STRIDE, &qnx_screen_surface->buf_stride);
    if (ret < 0) {
        g_critical (G_STRLOC ": failed to get buffer stride: %s", strerror(errno));
        return;
    }

    GDK_DEBUG(MISC, "%s buffer stride: %d", QNX_SCREEN, qnx_screen_surface->buf_stride);

    /* get pointer to buffer */
    for (int i = 0; i < QNXSCREEN_NUM_BUFFERS; i++) {
        ret = screen_get_buffer_property_pv(qnx_screen_surface->buffer_handles[i], SCREEN_PROPERTY_POINTER, &qnx_screen_surface->buf_ptrs[i]);
        if (ret < 0) {
            g_critical (G_STRLOC ": failed to get buffer pointer: %s", strerror(errno));
            return;
        }

        GDK_DEBUG(MISC, "%s successfully retrievd the screen buffer pointer %d", QNX_SCREEN, i);

        /* create the cairo surface */
        qnx_screen_surface->cairo_surfaces[i] = cairo_image_surface_create_for_data(qnx_screen_surface->buf_ptrs[i],
            CAIRO_FORMAT_ARGB32, qnx_screen_surface->buf_width, qnx_screen_surface->buf_height, qnx_screen_surface->buf_stride);
        if (qnx_screen_surface->cairo_surfaces[i] == NULL) {
            g_critical (G_STRLOC ": failed to create cairo surface: %s", strerror(errno));
            return;
        }

        GDK_DEBUG(MISC, "%s created cairo surface %d", QNX_SCREEN, i);
    }

    /* create a screen session */
    ret = screen_create_session_type(&qnx_screen_surface->session_handle, qnx_screen_display->qnxscreen_context, SCREEN_EVENT_POINTER);
    if (ret < 0) {
        g_critical (G_STRLOC ": failed to create window session: %s", strerror(errno));
        return;
    }

    GDK_DEBUG(MISC, "%s created screen session", QNX_SCREEN);

    /* associate session with window */
    ret = screen_set_session_property_pv(qnx_screen_surface->session_handle, SCREEN_PROPERTY_WINDOW, (void **)&qnx_screen_surface->window_handle);
    if (ret < 0) {
        g_critical (G_STRLOC ": failed to associate session with window: %s", strerror(errno));
        return;
    }

    GDK_DEBUG(MISC, "%s associated screen session with window", QNX_SCREEN);

    /* add lookup between QNX window_handle and GDKSurface */
    g_hash_table_insert(qnx_screen_display->surface_window_table, qnx_screen_surface->window_handle, surface);
}

G_DEFINE_TYPE_WITH_CODE (GdkQnxScreenToplevel, gdk_qnxscreen_toplevel, GDK_TYPE_QNXSCREEN_SURFACE,
                         G_IMPLEMENT_INTERFACE (GDK_TYPE_TOPLEVEL, gdk_qnxscreen_toplevel_iface_init))

static void gdk_qnxscreen_toplevel_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec)
{
    GDK_DEBUG(SETTINGS, "%s toplevel set property %d", QNX_SCREEN, prop_id);
}

static void gdk_qnxscreen_toplevel_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec)
{
    GDK_DEBUG(SETTINGS, "%s toplevel get property %d", QNX_SCREEN, prop_id);    
}

static void gdk_qnxscreen_surface_raise (GdkSurface *surface)
{
    GQueue *zorders = NULL;
    if (GDK_IS_TOPLEVEL(surface)) {
        /* Toplevel zorders are tracked by the GdkDisplay instance */
        zorders = GDK_QNXSCREEN_DISPLAY(gdk_surface_get_display(surface))->zorders;
    } else if (surface->parent) {
        /* Otherwise, zorders tracked by this surfaces parent */
        zorders = GDK_QNXSCREEN_SURFACE(surface->parent)->zorders;
    }
    if (zorders) {
        /* Get the QNX Screen display impl */
        GdkQnxScreenSurface *impl = GDK_QNXSCREEN_SURFACE(surface);

        /* Get the current highest zorder from the GdkDisplay */
        int new_zorder = (int) (intptr_t) g_queue_peek_head(zorders);
        /* Increment the zorder */
        new_zorder++;
        /* set window's z-order */
        int ret = screen_set_window_property_iv(impl->window_handle, SCREEN_PROPERTY_ZORDER, &new_zorder);
        if (ret) {
            g_critical (G_STRLOC ": screen_set_window_property_iv(SCREEN_PROPERTY_ZORDER): %s", strerror(errno));
        } else {
            ret = screen_flush_context(impl->context_handle, 0);
            if (ret) {
                g_critical (G_STRLOC ": screen_flush_context(): %s", strerror(errno));
            } else {
                intptr_t qval = (intptr_t) new_zorder;
                g_queue_push_tail(zorders, (void*) qval);
                impl->zorder = new_zorder;

                GDK_DEBUG(MISC, "%s Set window's z-order to: %d", QNX_SCREEN, new_zorder);
            }
        }
    }
}

static void gdk_qnxscreen_surface_lower (GdkSurface *surface)
{
    GQueue *zorders = NULL;
    if (GDK_IS_TOPLEVEL(surface)) {
        /* Toplevel zorders are tracked by the GdkDisplay instance */
        zorders = GDK_QNXSCREEN_DISPLAY(gdk_surface_get_display(surface))->zorders;
    } else if (surface->parent) {
        /* Otherwise, zorders tracked by this surfaces parent */
        zorders = GDK_QNXSCREEN_SURFACE(surface->parent)->zorders;
    }
    if (zorders) {
        /* Get the QNX Screen display impl */
        GdkQnxScreenSurface *impl = GDK_QNXSCREEN_SURFACE(surface);

        /* Get the current lowest zorder from the GdkDisplay */
        int new_zorder = (int) (intptr_t) g_queue_peek_tail(zorders);
        /* Decrement the zorder */
        new_zorder--;
        /* set window's z-order */
        int ret = screen_set_window_property_iv(impl->window_handle, SCREEN_PROPERTY_ZORDER, &new_zorder);
        if (ret) {
            g_critical (G_STRLOC ": screen_set_window_property_iv(SCREEN_PROPERTY_ZORDER): %s", strerror(errno));
        } else {
            ret = screen_flush_context(impl->context_handle, 0);
            if (ret) {
                g_critical (G_STRLOC ": screen_flush_context(): %s", strerror(errno));
            } else {
                intptr_t qval = (intptr_t) new_zorder;
                g_queue_push_tail(zorders, (void*) qval);
                impl->zorder = new_zorder;

                GDK_DEBUG(MISC, "%s Set window's z-order to: %d", QNX_SCREEN, new_zorder);
            }
        }
    }
}

static gboolean
gdk_qnxscreen_toplevel_lower (GdkToplevel *toplevel)
{
  gdk_qnxscreen_surface_lower (GDK_SURFACE (toplevel));

  return TRUE;
}

static void gdk_qnxscreen_toplevel_present(GdkToplevel* toplevel, GdkToplevelLayout *layout)
{
    GdkSurface* surface = GDK_SURFACE(toplevel);
    GdkDisplay *display = gdk_surface_get_display (surface);
    GdkMonitor *monitor;
    GdkToplevelSize size;
    int bounds_width, bounds_height;
    int width, height;
    GdkGeometry geometry;
    GdkSurfaceHints mask;
    gboolean fullscreen;
    gboolean maximize;

    monitor = gdk_display_get_monitor_at_surface (display, surface);
    if (monitor)
    {
        GdkRectangle monitor_geometry;
        gdk_monitor_get_geometry(monitor, &monitor_geometry);
        bounds_width = monitor_geometry.width;
        bounds_height = monitor_geometry.height;
        GDK_DEBUG(MISC, "%s monitor bounds - size %dx%d", QNX_SCREEN, bounds_width, bounds_height);

        /* Move the surface to this monitor internally */
        gdk_qnxscreen_surface_move_to_monitor(surface, monitor);
    }
    else
    {
      bounds_width = G_MAXINT;
      bounds_height = G_MAXINT;
    }

    gdk_toplevel_size_init (&size, bounds_width, bounds_height);
    gdk_toplevel_notify_compute_size (GDK_TOPLEVEL (surface), &size);
    g_warn_if_fail (size.width > 0);
    g_warn_if_fail (size.height > 0);
    width = size.width;
    height = size.height;

    if (gdk_toplevel_layout_get_resizable (layout)) {
        geometry.min_width = size.min_width;
        geometry.min_height = size.min_height;
        mask = GDK_HINT_MIN_SIZE;
    } else {
        geometry.max_width = geometry.min_width = width;
        geometry.max_height = geometry.min_height = height;
        mask = GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE;
    }

    gdk_surface_constrain_size (&geometry, mask, width, height, &width, &height);
    gdk_qnxscreen_surface_move_resize(surface, FALSE, -1, -1, width, height);

    /* On QNX, just make maximize the same as fullscreen */
    if (gdk_toplevel_layout_get_maximized (layout, &maximize))
    {
        if (maximize)
            gdk_qnxscreen_surface_fullscreen (surface);
        else
            gdk_qnxscreen_surface_unfullscreen (surface);
    }

    if (gdk_toplevel_layout_get_fullscreen (layout, &fullscreen))
    {
        if (fullscreen)
        {
            GdkMonitor *fullscreen_monitor =
                gdk_toplevel_layout_get_fullscreen_monitor (layout);

            if (fullscreen_monitor)
                gdk_qnxscreen_surface_fullscreen_on_monitor (surface, fullscreen_monitor);
            else
                gdk_qnxscreen_surface_fullscreen (surface);
        }
        else
        {
            gdk_qnxscreen_surface_unfullscreen (surface);
        }
    }

    /* TODO: Do something with shadows internally in Screen? */

    int was_mapped = GDK_SURFACE_IS_MAPPED (surface);

    if (!was_mapped)
        gdk_surface_set_is_mapped (surface, TRUE);

    gdk_qnxscreen_surface_show (surface);

    if (!was_mapped)
        gdk_surface_invalidate_rect (surface, NULL);
}

static void gdk_qnxscreen_toplevel_init (GdkQnxScreenToplevel* toplevel)
{
    GDK_DEBUG(MISC, "%s initializing GdkQnxScreenToplevel: %p", QNX_SCREEN, toplevel);
}

static void gdk_qnxscreen_toplevel_constructed (GObject *object)
{
    GdkQnxScreenSurface *qnx_screen_surface = GDK_QNXSCREEN_SURFACE(object);
    GdkSurface *surface = GDK_SURFACE(qnx_screen_surface);
    GDK_DEBUG(MISC, "%s toplevel object constructed: %p", QNX_SCREEN, object);
    
    /* frame clock: a toplevel surface does not have a parent, so use the parent frame clock */
    gdk_surface_set_frame_clock(surface, _gdk_frame_clock_idle_new());
    
    /* create QnxScreen window of type SCREEN_APPLICATION_WINDOW */
    gdk_qnxscreen_create_window(qnx_screen_surface, SCREEN_APPLICATION_WINDOW);
    
    /* call parent */
    G_OBJECT_CLASS(gdk_qnxscreen_toplevel_parent_class)->constructed(object);
}

static void gdk_qnxscreen_toplevel_class_init (GdkQnxScreenToplevelClass* class)
{
    GDK_DEBUG(MISC, "%s initializing GdkQnxScreenToplevelClass: %p", QNX_SCREEN, class);

    GObjectClass *object_class = G_OBJECT_CLASS (class);
    object_class->constructed = gdk_qnxscreen_toplevel_constructed;
    object_class->get_property = gdk_qnxscreen_toplevel_get_property;
    object_class->set_property = gdk_qnxscreen_toplevel_set_property;    
    gdk_toplevel_install_properties (object_class, 1);
}

void gdk_qnxscreen_toplevel_iface_init (GdkToplevelInterface* iface)
{
    GDK_DEBUG(MISC, "%s initializing toplevel interface: %p", QNX_SCREEN, iface);

    iface->present = gdk_qnxscreen_toplevel_present;
    iface->lower = gdk_qnxscreen_toplevel_lower;
}

G_DEFINE_TYPE_WITH_CODE (GdkQnxScreenPopup, gdk_qnxscreen_popup, GDK_TYPE_QNXSCREEN_SURFACE,
                         G_IMPLEMENT_INTERFACE (GDK_TYPE_POPUP, gdk_qnxscreen_popup_iface_init))

#define LAST_PROP 1

static void gdk_qnxscreen_popup_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec)
{
    GDK_DEBUG(SETTINGS, "%s popup get property %d", QNX_SCREEN, prop_id);

    GdkSurface* surface = GDK_SURFACE (object);

    switch (prop_id)
    {
        case LAST_PROP + GDK_POPUP_PROP_PARENT:
            g_value_set_object (value, surface->parent);
            break;

        case LAST_PROP + GDK_POPUP_PROP_AUTOHIDE:
            g_value_set_boolean (value, surface->autohide);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void gdk_qnxscreen_popup_set_property (GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec)
{
    GDK_DEBUG(SETTINGS, "%s popup set property %d", QNX_SCREEN, prop_id);

    GdkSurface* surface = GDK_SURFACE (object);

    switch (prop_id)
    {
        case LAST_PROP + GDK_POPUP_PROP_PARENT:
            surface->parent = g_value_dup_object (value);
            if (surface->parent != NULL)
                surface->parent->children = g_list_prepend (surface->parent->children, surface);
            break;

        case LAST_PROP + GDK_POPUP_PROP_AUTOHIDE:
            surface->autohide = g_value_get_boolean (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void show_popup (GdkSurface* surface)
{
    gdk_qnxscreen_surface_raise(surface);
    gdk_surface_set_is_mapped (surface, TRUE);
    gdk_qnxscreen_surface_show (surface);
    gdk_surface_invalidate_rect (surface, NULL);
}

static void show_grabbing_popup (GdkSeat* seat, GdkSurface* surface, gpointer user_data)
{
    show_popup (surface);
}

static gboolean gdk_qnxscreen_surface_present_popup (GdkPopup* popup, int width, int height, GdkPopupLayout* layout)
{
    GDK_DEBUG(MISC, "%s present popup: %p", QNX_SCREEN, popup);

    GdkSurface* surface = GDK_SURFACE(popup);
    GdkQnxScreenSurface *impl = GDK_QNXSCREEN_SURFACE(surface);

    gdk_qnxscreen_surface_layout_popup(surface, width, height, layout);

    if (GDK_SURFACE_IS_MAPPED (surface))
        return TRUE;

    if (!surface->parent) {
        g_critical (G_STRLOC ": popup does not have a parent: %s", strerror(errno));
        return FALSE;
    }

    /* Need parent to create a group */
    gdk_qnxscreen_surface_create_group(surface->parent);

    GdkQnxScreenSurface* parent_impl = GDK_QNXSCREEN_SURFACE(surface->parent);
    if (NULL == parent_impl->group_handle || strlen(parent_impl->group_name) <= 0) {
        g_warning (G_STRLOC ": popup parent has invalid qnxscreen window group");
        return FALSE;
    }

    /* Make this window join the parents window group */
    int ret = screen_join_window_group(impl->window_handle, parent_impl->group_name);
    if (ret) {
        g_critical (G_STRLOC ": screen_join_window_group(): %s", strerror(errno));
    } else {
        ret = screen_flush_context(impl->context_handle, 0);
        if (ret) {
            g_critical (G_STRLOC ": screen_flush_context(): %s", strerror(errno));
        } else {
            GDK_DEBUG(MISC, "%s popup window joined qnxscreen window group: %s", QNX_SCREEN, parent_impl->group_name);
        }
    }

    if (surface->autohide) {
        gdk_seat_grab (gdk_display_get_default_seat (surface->display),
                        surface,
                        GDK_SEAT_CAPABILITY_ALL,
                        TRUE,
                        NULL, NULL,
                        show_grabbing_popup, NULL);
    } else {
        show_popup (surface);
    }

    return GDK_SURFACE_IS_MAPPED (surface);
}

static GdkGravity gdk_qnxscreen_popup_get_surface_anchor (GdkPopup* popup)
{
    GdkSurface* surface = GDK_SURFACE(popup);
    GDK_DEBUG(MISC, "%s popup get surface anchor %d", QNX_SCREEN, surface->popup.surface_anchor);
    return surface->popup.surface_anchor;
}

static GdkGravity gdk_qnxscreen_popup_get_rect_anchor (GdkPopup* popup)
{
    GdkSurface* surface = GDK_SURFACE(popup);
    GDK_DEBUG(MISC, "%s popup get rect anchor %d", QNX_SCREEN, surface->popup.rect_anchor);
    return surface->popup.rect_anchor;
}

static int gdk_qnxscreen_popup_get_position_x (GdkPopup* popup)
{
    GdkSurface* surface = GDK_SURFACE(popup);
    GDK_DEBUG(MISC, "%s popup get position x %d", QNX_SCREEN, surface->x);
    return surface->x;
}

static int gdk_qnxscreen_popup_get_position_y (GdkPopup* popup)
{
    GdkSurface* surface = GDK_SURFACE(popup);
    GDK_DEBUG(MISC, "%s popup get position y %d", QNX_SCREEN, surface->y);
    return surface->y;
}

static void gdk_qnxscreen_popup_init (GdkQnxScreenPopup *popup)
{
    GDK_DEBUG(MISC, "%s initializing GdkQnxScreenPopup: %p", QNX_SCREEN, popup);
}

static void gdk_qnxscreen_popup_constructed (GObject *object)
{
    GdkQnxScreenSurface *qnx_screen_surface = GDK_QNXSCREEN_SURFACE(object);
    GdkSurface *surface = GDK_SURFACE(qnx_screen_surface);
    GDK_DEBUG(MISC, "%s popup object constructed: %p", QNX_SCREEN, object);
    
    /* frame clock: a popup must have a parent, so use the parent frame clock */
    gdk_surface_set_frame_clock(surface, gdk_surface_get_frame_clock(surface->parent));
    
    /* create QnxScreen window of type SCREEN_CHILD_WINDOW */
    gdk_qnxscreen_create_window(qnx_screen_surface, SCREEN_CHILD_WINDOW);
    
    /* call parent */
    G_OBJECT_CLASS(gdk_qnxscreen_popup_parent_class)->constructed(object);
}

static void gdk_qnxscreen_popup_class_init (GdkQnxScreenPopupClass* class)
{
    GDK_DEBUG(MISC, "%s initializing GdkQnxScreenPopupClass: %p", QNX_SCREEN, class);

    GObjectClass *object_class = G_OBJECT_CLASS (class);
    object_class->constructed = gdk_qnxscreen_popup_constructed;
    object_class->get_property = gdk_qnxscreen_popup_get_property;
    object_class->set_property = gdk_qnxscreen_popup_set_property;
    gdk_popup_install_properties (object_class, 1);
}

void gdk_qnxscreen_popup_iface_init (GdkPopupInterface* iface)
{
    GDK_DEBUG(MISC, "%s initializing popup interface: %p", QNX_SCREEN, iface);
    iface->present = gdk_qnxscreen_surface_present_popup;
    iface->get_surface_anchor = gdk_qnxscreen_popup_get_surface_anchor;
    iface->get_rect_anchor = gdk_qnxscreen_popup_get_rect_anchor;
    iface->get_position_x = gdk_qnxscreen_popup_get_position_x;
    iface->get_position_y = gdk_qnxscreen_popup_get_position_y;
}
