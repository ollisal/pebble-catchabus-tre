#include <pebble.h>

#include "common.h"
#include "view.h"
  
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  view_show_time(tick_time);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  //static int temperature = 0;
  //static char conditions[32] = { 0 };

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      //temperature = (int) t->value->int32;
      break;
    case KEY_CONDITIONS:
      //snprintf(conditions, sizeof(conditions), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // TODO get bus info from phone
  static Bus s_buses[] = {
    { 13, "Ylöjärvi Matkatie", 8, 25},
    { 20, "Pyynikintori", 3, 32}
  };
  view_show_buses("Hermiankatu 7", s_buses, 2);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // Init view
  view_init();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // Make sure the time is displayed from the start
  update_time();
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  view_deinit();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}