#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#ifndef IMAGE_RESOURCE_DIR
	#error IMAGE_RESOURCE_DIR for the target location of the image resources must be defined.
#endif

int temperature = 21;
char temp_str[4] = {0};
GtkWidget* window = NULL;
GtkWidget* target_label = NULL;
int screen_width = 0, screen_height = 0;
int baseline_width = 1280, baseline_height = 720;

// The original size is absolute size based on a screen resolution of 1080*720
// After screen_width and screen_height retrieved
int convert_relative_width(int original_width) {
	g_assert (screen_width != 0);
	g_print ("converted size from %d to %d \n", original_width, original_width * screen_width / baseline_width);
	return original_width * screen_width / baseline_width;
}

int convert_relative_height(int original_height) {
	g_assert (screen_height != 0);
	return original_height * screen_height / baseline_height;
}

static void temperature_up(GtkWidget *widget, gpointer data)
{
	g_print ("temp up press\n");
	temperature++;
	sprintf(temp_str, "%d°", temperature);
	gtk_label_set_text (GTK_LABEL(target_label), temp_str);
}

static void temperature_down(GtkWidget *widget, gpointer data)
{
	g_print ("temp down press\n");
	temperature--;
	sprintf(temp_str, "%d°", temperature);
	gtk_label_set_text (GTK_LABEL(target_label), temp_str);
}

static void settings_press(GtkWidget *widget, gpointer data)
{
	g_print ("settings press\n");

	GtkWidget* dialog = NULL;
	GtkWidget* content_area = NULL;
	GtkWidget* fixed_container = NULL;
	GtkWidget* dialog_bg = NULL;
	GtkWidget* switch_btn = NULL;
	GtkWidget* spin_btn = NULL;
	GtkCssProvider* provider = NULL;
	GtkWidget* switch_label = NULL;
	GtkWidget* spin_label = NULL;
	GtkWidget* cancel_btn = NULL;
	GtkWidget* cancel_img = NULL;
	GtkWidget* close_btn = NULL;
	GtkWidget* close_img = NULL;

	/* create popup dialog */
	dialog = gtk_dialog_new ();
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_window_set_default_size (GTK_WINDOW (dialog), screen_width, screen_height);
	fixed_container = gtk_fixed_new ();
	dialog_bg = gtk_picture_new_for_filename (IMAGE_RESOURCE_DIR"/bg.png");
	gtk_widget_set_size_request (dialog_bg, screen_width, screen_height);
	g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_window_destroy), dialog);

	/* css for labels */
	provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(provider, "#dialog_label { color: white; font-weight: bold; font-size: 30px; }", -1);
	gtk_style_context_add_provider(gtk_widget_get_style_context(target_label), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	/* create various widgets for demo purposes */
	switch_btn = gtk_switch_new ();
	spin_btn = gtk_spin_button_new_with_range (-25.0, 25.0, 1.0);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(spin_btn), 0);
	cancel_btn = gtk_button_new();
	cancel_img = gtk_picture_new_for_filename (IMAGE_RESOURCE_DIR"/cancel_btn.png");
	gtk_button_set_has_frame (GTK_BUTTON(cancel_btn), FALSE);
	gtk_widget_set_size_request (cancel_btn, convert_relative_width (339), convert_relative_height (88));
	gtk_button_set_child (GTK_BUTTON(cancel_btn), cancel_img);
	g_signal_connect_swapped (cancel_btn, "clicked", G_CALLBACK (gtk_window_destroy), dialog);

	close_btn = gtk_button_new();
	close_img = gtk_picture_new_for_filename (IMAGE_RESOURCE_DIR"/close_btn.png");
	gtk_button_set_has_frame (GTK_BUTTON(close_btn), FALSE);
	gtk_widget_set_size_request (close_btn, convert_relative_width (339), convert_relative_height (88));
	gtk_button_set_child (GTK_BUTTON(close_btn), close_img);
	g_signal_connect_swapped (close_btn, "clicked", G_CALLBACK (gtk_window_destroy), dialog);

	/* create labels */
	switch_label = gtk_label_new ("Setting switch:");
	gtk_widget_set_name(switch_label, "dialog_label");
	gtk_widget_set_size_request (switch_label, convert_relative_width (200), convert_relative_height (200));
	gtk_style_context_add_provider(gtk_widget_get_style_context(switch_label), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	spin_label = gtk_label_new ("Spin button:");
	gtk_widget_set_name(spin_label, "dialog_label");
	gtk_widget_set_size_request (spin_label, convert_relative_width (200), convert_relative_height (200));
	gtk_style_context_add_provider(gtk_widget_get_style_context(spin_label), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	/* add widgets to dialogs */
	gtk_fixed_put (GTK_FIXED(fixed_container), dialog_bg, 0, 0);
	gtk_fixed_put (GTK_FIXED(fixed_container), switch_label, convert_relative_width (300), convert_relative_height (210));
	gtk_fixed_put (GTK_FIXED(fixed_container), switch_btn, convert_relative_width (600), convert_relative_height (300));
	gtk_fixed_put (GTK_FIXED(fixed_container), spin_label, convert_relative_width (300), convert_relative_height (300));
	gtk_fixed_put (GTK_FIXED(fixed_container), spin_btn, convert_relative_width (600), convert_relative_height (385));
	gtk_fixed_put (GTK_FIXED(fixed_container), close_btn, convert_relative_width (950), convert_relative_height (500));
	gtk_fixed_put (GTK_FIXED(fixed_container), cancel_btn, convert_relative_width (950), convert_relative_height (600));

	/* show dialog */
	gtk_box_append (GTK_BOX (content_area), fixed_container);
	gtk_widget_show (dialog);
}

static void heat_press(GtkWidget *widget, gpointer data)
{
	g_print ("heat press\n");
	// todo toggle to heat mode
}

static void ac_press(GtkWidget *widget, gpointer data)
{
	g_print ("ac press\n");
	// todo toggle to ac mode
}

static void status_press(GtkWidget *widget, gpointer data)
{
	g_print ("status press\n");
}

static void activate (GtkApplication* app, gpointer user_data)
{
	GtkWidget* fixed_container = NULL;
	GtkWidget* background = NULL;
	GtkWidget* temperature_frame = NULL;
	GtkWidget* up_btn = NULL;
	GtkWidget* down_btn = NULL;
	GtkWidget* up_img = NULL;
	GtkWidget* down_img = NULL;
	GtkCssProvider* provider = NULL;
	GtkWidget* settings_btn = NULL;
	GtkWidget* settings_img = NULL;
	GtkWidget* heat_btn = NULL;
	GtkWidget* heat_img = NULL;
	GtkWidget* ac_btn = NULL;
	GtkWidget* ac_img = NULL;
	GtkWidget* status_btn = NULL;
	GtkWidget* status_img = NULL;

	GdkDisplay *display = gdk_display_get_default();
	GdkMonitor *monitor = gdk_display_get_monitor_at_surface(display, NULL);
	GdkRectangle geometry;
	gdk_monitor_get_geometry(monitor, &geometry);
	g_print("Fulscreen size: %d x %d \n", geometry.width, geometry.height);
	screen_width = geometry.width;
	screen_height = geometry.height;


	/* main window */
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Window");
	gtk_window_set_default_size (GTK_WINDOW (window), screen_width, screen_height);
	fixed_container = gtk_fixed_new ();

	// vertical_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);

	/* background */
	background = gtk_picture_new_for_filename (IMAGE_RESOURCE_DIR"/bg.png");
	gtk_widget_set_size_request (background, screen_width, screen_height);
	gtk_fixed_put (GTK_FIXED(fixed_container), background, 0, 0);

	/* up button */
	up_btn = gtk_button_new();
	up_img = gtk_picture_new_for_filename (IMAGE_RESOURCE_DIR"/temp_up.png");
	gtk_button_set_has_frame (GTK_BUTTON(up_btn), FALSE);
	gtk_widget_set_size_request (up_btn, convert_relative_width(300), convert_relative_height(220));
	gtk_button_set_child (GTK_BUTTON(up_btn), up_img);
	gtk_fixed_put (GTK_FIXED(fixed_container), up_btn, convert_relative_width(780), convert_relative_height(93));
	g_signal_connect (up_btn, "clicked", G_CALLBACK (temperature_up), NULL);

	/* down button */
	down_btn = gtk_button_new();
	down_img = gtk_picture_new_for_filename (IMAGE_RESOURCE_DIR"/temp_down.png");
	gtk_button_set_has_frame (GTK_BUTTON(down_btn), FALSE);
	gtk_widget_set_size_request (down_btn, convert_relative_width (315), convert_relative_height (235));
	gtk_button_set_child (GTK_BUTTON(down_btn), down_img);
	gtk_fixed_put (GTK_FIXED(fixed_container), down_btn, convert_relative_width (773), convert_relative_height (305));
	g_signal_connect (down_btn, "clicked", G_CALLBACK (temperature_down), NULL);

	/* temperature frame */
	temperature_frame = gtk_picture_new_for_filename (IMAGE_RESOURCE_DIR"/temp_guage.png");
	gtk_widget_set_size_request (temperature_frame, convert_relative_width (475), convert_relative_height (475));
	gtk_fixed_put (GTK_FIXED(fixed_container), temperature_frame, convert_relative_width (130), convert_relative_height (75));

	/* target temperature label */
	target_label = gtk_label_new ("21°");
	provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(provider, "#temperature { color: white; font-weight: bold; font-size: 150px; }", -1);
	gtk_style_context_add_provider(gtk_widget_get_style_context(target_label), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	gtk_widget_set_name(target_label, "temperature");
	gtk_widget_set_size_request (target_label, convert_relative_width (200), convert_relative_height (200));
	gtk_fixed_put (GTK_FIXED(fixed_container), target_label, convert_relative_width (270), convert_relative_height (210));

	/* settings button */
	settings_btn = gtk_button_new();
	settings_img = gtk_picture_new_for_filename (IMAGE_RESOURCE_DIR"/menu_settings_button.png");
	gtk_button_set_has_frame (GTK_BUTTON(settings_btn), FALSE);
	gtk_widget_set_size_request (settings_btn, convert_relative_width (339), convert_relative_height (88));
	gtk_button_set_child (GTK_BUTTON(settings_btn), settings_img);
	gtk_fixed_put (GTK_FIXED(fixed_container), settings_btn, convert_relative_width (56), convert_relative_height (632));
	g_signal_connect (settings_btn, "clicked", G_CALLBACK (settings_press), NULL);

	/* heat button */
	heat_btn = gtk_button_new();
	heat_img = gtk_picture_new_for_filename (IMAGE_RESOURCE_DIR"/menu_heat_button.png");
	gtk_button_set_has_frame (GTK_BUTTON(heat_btn), FALSE);
	gtk_widget_set_size_request (heat_btn, convert_relative_width (298), convert_relative_height (88));
	gtk_button_set_child (GTK_BUTTON(heat_btn), heat_img);
	gtk_fixed_put (GTK_FIXED(fixed_container), heat_btn, convert_relative_width (362), convert_relative_height (632));
	g_signal_connect (heat_btn, "clicked", G_CALLBACK (heat_press), NULL);

	/* ac button */
	ac_btn = gtk_button_new();
	ac_img = gtk_picture_new_for_filename (IMAGE_RESOURCE_DIR"/menu_ac_button.png");
	gtk_button_set_has_frame (GTK_BUTTON(ac_btn), FALSE);
	gtk_widget_set_size_request (ac_btn, convert_relative_width (298), convert_relative_height (88));
	gtk_button_set_child (GTK_BUTTON(ac_btn), ac_img);
	gtk_fixed_put (GTK_FIXED(fixed_container), ac_btn, convert_relative_width (628), convert_relative_height (632));
	g_signal_connect (ac_btn, "clicked", G_CALLBACK (ac_press), NULL);

	/* status button */
	status_btn = gtk_button_new();
	status_img = gtk_picture_new_for_filename (IMAGE_RESOURCE_DIR"/menu_status_button.png");
	gtk_button_set_has_frame (GTK_BUTTON(status_btn), FALSE);
	gtk_widget_set_size_request (status_btn, convert_relative_width (339), convert_relative_height (88));
	gtk_button_set_child (GTK_BUTTON(status_btn), status_img);
	gtk_fixed_put (GTK_FIXED(fixed_container), status_btn, convert_relative_width (892), convert_relative_height (632));
	g_signal_connect (status_btn, "clicked", G_CALLBACK (status_press), NULL);

	/* show window */
	gtk_window_set_child (GTK_WINDOW (window), fixed_container);
	gtk_widget_show (window);
}

static void
activate_quit (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       user_data)
{
  printf("======QUITED\n");
  GtkApplication *app = user_data;
  GtkWidget *win;
  GList *list, *next;

  list = gtk_application_get_windows (app);
  while (list)
    {
      win = list->data;
      next = list->next;

      gtk_window_destroy (GTK_WINDOW (win));

      list = next;
    }
}

static void activate_shift (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
	printf("Ctrl Shift . pressed\n");
}

static void activate_question (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
	printf("Ctrl question pressed\n");
}

static void activate_info (GSimpleAction *action, GVariant *parameter, gpointer user_data) {
	printf("Ctrl p pressed\n");
}

int main (int argc, char **argv)
{
	GtkApplication *app;
	int status;

	static GActionEntry app_entries[] = {
		{ "quit", activate_quit, NULL, NULL, NULL },
		{ "question", activate_question, NULL, NULL, NULL},
		{ "shift", activate_shift, NULL, NULL, NULL},
		{ "info", activate_info, NULL, NULL, NULL},
	};
	struct {
		const char *action_and_target;
		const char *accelerators[2];
	} accels[] = {
		{ "app.quit", { "<Control>q", NULL } },
		{ "app.shift", { "<Control><Shift>period", NULL } },
		{ "app.question", { "<Control>question", NULL } },
		{ "app.info", { "<Control>p", NULL } },
	};
	int i;

	gtk_init ();

	app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
	g_action_map_add_action_entries (G_ACTION_MAP (app),
									app_entries, G_N_ELEMENTS (app_entries),
									app);
  	for (i = 0; i < G_N_ELEMENTS (accels); i++) {
  		gtk_application_set_accels_for_action (app, accels[i].action_and_target, accels[i].accelerators);
	}

	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);

	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}
