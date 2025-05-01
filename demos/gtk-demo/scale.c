/* Scales
 *
 * GtkScale is a way to select a value from a range.
 * Scales can have marks to help pick special values,
 * and they can also restrict the values that can be
 * chosen.
 */

#include <gtk/gtk.h>

GtkWidget *
do_scale (GtkWidget *do_widget)
{
  static GtkWidget *window = NULL;
  GtkWidget *headerbar;

  if (!window)
    {
      GtkBuilder *builder;

      builder = gtk_builder_new_from_resource ("/scale/scale.ui");
      window = GTK_WIDGET (gtk_builder_get_object (builder, "window1"));
      headerbar = gtk_header_bar_new ();
      gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);
      gtk_header_bar_set_decoration_layout(GTK_HEADER_BAR(headerbar), ":close");
      gtk_window_set_display (GTK_WINDOW (window),
                              gtk_widget_get_display (do_widget));
      g_object_add_weak_pointer (G_OBJECT (window), (gpointer *)&window);

      g_object_unref (builder);
    }

  if (!gtk_widget_get_visible (window))
    gtk_widget_set_visible (window, TRUE);
  else
    gtk_window_destroy (GTK_WINDOW (window));


  return window;
}
