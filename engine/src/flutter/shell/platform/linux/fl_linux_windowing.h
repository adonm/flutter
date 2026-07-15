// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_LINUX_WINDOWING_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_LINUX_WINDOWING_H_

#include <gtk/gtk.h>

#include "flutter/shell/platform/linux/public/flutter_linux/fl_engine.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_view.h"

G_BEGIN_DECLS

typedef struct {
  GtkWindow* window;
  FlView* view;
  gint64 view_id;
} FlLinuxWindowingWindow;

typedef enum {
  FL_LINUX_WINDOWING_WINDOW_STATE_MINIMIZED = 1 << 0,
  FL_LINUX_WINDOWING_WINDOW_STATE_MAXIMIZED = 1 << 1,
  FL_LINUX_WINDOWING_WINDOW_STATE_FULLSCREEN = 1 << 2,
} FlLinuxWindowingWindowState;

gint fl_linux_windowing_get_gtk_major_version(void);

FlLinuxWindowingWindow* fl_linux_windowing_create_regular_window(
    FlEngine* engine,
    gboolean has_preferred_size,
    gint preferred_width,
    gint preferred_height,
    gboolean has_preferred_constraints,
    gint min_width,
    gint min_height,
    gint max_width,
    gint max_height,
    const gchar* title,
    gboolean decorated,
    gboolean resizable);

FlLinuxWindowingWindow* fl_linux_windowing_create_dialog_window(
    FlEngine* engine,
    GtkWindow* parent,
    gboolean has_preferred_size,
    gint preferred_width,
    gint preferred_height,
    gboolean has_preferred_constraints,
    gint min_width,
    gint min_height,
    gint max_width,
    gint max_height,
    const gchar* title,
    gboolean decorated,
    gboolean resizable);

void fl_linux_windowing_destroy_window(GtkWindow* window);

void fl_linux_windowing_get_window_size(GtkWindow* window,
                                        gint* width,
                                        gint* height);

guint fl_linux_windowing_get_window_state(GtkWindow* window);

void fl_linux_windowing_set_window_size(GtkWindow* window,
                                        gint width,
                                        gint height);

gboolean fl_linux_windowing_set_window_constraints(GtkWindow* window,
                                                   gint min_width,
                                                   gint min_height,
                                                   gint max_width,
                                                   gint max_height);

void fl_linux_windowing_set_window_minimized(GtkWindow* window,
                                             gboolean minimized);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_LINUX_WINDOWING_H_
