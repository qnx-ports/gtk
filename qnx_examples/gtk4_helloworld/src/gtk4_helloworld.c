#include <gtk/gtk.h>

static void print_button (GtkWidget *widget, gpointer data)
{
	g_print ("button Hello World\n");
}

static void print_toggled (GtkWidget *widget, gpointer data)
{
	g_print ("check box Hello World\n");
}

static void activate (GtkApplication* app, gpointer user_data)
{
	GtkWidget* window = NULL;
	GtkWidget* box = NULL;
	GtkWidget* button = NULL;
	GtkWidget* check_box = NULL;
	GtkWidget* text_box = NULL;
	GtkWidget* color_picker_btn = NULL;
	GtkWidget* scale_slider = NULL;
	GtkWidget* volume_btn = NULL;
	GtkWidget* close_btn = NULL;

	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Window");
	gtk_window_set_default_size (GTK_WINDOW (window), 500, 600);

	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 20);
	gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
	gtk_widget_set_valign (box, GTK_ALIGN_CENTER);
	gtk_window_set_child (GTK_WINDOW (window), box);

	button = gtk_button_new_with_label ("Hello World");
	g_signal_connect (button, "clicked", G_CALLBACK (print_button), NULL);
	gtk_box_append (GTK_BOX (box), button);

	check_box = gtk_check_button_new_with_label("check box");
	g_signal_connect (check_box, "toggled", G_CALLBACK (print_toggled), NULL);
	gtk_box_append (GTK_BOX (box), check_box);

	text_box = gtk_entry_new();
	gtk_box_append (GTK_BOX (box), text_box);

	color_picker_btn = gtk_color_button_new();
	gtk_box_append (GTK_BOX (box), color_picker_btn);

	scale_slider = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 100, 5);
	gtk_scale_set_draw_value (GTK_SCALE(scale_slider), TRUE);
	gtk_box_append (GTK_BOX (box), scale_slider);

	volume_btn = gtk_volume_button_new ();
	gtk_box_append (GTK_BOX (box), volume_btn);

	close_btn = gtk_button_new_with_label("Close");
	g_signal_connect_swapped (close_btn, "clicked", G_CALLBACK (gtk_window_destroy), window);
	gtk_box_append (GTK_BOX (box), close_btn);

	gtk_widget_show (window);
}

int main (int argc, char **argv)
{
	int status;
	GtkApplication *app = NULL;

	app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);

	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}
