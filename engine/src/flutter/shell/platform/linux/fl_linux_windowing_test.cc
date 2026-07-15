// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_linux_windowing.h"

#include "flutter/shell/platform/linux/testing/linux_test.h"
#if !FLUTTER_LINUX_GTK4
#include "flutter/shell/platform/linux/testing/mock_gtk.h"
#include "gmock/gmock.h"
#endif
#include "gtest/gtest.h"

class FlLinuxWindowingTest : public flutter::testing::LinuxTest {
 protected:
#if !FLUTTER_LINUX_GTK4
  ::testing::NiceMock<flutter::testing::MockGtk> mock_gtk;
#endif
};

TEST_F(FlLinuxWindowingTest, ReportsGtkMajorVersion) {
#if FLUTTER_LINUX_GTK4
  EXPECT_EQ(fl_linux_windowing_get_gtk_major_version(), 4);
#else
  EXPECT_EQ(fl_linux_windowing_get_gtk_major_version(), 3);
#endif
}

TEST_F(FlLinuxWindowingTest, SetsWindowSize) {
  g_autoptr(GtkWindow) window = GTK_WINDOW(
      g_object_ref_sink(g_object_new(gtk_window_get_type(), nullptr)));
#if FLUTTER_LINUX_GTK4
  fl_linux_windowing_set_window_size(window, 640, 480);
  gint width = 0;
  gint height = 0;
  fl_linux_windowing_get_window_size(window, &width, &height);
  EXPECT_EQ(width, 640);
  EXPECT_EQ(height, 480);
#else
  EXPECT_CALL(mock_gtk, gtk_window_resize(window, 640, 480));
  fl_linux_windowing_set_window_size(window, 640, 480);
#endif
}

#if FLUTTER_LINUX_GTK4
TEST_F(FlLinuxWindowingTest, GetsRealizedSurfaceSize) {
  g_autoptr(GtkWindow) window = GTK_WINDOW(
      g_object_ref_sink(g_object_new(gtk_window_get_type(), nullptr)));
  gtk_window_set_default_size(window, 640, 480);
  gtk_widget_realize(GTK_WIDGET(window));
  GdkSurface* surface = gtk_native_get_surface(GTK_NATIVE(window));
  ASSERT_NE(surface, nullptr);

  gint width = 0;
  gint height = 0;
  fl_linux_windowing_get_window_size(window, &width, &height);
  EXPECT_EQ(width, gdk_surface_get_width(surface));
  EXPECT_EQ(height, gdk_surface_get_height(surface));
}
#endif

TEST_F(FlLinuxWindowingTest, SetsWindowConstraints) {
  g_autoptr(GtkWindow) window = GTK_WINDOW(
      g_object_ref_sink(g_object_new(gtk_window_get_type(), nullptr)));
#if FLUTTER_LINUX_GTK4
  EXPECT_TRUE(fl_linux_windowing_set_window_constraints(window, 320, 240,
                                                        G_MAXINT, G_MAXINT));
  gint width = 0;
  gint height = 0;
  gtk_widget_get_size_request(GTK_WIDGET(window), &width, &height);
  EXPECT_EQ(width, 320);
  EXPECT_EQ(height, 240);
  EXPECT_FALSE(
      fl_linux_windowing_set_window_constraints(window, 320, 240, 640, 480));
#else
  EXPECT_CALL(mock_gtk, gtk_window_set_geometry_hints(
                            window, nullptr, ::testing::_, ::testing::_))
      .WillOnce([](GtkWindow*, GtkWidget*, GdkGeometry* geometry,
                   GdkWindowHints hints) {
        EXPECT_EQ(geometry->min_width, 320);
        EXPECT_EQ(geometry->min_height, 240);
        EXPECT_EQ(geometry->max_width, 640);
        EXPECT_EQ(geometry->max_height, 480);
        EXPECT_EQ(hints, GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);
      });
  EXPECT_TRUE(
      fl_linux_windowing_set_window_constraints(window, 320, 240, 640, 480));
#endif
}

TEST_F(FlLinuxWindowingTest, SetsWindowMinimized) {
  g_autoptr(GtkWindow) window = GTK_WINDOW(
      g_object_ref_sink(g_object_new(gtk_window_get_type(), nullptr)));
#if FLUTTER_LINUX_GTK4
  fl_linux_windowing_set_window_minimized(window, TRUE);
  fl_linux_windowing_set_window_minimized(window, FALSE);
#else
  EXPECT_CALL(mock_gtk, gtk_window_iconify(window));
  fl_linux_windowing_set_window_minimized(window, TRUE);
  EXPECT_CALL(mock_gtk, gtk_window_deiconify(window));
  fl_linux_windowing_set_window_minimized(window, FALSE);
#endif
}

TEST_F(FlLinuxWindowingTest, InitialWindowStateIsEmpty) {
  g_autoptr(GtkWindow) window = GTK_WINDOW(
      g_object_ref_sink(g_object_new(gtk_window_get_type(), nullptr)));
  gtk_widget_realize(GTK_WIDGET(window));
  EXPECT_EQ(fl_linux_windowing_get_window_state(window), 0u);
}

TEST_F(FlLinuxWindowingTest, DestroysWindow) {
  g_autoptr(GtkWindow) window = GTK_WINDOW(
      g_object_ref_sink(g_object_new(gtk_window_get_type(), nullptr)));
#if FLUTTER_LINUX_GTK4
  gtk_widget_realize(GTK_WIDGET(window));
  gboolean unrealized = FALSE;
  g_signal_connect(window, "unrealize",
                   G_CALLBACK(+[](GtkWindow*, gpointer user_data) {
                     *static_cast<gboolean*>(user_data) = TRUE;
                   }),
                   &unrealized);
  fl_linux_windowing_destroy_window(window);
  EXPECT_TRUE(unrealized);
#else
  EXPECT_CALL(mock_gtk, gtk_widget_destroy(GTK_WIDGET(window)));
  fl_linux_windowing_destroy_window(window);
#endif
}
