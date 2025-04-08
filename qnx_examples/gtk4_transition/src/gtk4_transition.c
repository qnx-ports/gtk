/* Layout Manager/Transition
 * #Keywords: GtkLayoutManager
 *
 * This demo shows a simple example of a custom layout manager
 * and a widget using it. The layout manager places the children
 * of the widget in a grid or a circle.
 *
 * The widget is animating the transition between the two layouts.
 *
 * Click to start the transition.
 */

#include <gtk/gtk.h>

#include "demowidget.h"
#include "demochild.h"


static void
activate (GtkApplication *app)
{
  static GtkWidget *window, *headerbar= NULL;

  GtkWidget *widget;
  GtkWidget *child;
  const char *color[] = {
    "red", "orange", "yellow", "green",
    "blue", "grey", "magenta", "lime",
    "yellow", "firebrick", "aqua", "purple",
    "tomato", "pink", "thistle", "maroon"
  };
  int i;
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

  widget = demo_widget_new ();

  for (i = 0; i < 16; i++)
    {
      child = demo_child_new (color[i]);
      gtk_widget_set_margin_start (child, 4);
      gtk_widget_set_margin_end (child, 4);
      gtk_widget_set_margin_top (child, 4);
      gtk_widget_set_margin_bottom (child, 4);
      demo_widget_add_child (DEMO_WIDGET (widget), child);
    }

  gtk_window_set_child (GTK_WINDOW (window), widget);
  
  gtk_window_present(GTK_WINDOW(window));

}

int
main (int argc, char **argv)
{
  GtkApplication *app;

  app = gtk_application_new ("com.example.GtkTransition", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return 0;
}
