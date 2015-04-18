#include "view.h"

#include "common.h"

#define SCREEN_WIDTH (144)
#define SCREEN_HEIGHT (168)

#define LINE_HEIGHT (16)
  
#define COL1_WIDTH (106)
#define COL2_WIDTH (SCREEN_WIDTH - COL1_WIDTH)
  
static Window *s_main_window = NULL;
static GFont s_cs14_font;
static GFont s_cs16_font;

static TextLayer *s_time_layer;
static TextLayer *s_stop_layer;
static TextLayer *s_bus_name_layers[NUM_BUSES_MAX];
static TextLayer *s_bus_eta_layers[NUM_BUSES_MAX];

static void main_window_load(Window *window) {
  // Load fonts
  s_cs14_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_COMICSANS_14));
  s_cs16_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_COMICSANS_16));

  // Background
  window_set_background_color(window, GColorBlack);
  
  // Stop/time row
  s_stop_layer = text_layer_create(GRect(0, 0, COL1_WIDTH, LINE_HEIGHT));
  text_layer_set_text_alignment(s_stop_layer, GTextAlignmentLeft);
  text_layer_set_background_color(s_stop_layer, GColorWhite);
  text_layer_set_text_color(s_stop_layer, GColorBlack);
  text_layer_set_font(s_stop_layer, s_cs14_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_stop_layer));

  text_layer_set_text(s_stop_layer, "Loading...");
  
  s_time_layer = text_layer_create(GRect(COL1_WIDTH, 0, COL2_WIDTH, LINE_HEIGHT));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);
  text_layer_set_background_color(s_time_layer, GColorWhite);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, s_cs14_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Bus rows
  for (int i = 0; i < NUM_BUSES_MAX; i++) {
    s_bus_name_layers[i] = text_layer_create(GRect(0, (i + 1) * LINE_HEIGHT, COL1_WIDTH, LINE_HEIGHT));
    text_layer_set_text_alignment(s_bus_name_layers[i], GTextAlignmentLeft);
    text_layer_set_background_color(s_bus_name_layers[i], GColorClear);
    text_layer_set_text_color(s_bus_name_layers[i], GColorWhite);
    text_layer_set_font(s_bus_name_layers[i], s_cs14_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bus_name_layers[i]));
  
    s_bus_eta_layers[i] = text_layer_create(GRect(COL1_WIDTH, (i + 1) * LINE_HEIGHT, COL2_WIDTH, LINE_HEIGHT));
    text_layer_set_text_alignment(s_bus_eta_layers[i], GTextAlignmentRight);
    text_layer_set_background_color(s_bus_eta_layers[i], GColorClear);
    text_layer_set_text_color(s_bus_eta_layers[i], GColorWhite);
    text_layer_set_font(s_bus_eta_layers[i], s_cs14_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bus_eta_layers[i]));
  }
}

static void main_window_unload(Window *window)
{
  // Destroy TextLayers
  text_layer_destroy(s_stop_layer);
  text_layer_destroy(s_time_layer);
  
  for (int i= 0; i < NUM_BUSES_MAX; i++) {
    text_layer_destroy(s_bus_name_layers[i]);
    text_layer_destroy(s_bus_eta_layers[i]);
  }

  // Unload fonts
  fonts_unload_custom_font(s_cs14_font);
  fonts_unload_custom_font(s_cs16_font);
}

void view_init()
{
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
}

void view_deinit()
{
  // Destroy Window
  window_destroy(s_main_window);
}

void view_show_time(struct tm *t)
{
  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", t);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", t);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

void view_show_buses(const char *stop_name, const Bus *buses, int num_buses)
{
  static char stop_name_buffer[24];
  static char bus_name_buffers[NUM_BUSES_MAX][24];
  static char bus_eta_buffers[NUM_BUSES_MAX][6];

  strncpy(stop_name_buffer, stop_name, sizeof(stop_name_buffer));
  text_layer_set_text(s_stop_layer, stop_name_buffer);

  for (int i = 0; i < num_buses; i++) {
    snprintf(bus_name_buffers[i], sizeof(bus_name_buffers[0]), "%d %s", buses[i].id, buses[i].dest);
    text_layer_set_text(s_bus_name_layers[i], bus_name_buffers[i]);
    layer_set_hidden((Layer *) s_bus_name_layers[i], false);
    
    snprintf(bus_eta_buffers[i], sizeof(bus_eta_buffers[0]), "%d %d", buses[i].min1, buses[i].min2);
    text_layer_set_text(s_bus_eta_layers[i], bus_eta_buffers[i]);
    layer_set_hidden((Layer *) s_bus_eta_layers[i], false);
  }
  
  for (int i = num_buses; i < NUM_BUSES_MAX; i++) {
    layer_set_hidden((Layer *) s_bus_name_layers[i], true);
    layer_set_hidden((Layer *) s_bus_eta_layers[i], true);
  }
}