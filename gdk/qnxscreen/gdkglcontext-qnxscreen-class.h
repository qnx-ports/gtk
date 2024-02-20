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

#ifndef __GDK_QNXSCREEN_GL_CONTEXT_CLASS_H__
#define __GDK_QNXSCREEN_GL_CONTEXT_CLASS_H__

#include <gdk/gdk.h>

typedef struct _GdkQnxScreenGLContext GdkQnxScreenGLContext;
typedef struct _GdkQnxScreenGLContextClass GdkQnxScreenGLContextClass;

#define GDK_TYPE_QNXSCREEN_GL_CONTEXT (gdk_qnxscreen_gl_context_get_type ())
#define GDK_QNXSCREEN_GL_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDK_TYPE_QNXSCREEN_GL_CONTEXT, GdkQnxScreenGLContext))
#define GDK_QNXSCREEN_IS_GL_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDK_TYPE_QNXSCREEN_GL_CONTEXT))

GType gdk_qnxscreen_gl_context_get_type (void);
GdkGLContext *gdk_qnxscreen_display_init_gl (GdkDisplay *display, GError **error);

#endif
