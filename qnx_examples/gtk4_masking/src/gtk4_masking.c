/* Masking
 *
 * Demonstrates mask nodes.
 *
 * This demo uses a text node as mask for
 * an animated linear gradient.
 */

#include <gtk/gtk.h>
#include "demo4widget.h"


static void
activate (GtkApplication *app)
{
  static GtkWidget *window, *headerbar = NULL;

  GtkWidget *box;
  GtkWidget *demo;
  GtkWidget *scale;
  GdkDisplay *display = gdk_display_get_default();
	GdkMonitor *monitor = gdk_display_get_monitor_at_surface(display, NULL);
	GdkRectangle geometry;
  int screen_width = 0, screen_height = 0;
	gdk_monitor_get_geometry(monitor, &geometry);
	g_print("Fulscreen size: %d x %d \n", geometry.width, geometry.height);
	screen_width = geometry.width;
	screen_height = geometry.height;

  window = gtk_application_window_new(app);
  headerbar = gtk_header_bar_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Layout Manager — Transition");
  gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);
  gtk_header_bar_set_decoration_layout(GTK_HEADER_BAR(headerbar), ":close");
  gtk_window_set_default_size (GTK_WINDOW (window), screen_width, screen_height);
  
  g_object_add_weak_pointer (G_OBJECT (window), (gpointer *)&window);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_window_set_child (GTK_WINDOW (window), box);

  demo = demo4_widget_new ();
  gtk_widget_set_hexpand (demo, TRUE);
  gtk_widget_set_vexpand (demo, TRUE);

  gtk_box_append (GTK_BOX (box), demo);

  scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1, 0.1);
  gtk_range_set_value (GTK_RANGE (scale), 0.5);
  g_object_bind_property (gtk_range_get_adjustment (GTK_RANGE (scale)), "value", demo, "progress", 0);

  gtk_box_append (GTK_BOX (box), scale);

  gtk_window_present(GTK_WINDOW(window));
}


int
main (int argc, char **argv)
{
  GtkApplication *app;

  app = gtk_application_new ("com.example.GtkMasking", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return 0;
}
