#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#ifndef IMAGE_RESOURCE_DIR
	#error IMAGE_RESOURCE_DIR for the target location of the image resources must be defined.
#endif

int temperature = 21;
char temp_str[4] = {0};
GtkWidget* window = NULL;
GtkWidget* fixed_container = NULL;
GtkWidget* background = NULL;
GtkWidget* temperature_frame = NULL;
GtkWidget* up_btn = NULL;
GtkWidget* down_btn = NULL;
GtkWidget* up_img = NULL;
GtkWidget* down_img = NULL;
GtkWidget* temperature_bar = NULL;
GtkWidget* target_label = NULL;
GtkCssProvider* provider = NULL;

static void temperature_up(GtkWidget *widget, gpointer data)
{
	g_print ("temp up\n");
	temperature++;
	sprintf(temp_str, "%d°", temperature);
	gtk_label_set_text (GTK_LABEL(target_label), temp_str);
	gtk_level_bar_set_value (GTK_LEVEL_BAR(temperature_bar), temperature);
}

static void temperature_down(GtkWidget *widget, gpointer data)
{
	g_print ("temp down\n");
	temperature--;
	sprintf(temp_str, "%d°", temperature);
	gtk_label_set_text (GTK_LABEL(target_label), temp_str);
	gtk_level_bar_set_value (GTK_LEVEL_BAR(temperature_bar), temperature);
}

static void activate (GtkApplication* app, gpointer user_data)
{
	/* main window */
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Window");
	gtk_window_set_default_size (GTK_WINDOW (window), 1280, 720);
	fixed_container = gtk_fixed_new ();

	/* background */
	background = gtk_picture_new_for_filename (IMAGE_RESOURCE_DIR"/bg_metal_1.png");
	gtk_widget_set_size_request (background, 1280, 720);
	gtk_fixed_put (GTK_FIXED(fixed_container), background, 0, 0);

	/* temperature bar */
	temperature_bar = gtk_level_bar_new_for_interval (15, 30);
	gtk_widget_set_size_request (temperature_bar, 650, 150);
	gtk_level_bar_set_value (GTK_LEVEL_BAR(temperature_bar), 21);
	gtk_fixed_put (GTK_FIXED(fixed_container), temperature_bar, 200, 265);

	/* up button */
	up_btn = gtk_button_new();
	up_img = gtk_image_new_from_file (IMAGE_RESOURCE_DIR"/plus_up.png");
	gtk_button_set_has_frame (GTK_BUTTON(up_btn), FALSE);
	gtk_widget_set_size_request (up_btn, 200, 200);
	gtk_button_set_child (GTK_BUTTON(up_btn), up_img);
	gtk_fixed_put (GTK_FIXED(fixed_container), up_btn, 852, 60);
	g_signal_connect (up_btn, "clicked", G_CALLBACK (temperature_up), NULL);

	/* down button */
	down_btn = gtk_button_new();
	down_img = gtk_image_new_from_file (IMAGE_RESOURCE_DIR"/minus_up.png");
	gtk_button_set_has_frame (GTK_BUTTON(down_btn), FALSE);
	gtk_widget_set_size_request (down_btn, 200, 200);
	gtk_button_set_child (GTK_BUTTON(down_btn), down_img);
	gtk_fixed_put (GTK_FIXED(fixed_container), down_btn, 852, 425);
	g_signal_connect (down_btn, "clicked", G_CALLBACK (temperature_down), NULL);

	/* temperature frame */
	temperature_frame = gtk_image_new_from_file (IMAGE_RESOURCE_DIR"/weather_frame.png");
	gtk_widget_set_size_request (temperature_frame, 300, 300);
	gtk_fixed_put (GTK_FIXED(fixed_container), temperature_frame, 800, 200);

	/* target temperature label */
	target_label = gtk_label_new ("21°");
	provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(provider, "#temperature { color: black; font-weight: bold; font-size: 80px }", -1);
	gtk_style_context_add_provider(gtk_widget_get_style_context(target_label), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	gtk_widget_set_name(target_label, "temperature");
	gtk_widget_set_size_request (target_label, 100, 100);
	gtk_fixed_put (GTK_FIXED(fixed_container), target_label, 890, 270);

	/* show window */
	gtk_window_set_child (GTK_WINDOW (window), fixed_container);
	gtk_widget_show (window);
}

int main (int argc, char **argv)
{
	GtkApplication *app;
	int status;

	gtk_init ();

	app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);

	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}
