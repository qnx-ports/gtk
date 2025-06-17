#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
 
#define AMPLITUDE 21 //changes height of wave
#define AMPLITUDE2 25 //changes height of wave
#define FREQUENCY 2 //more periods in wave if increased
#define DISTANCE 200 
#define BUFFER_SIZE 1000 //size of the buffer array
#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080

// EKG, Pleth, RESP
// Offset from left side of the screen
#define TEXT_COLUMN_1_OFFSET 20
// Offset from right side of the screen
#define TEXT_COLUMN_2_OFFSET 690
// Offset from right side of the screen
#define TEXT_COLUMN_3_OFFSET 360
// Where the waveforms end and the stats text begins
// Offset from left side of the screen
#define WAVEFORM_END 1180
#define WAVEFORM_WIDTH (WAVEFORM_END-1)
 
double wave_buffer[BUFFER_SIZE] = {0.0}; //buffer to store wave values
double wave_buffer2[BUFFER_SIZE] = {0.0};
double wave_buffer3[BUFFER_SIZE] = {0.0};
 
double current_x=WAVEFORM_WIDTH;
double current_x2=WAVEFORM_WIDTH;
double current_x3=WAVEFORM_WIDTH;
 
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
 
//double pulse_pattern[25] = {1.0, 1.0, 0.5, 0.5, 1.0, 1.0, 1.5, 1.5, 1.5, 1.5, 2.0, 2.0, -5.0, 3.0, 1.5, 1.5, 1.5, 1.5, 0.25, 0.25, 0.25, 1.25, 1.75, 1.75, 1.75};
double pulse_pattern[25] = {1.75, 1.75, 1.75, 1.25, 0.25, 0.25, 0.25, 1.5, 1.5, 1.5, 1.5, 3.0, -5.0, 2.0, 2.0, 1.5, 1.5, 1.5, 1.5, 1.0, 1.0, 0.5, 0.5, 1.0, 1.0};
double pulse_pattern2[25] = {-5.0, -6.0, -7.0, -8.0, -9.0, -9.25, -9.0, -5.0, -4.0, -2.0, -1.0, 0.0, 0.25, 0.0, -0.25, -0.5, -1.0, -2.0, -3.0, -3.5, -4.0, -4.5, -5.0, -5.0, -5.0};
double pulse_pattern3[25] = {6.0, 6.0, 6.0, 5.5, 5.0, 4.75, 4.0, 3.5, 3.0, 2.75, 2, 1.5, 1.25, 1.0, 1.0, 1.0, 1.25, 1.5, 2.0, 2.75, 3.0, 3.5, 4.0, 5.0, 5.75};
 
gboolean animation_active = FALSE;
 
static gboolean update_animation(gpointer user_data) {
    GtkWidget *drawing_area = user_data;
 
    static int counter = 0;
    if(counter%40==0){ //40 * 25 milliseconds (from timer delay) = 1000 milliseconds = 1 second
        int pulse_change = rand() % (2*max_pulse_change + 1) - max_pulse_change;
        pulse_value += pulse_change;
        if(pulse_value > 120){
            pulse_value = 120;
        }
        else if(pulse_value < 50){
            pulse_value = 50;
        }
    }
    counter++;
    static int counter2 = 0;
    if (counter2 % 40 == 0) { 
        int respiratory_change = rand() % (2 * max_respiratory_change + 1) - max_respiratory_change;
        respiratory_rate_value += respiratory_change;
        if (respiratory_rate_value > 23) {
            respiratory_rate_value = 23; 
        } else if (respiratory_rate_value < 19) {
            respiratory_rate_value = 19; 
        }
    }
    counter2++;
 
    static int counter3 = 0;
    if (counter3 % 40 == 0) { 
        int oximeter_change = rand() % (2 * max_oximeter_change + 1) - max_oximeter_change;
        oximeter_percentage += oximeter_change;
        if (oximeter_percentage > 100) {
            oximeter_percentage = 100; 
        } else if (oximeter_percentage < 91) {
            oximeter_percentage = 91;  
        }
    }
    counter3++;
    static int counter4 = 0;
    if (counter4 % 40 == 0) { 
        int systolic_change = rand() % (2 * max_bp_change + 1) - max_bp_change;
        int diastolic_change = rand() % (2 * max_bp_change + 1) - max_bp_change;
 
        systolic_bp += systolic_change;
        if (systolic_bp > 140) {
            systolic_bp = 140; 
        } else if (systolic_bp < 120) {
            systolic_bp = 120; 
        }
 
        diastolic_bp += diastolic_change;
        if (diastolic_bp > 90) {
            diastolic_bp = 90; 
        } else if (diastolic_bp < 80) {
            diastolic_bp = 80;  
        }
    }
    counter4++;
 
    static int counter5 = 0;
    if (counter5 % 40 == 0) { 
        float temp_change = ((float)rand() / RAND_MAX) * 2 * max_temp_change - max_temp_change;
        body_temperature += temp_change;
        if (body_temperature > 99.9) {
            body_temperature = 99.9; 
        } else if (body_temperature < 97) {
            body_temperature = 97;  
        }
    }
    counter5++;
 
    static int counter6 = 0;
    if (counter6 % 40 == 0) { 
        int co2_change = rand() % (2 * max_co2_change + 1) - max_co2_change;
        co2_level += co2_change;
        if (co2_level > 45) {
            co2_level = 45; 
        } else if (co2_level < 35) {
            co2_level = 35;  
        }
    }
    counter6++;
 
    gtk_widget_queue_draw(drawing_area);
    return G_SOURCE_CONTINUE;
}
 
static void draw_callback(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer data) {
    cairo_set_source_rgb(cr, 0, 0, 0); //black background
    cairo_paint(cr);
 
    cairo_t *cr2 = cairo_create(cairo_get_target(cr));
    cairo_t *cr3 = cairo_create(cairo_get_target(cr));
 
    //Draw horizontal line (sky blue) on top of screen
    cairo_set_source_rgb(cr, 0.529, 0.808, 0.922);
    cairo_set_line_width(cr, 150); 
    cairo_move_to(cr, 0, 0); 
    cairo_line_to(cr, SCREEN_WIDTH, 0); 
    cairo_stroke(cr);
 
    //Draw gray vertical line that delineates waveforms from stats
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, WAVEFORM_END, 81);
    cairo_line_to(cr, WAVEFORM_END, SCREEN_HEIGHT);
    cairo_stroke(cr);
 
    //Draw gray horizontal line on bottom of screen
    cairo_move_to(cr, WAVEFORM_END, SCREEN_HEIGHT-100);
    cairo_line_to(cr, SCREEN_WIDTH, SCREEN_HEIGHT-100);
    cairo_stroke(cr);
 
 
    //Draw pulse value
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_2_OFFSET, 235);
    cairo_set_font_size(cr, 125);
    char pulse_text[20]; //buffer to hold text
    snprintf(pulse_text, sizeof(pulse_text), "%d", pulse_value);
    cairo_show_text(cr, pulse_text);
    cairo_set_source_rgb(cr, 1, 1, 0);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_2_OFFSET, 285);
    cairo_set_font_size(cr, 30);
    cairo_show_text(cr, "Pulse");
 
    //Draw respiratory value
    cairo_set_source_rgb(cr, 0, 0, 1);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_2_OFFSET, 880);
    cairo_set_font_size(cr, 125);
    char respiratory_text[30]; //buffer to hold text
    snprintf(respiratory_text, sizeof(respiratory_text), "%d", respiratory_rate_value);
    cairo_show_text(cr, respiratory_text);
    cairo_set_source_rgb(cr, 1, 1, 0);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_2_OFFSET, 920);
    cairo_set_font_size(cr, 30);
    cairo_show_text(cr, "RESP(bpm)");
 
    //Draw oximeter value
    cairo_set_source_rgb(cr, 0, 1, 0.5);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_3_OFFSET, 585);
    cairo_set_font_size(cr, 125);
    char oximeter_text[30]; //buffer to hold text
    snprintf(oximeter_text, sizeof(oximeter_text), "%d", oximeter_percentage);
    cairo_show_text(cr, oximeter_text);
    cairo_set_source_rgb(cr, 1, 1, 0);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_3_OFFSET, 625);
    cairo_set_font_size(cr, 30);
    cairo_show_text(cr, "Oximeter");
 
    //Draw blood pressure value
    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_2_OFFSET, 540);
    cairo_set_font_size(cr, 125);
    char bp_text[50]; //buffer to hold text
    snprintf(bp_text, sizeof(bp_text), "%d", systolic_bp);
    cairo_show_text(cr, bp_text);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_2_OFFSET, 540);
    snprintf(bp_text, sizeof(bp_text), "____", diastolic_bp);
    cairo_show_text(cr, bp_text);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_2_OFFSET+40, 680);
    snprintf(bp_text, sizeof(bp_text), "%d", diastolic_bp);
    cairo_show_text(cr, bp_text);
    cairo_set_source_rgb(cr, 1, 1, 0);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_2_OFFSET, 730);
    cairo_set_font_size(cr, 30);
    cairo_show_text(cr, "Blood Pressure");
 
    //Draw temperature value
    cairo_set_source_rgb(cr, 1, 0.5, 0);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_3_OFFSET, 235);
    cairo_set_font_size(cr, 125);
    char temp_text[50]; //buffer to hold text
    snprintf(temp_text, sizeof(temp_text), "%.1f°", body_temperature);
    cairo_show_text(cr, temp_text);
    cairo_set_source_rgb(cr, 1, 1, 0);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_3_OFFSET, 285);
    cairo_set_font_size(cr, 30);
    cairo_show_text(cr, "Temperature");

    // Patient name 
    cairo_set_source_rgb(cr, 0, 0, 0); 
    cairo_move_to(cr, 40, 50); 
    cairo_set_font_size(cr, 30);
    cairo_show_text(cr, "Patient: Mr. Sick" );
    //Draw time
    time_t current_time;
    time(&current_time);
 
    struct tm *time_info;
    time_info = localtime(&current_time);
 
    char time_string[50];
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", time_info);
 
    cairo_set_source_rgb(cr, 1, 1, 1); 
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_2_OFFSET, 1030); 
    cairo_set_font_size(cr, 30);
    cairo_show_text(cr, time_string);
 
    //Draw CO2 level value
    cairo_set_source_rgb(cr, 0.5, 0, 1);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_3_OFFSET, 880);
    cairo_set_font_size(cr, 125);
    char co2_text[50]; //buffer to hold text
    snprintf(co2_text, sizeof(co2_text), "%d", co2_level);
    cairo_show_text(cr, co2_text);
    cairo_set_source_rgb(cr, 1, 1, 0);
    cairo_move_to(cr, SCREEN_WIDTH-TEXT_COLUMN_3_OFFSET, 920);
    cairo_set_font_size(cr, 30);
    cairo_show_text(cr, "CO2(mmHg)");

    // Draw first wave (EKG) label
    cairo_set_source_rgb(cr, 1, 0, 0); // Red
    cairo_move_to(cr, TEXT_COLUMN_1_OFFSET, 350);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 30);
    cairo_show_text(cr, "EKG");
 
    // Draw second wave (Pleth) label
    cairo_set_source_rgb(cr, 0, 1, 0); // Green
    cairo_move_to(cr, TEXT_COLUMN_1_OFFSET, 705);
    cairo_show_text(cr, "Pleth");
 
    // Draw third wave (RESP) label
    cairo_set_source_rgb(cr, 0, 0, 1); // Blue
    cairo_move_to(cr, TEXT_COLUMN_1_OFFSET, 990); //950);
    cairo_show_text(cr, "RESP");
    
    //EKG (heartbeat) wave
    double wave_start_x = fmod(current_x, BUFFER_SIZE);
    cairo_set_source_rgb(cr, 1, 0, 0); //red line
    cairo_set_line_width(cr, 3); //thickness of the wave
    for (int i = 0; i < DISTANCE; i++) {  //increasing DISTANCE increases width of wave
        int buffer_idx = (int)wave_start_x + i;
        double x = WAVEFORM_WIDTH - i * (double)WAVEFORM_WIDTH / DISTANCE;
        double y = (SCREEN_HEIGHT-600) / 2 + AMPLITUDE * wave_buffer[buffer_idx % BUFFER_SIZE];
        if (i == 0) {
            cairo_move_to(cr, x, y);
        } else {
            cairo_line_to(cr, x, y);
        }
    }
    current_x -= 0.7; //smaller value slows down speed of wave
    if(current_x<=0){
        current_x = WAVEFORM_WIDTH;
    }
 
    //Pleth (blood flow) wave
    double wave_start_x2 = fmod(current_x2, BUFFER_SIZE);
    cairo_set_source_rgb(cr2, 0, 1, 0); //green line
    cairo_set_line_width(cr2, 3); //thickness of the wave
    for (int j = 0; j < DISTANCE; j++) {  //increasing DISTANCE increases width of wave
        int buffer_idx2 = (int)wave_start_x2 + j;
        double x2 = WAVEFORM_WIDTH - j * (double)WAVEFORM_WIDTH / DISTANCE;
       // double y2 = (SCREEN_HEIGHT-80) / 2 + AMPLITUDE * wave_buffer2[buffer_idx2 % BUFFER_SIZE];
    //    double y2 = (SCREEN_HEIGHT+170) / 2 + AMPLITUDE * wave_buffer2[buffer_idx2 % BUFFER_SIZE];
       double y2 = (SCREEN_HEIGHT+220) / 2 + AMPLITUDE * wave_buffer2[buffer_idx2 % BUFFER_SIZE];
        if (j == 0) {
            cairo_move_to(cr2, x2, y2);
        } else {
            cairo_line_to(cr2, x2, y2);
        }
    }
    current_x2 -= 0.95; //smaller value slows down speed of wave
    if(current_x2<=0){
        current_x2 = WAVEFORM_WIDTH;
    }
 
    //RESP (breathing) wave
    double wave_start_x3 = fmod(current_x3, BUFFER_SIZE);
    cairo_set_source_rgb(cr3, 0, 0, 1); //blue line
    cairo_set_line_width(cr3, 3); //thickness of the wave
    for (int k = 0; k < DISTANCE; k++) {  //increasing DISTANCE increases width of wave
        int buffer_idx3 = (int)wave_start_x3 + k;
        double x3 = WAVEFORM_WIDTH - k * (double)WAVEFORM_WIDTH / DISTANCE;
        // double y3 = (SCREEN_HEIGHT+485) / 2 + AMPLITUDE * wave_buffer3[buffer_idx3 % BUFFER_SIZE];
        double y3 = (SCREEN_HEIGHT+550) / 2 + AMPLITUDE * wave_buffer3[buffer_idx3 % BUFFER_SIZE];
        if (k == 0) {
            cairo_move_to(cr3, x3, y3);
        } else {
            cairo_line_to(cr3, x3, y3);
        }
    }
    current_x3 -= 0.45; //smaller value slows down speed of wave
    if(current_x3<=0){
        current_x3 = WAVEFORM_WIDTH;
    }
 
    //draws the three waves to the screen
    cairo_stroke(cr);
    cairo_stroke(cr2);
    cairo_stroke(cr3);
 
    cairo_destroy(cr2);
    cairo_destroy(cr3);
}
 
 
void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window3 = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(window3), SCREEN_WIDTH, SCREEN_HEIGHT);
    gtk_window_set_title(GTK_WINDOW(window3), "Patient Vitals");
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_drawing_area_set_content_width;
    gtk_window_set_child(GTK_WINDOW(window3), drawing_area);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_callback, NULL, NULL);
    gtk_widget_show(window3);
 
    animation_active = TRUE;
    g_timeout_add(25, update_animation, drawing_area); //smaller number = faster speed of wave
}
 
 
int main(int argc, char *argv[]) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        wave_buffer[i] = pulse_pattern[i%25];
    }
    for (int j = 0; j < BUFFER_SIZE; j++) {
        wave_buffer2[j] = pulse_pattern2[j%25];
    }
    for (int k = 0; k < BUFFER_SIZE; k++) {
        wave_buffer3[k] = pulse_pattern3[k%25];
    }
 
    GtkApplication *app = gtk_application_new("com.BlackBerry", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
 
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
 
    return status;
}


