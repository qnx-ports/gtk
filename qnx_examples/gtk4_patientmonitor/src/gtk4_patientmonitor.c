/*
* Copyright (c) 2025, BlackBerry Limited. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
 
// Constants for wave drawing and UI layout
#define AMPLITUDE 21 // Height of waveform
#define DISTANCE 200 // Width of the wave drawn per frame
#define BUFFER_SIZE 1000 // Size of waveform buffer
#define SCREEN_WIDTH  1920 // Window width
#define SCREEN_HEIGHT 1080 // Window height


// Text label positioning offsets
#define TEXT_COLUMN_1_OFFSET 20
#define TEXT_COLUMN_2_OFFSET 690
#define TEXT_COLUMN_3_OFFSET 360

// Layout region dividing waveform from text stats
#define WAVEFORM_END 1180
#define WAVEFORM_WIDTH (WAVEFORM_END - 1)

// Vertical Y offsets for placing waveforms
#define EKG_Y_OFFSET ((SCREEN_HEIGHT - 600) / 2)
#define PLETH_Y_OFFSET ((SCREEN_HEIGHT + 220) / 2)
#define RESP_Y_OFFSET ((SCREEN_HEIGHT + 550) / 2)

// Wave scroll speeds (lower = slower)
#define SCROLL_SPEED_EKG    0.7
#define SCROLL_SPEED_PLETH  0.95
#define SCROLL_SPEED_RESP   0.45

// Buffers to hold waveform values
double wave_buffer_ekg[BUFFER_SIZE] = {0.0}; 
double wave_buffer_pleth[BUFFER_SIZE] = {0.0};
double wave_buffer_resp[BUFFER_SIZE] = {0.0};

// Current X positions for waveform scrolling
double current_x_ekg = WAVEFORM_WIDTH;
double current_x_pleth = WAVEFORM_WIDTH;
double current_x_resp = WAVEFORM_WIDTH;

// Vital sign values and update ranges
int pulse_value = 75;
int max_pulse_change = 3;
 
int systolic_bp = 130; 
int diastolic_bp = 85; 
int max_bp_change = 2;
 
double body_temperature = 98.7; 
double max_temp_change = 0.3;
 
int respiratory_rate_value = 22;
int max_respiratory_change = 1;
 
int oximeter_percentage = 93;
int max_oximeter_change = 1;
 
int co2_level = 40;
int max_co2_change = 1;
 
// Simulated waveform pattern values
double pulse_pattern_ekg[25] = {1.75, 1.75, 1.75, 1.25, 0.25, 0.25, 0.25, 1.5, 1.5, 1.5, 1.5, 3.0, -5.0, 2.0, 2.0, 1.5, 1.5, 1.5, 1.5, 1.0, 1.0, 0.5, 0.5, 1.0, 1.0};
double pulse_pattern_pleth[25] = {-5.0, -6.0, -7.0, -8.0, -9.0, -9.25, -9.0, -5.0, -4.0, -2.0, -1.0, 0.0, 0.25, 0.0, -0.25, -0.5, -1.0, -2.0, -3.0, -3.5, -4.0, -4.5, -5.0, -5.0, -5.0};
double pulse_pattern_resp[25] = {6.0, 6.0, 6.0, 5.5, 5.0, 4.75, 4.0, 3.5, 3.0, 2.75, 2, 1.5, 1.25, 1.0, 1.0, 1.0, 1.25, 1.5, 2.0, 2.75, 3.0, 3.5, 4.0, 5.0, 5.75};

// Periodic animation update: refreshes vitals and redraws
static gboolean update_animation(gpointer user_data) {
    GtkWidget *drawing_area = user_data;

    if (g_random_int_range(0, 40) == 0) {
        pulse_value = CLAMP(pulse_value + g_random_int_range(-max_pulse_change, max_pulse_change + 1), 50, 120);
        respiratory_rate_value = CLAMP(respiratory_rate_value + g_random_int_range(-max_respiratory_change, max_respiratory_change + 1), 19, 23);
        oximeter_percentage = CLAMP(oximeter_percentage + g_random_int_range(-max_oximeter_change, max_oximeter_change + 1), 91, 100);
        systolic_bp = CLAMP(systolic_bp + g_random_int_range(-max_bp_change, max_bp_change + 1), 120, 140);
        diastolic_bp = CLAMP(diastolic_bp + g_random_int_range(-max_bp_change, max_bp_change + 1), 80, 90);
        body_temperature += ((double)rand() / RAND_MAX * 2 * max_temp_change) - max_temp_change;
        body_temperature = CLAMP(body_temperature, 97.0, 99.9);
        co2_level = CLAMP(co2_level + g_random_int_range(-max_co2_change, max_co2_change + 1), 35, 45);
    }

    gtk_widget_queue_draw(drawing_area);
    return G_SOURCE_CONTINUE;
}

// Scrolls waveform leftward at a constant speed
void update_wave_position(double *current_x, double speed) {
    *current_x -= speed;
    if (*current_x <= 0) {
        *current_x = WAVEFORM_WIDTH;
    }
}

// Renders a waveform using Cairo from a buffer of values
void draw_waveform(cairo_t *cr, const double *buffer, double current_x, double y_offset, int amplitude, double r, double g, double b) {
    double wave_start_x = fmod(current_x, BUFFER_SIZE);
    cairo_set_source_rgb(cr, r, g, b);
    cairo_set_line_width(cr, 3);

    for (int i = 0; i < DISTANCE; i++) {
        int buffer_idx = ((int)wave_start_x + i) % BUFFER_SIZE;
        double x = WAVEFORM_WIDTH - i * ((double)WAVEFORM_WIDTH / DISTANCE);
        double y = y_offset + amplitude * buffer[buffer_idx];
        if (i == 0) {
            cairo_move_to(cr, x, y);
        } else {
            cairo_line_to(cr, x, y);
        }
    }

    // Draw the wave to the screen
    cairo_stroke(cr);
}

// Utility to draw a single text label at a given position
void draw_label(cairo_t *cr, const char *text, double x, double y, double r, double g, double b, int font_size) {
    cairo_set_source_rgb(cr, r, g, b);
    cairo_move_to(cr, x, y);
    cairo_set_font_size(cr, font_size);
    cairo_show_text(cr, text);
}

// Main drawing routine for UI and waveforms
static void draw_callback(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer data) {

    cairo_set_source_rgb(cr, 0, 0, 0); // Black background
    cairo_paint(cr);
 
    cairo_t *cr2 = cairo_create(cairo_get_target(cr));
    cairo_t *cr3 = cairo_create(cairo_get_target(cr));
 
    // Draw horizontal line (sky blue) on top of screen
    cairo_set_source_rgb(cr, 0.529, 0.808, 0.922);
    cairo_set_line_width(cr, 150); 
    cairo_move_to(cr, 0, 0); 
    cairo_line_to(cr, SCREEN_WIDTH, 0); 
    cairo_stroke(cr);
 
    // Draw gray vertical line that delineates waveforms from stats
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, WAVEFORM_END, 81);
    cairo_line_to(cr, WAVEFORM_END, SCREEN_HEIGHT);
    cairo_stroke(cr);
 
    // Draw gray horizontal line on bottom of screen
    cairo_move_to(cr, WAVEFORM_END, SCREEN_HEIGHT-100);
    cairo_line_to(cr, SCREEN_WIDTH, SCREEN_HEIGHT-100);
    cairo_stroke(cr);
 
    // Patient name
    draw_label(cr, "Patient: Mr. Sick", 40, 50, 0, 0, 0, 30);

    // Draw pulse value
    char text_buf[64];
    snprintf(text_buf, sizeof(text_buf), "%d", pulse_value);
    draw_label(cr, text_buf, SCREEN_WIDTH - TEXT_COLUMN_2_OFFSET, 235, 1, 0, 0, 125);
    draw_label(cr, "Pulse", SCREEN_WIDTH - TEXT_COLUMN_2_OFFSET, 285, 1, 1, 0, 30);

    // Draw temperature value
    snprintf(text_buf, sizeof(text_buf), "%.1f°", body_temperature);
    draw_label(cr, text_buf, SCREEN_WIDTH - TEXT_COLUMN_3_OFFSET, 235, 1, 0.5, 0, 125);
    draw_label(cr, "Temperature", SCREEN_WIDTH - TEXT_COLUMN_3_OFFSET, 285, 1, 1, 0, 30);

    // Draw blood pressure value
    snprintf(text_buf, sizeof(text_buf), "%d", systolic_bp);
    draw_label(cr, text_buf, SCREEN_WIDTH - TEXT_COLUMN_2_OFFSET, 540, 0, 1, 0, 125);
    draw_label(cr, "____", SCREEN_WIDTH - TEXT_COLUMN_2_OFFSET, 540, 0, 1, 0, 125);
    snprintf(text_buf, sizeof(text_buf), "%d", diastolic_bp);
    draw_label(cr, text_buf, SCREEN_WIDTH - TEXT_COLUMN_2_OFFSET + 40, 680, 0, 1, 0, 125);
    draw_label(cr, "Blood Pressure", SCREEN_WIDTH - TEXT_COLUMN_2_OFFSET, 730, 1, 1, 0, 30);
 
    // Draw oximeter value
    snprintf(text_buf, sizeof(text_buf), "%d", oximeter_percentage);
    draw_label(cr, text_buf, SCREEN_WIDTH - TEXT_COLUMN_3_OFFSET, 585, 0, 1, 0.5, 125);
    draw_label(cr, "Oximeter", SCREEN_WIDTH - TEXT_COLUMN_3_OFFSET, 625, 1, 1, 0, 30);

    // Draw respiratory value
    snprintf(text_buf, sizeof(text_buf), "%d", respiratory_rate_value);
    draw_label(cr, text_buf, SCREEN_WIDTH - TEXT_COLUMN_2_OFFSET, 880, 0, 0, 1, 125);
    draw_label(cr, "RESP(bpm)", SCREEN_WIDTH - TEXT_COLUMN_2_OFFSET, 920, 1, 1, 0, 30);

    // Draw CO2 level value
    snprintf(text_buf, sizeof(text_buf), "%d", co2_level);
    draw_label(cr, text_buf, SCREEN_WIDTH - TEXT_COLUMN_3_OFFSET, 880, 0.5, 0, 1, 125);
    draw_label(cr, "CO2(mmHg)", SCREEN_WIDTH - TEXT_COLUMN_3_OFFSET, 920, 1, 1, 0, 30);
    
    // Draw time
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
    draw_label(cr, time_buf, SCREEN_WIDTH - TEXT_COLUMN_2_OFFSET, 1030, 1, 1, 1, 30);

    // Draw wave labels
    draw_label(cr, "EKG", TEXT_COLUMN_1_OFFSET, 350, 1, 0, 0, 30);
    draw_label(cr, "Pleth", TEXT_COLUMN_1_OFFSET, 705, 0, 1, 0, 30);
    draw_label(cr, "RESP", TEXT_COLUMN_1_OFFSET, 990, 0, 0, 1, 30);

    // Draw waveforms
    draw_waveform(cr,  wave_buffer_ekg,   current_x_ekg,   EKG_Y_OFFSET,   AMPLITUDE, 1.0, 0.0, 0.0);
    draw_waveform(cr2, wave_buffer_pleth, current_x_pleth, PLETH_Y_OFFSET, AMPLITUDE, 0.0, 1.0, 0.0);
    draw_waveform(cr3, wave_buffer_resp,  current_x_resp,  RESP_Y_OFFSET,  AMPLITUDE, 0.0, 0.0, 1.0);

    // Update the wave positions
    update_wave_position(&current_x_ekg, SCROLL_SPEED_EKG);
    update_wave_position(&current_x_pleth, SCROLL_SPEED_PLETH);
    update_wave_position(&current_x_resp, SCROLL_SPEED_RESP);

    cairo_destroy(cr2);
    cairo_destroy(cr3);
}

// GTK application activation callback
void activate(GtkApplication *app, gpointer user_data) {

    GtkWidget *window3 = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(window3), SCREEN_WIDTH, SCREEN_HEIGHT);
    gtk_window_set_title(GTK_WINDOW(window3), "Patient Vitals");
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_window_set_child(GTK_WINDOW(window3), drawing_area);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_callback, NULL, NULL);
    // gtk_widget_show(window3);
    gtk_widget_set_visible(window3, TRUE);
 
    g_timeout_add(25, update_animation, drawing_area); // Smaller number = faster speed of wave
}

// Fills waveform buffer by repeating a pattern
void fill_wave_buffer(double *buffer, const double *pattern, int pattern_len) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = pattern[i % pattern_len];
    }
}
 
// Application entry point
int main(int argc, char *argv[]) {

    fill_wave_buffer(wave_buffer_ekg, pulse_pattern_ekg, G_N_ELEMENTS(pulse_pattern_ekg));
    fill_wave_buffer(wave_buffer_pleth, pulse_pattern_pleth, G_N_ELEMENTS(pulse_pattern_pleth));
    fill_wave_buffer(wave_buffer_resp, pulse_pattern_resp, G_N_ELEMENTS(pulse_pattern_resp));

    // GtkApplication *app = gtk_application_new("com.BlackBerry", G_APPLICATION_FLAGS_NONE);
    GtkApplication *app = gtk_application_new("com.BlackBerry", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
 
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app); 
    return status;
}