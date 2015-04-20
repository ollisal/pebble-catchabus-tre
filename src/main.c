#include <pebble.h>

#include "common.h"
#include "view.h"
  
#define KEY_STOP_NAME (0)
#define KEY_BUSES (1)

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  view_show_time(tick_time);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Update time
  update_time();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  char stop_name[24];
  Bus buses[NUM_BUSES_MAX];
  int num_buses = 0;

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_STOP_NAME:
      strncpy(stop_name, t->value->cstring, sizeof(stop_name));
      break;

    case KEY_BUSES:
      for (const char *row = t->value->cstring; row && *row && num_buses < NUM_BUSES_MAX; row = (strchr(row, '\n') + 1)) {
        char row_buf[256] = { '\0' };
        strncpy(row_buf, row, sizeof(row_buf));
        
        Bus *bus = &buses[num_buses];
        
        char *c1 = strchr(row_buf, ','); *c1 = '\0';
        char *c2 = strchr(c1 + 1, ','); *c2 = '\0';
        char *c3 = strchr(c2 + 1, ','); *c3 = '\0';
        char *nl = strchr(c3 + 1, '\n'); *nl = '\0';

        bus->id = atoi(row_buf);
        strncpy(bus->dest, c1 + 1, sizeof(bus->dest));
        bus->min1 = atoi(c2 + 1);
        bus->min2 = atoi(c3 + 1);
        
        num_buses++;
      }
      
      break;

    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }

  view_show_buses(stop_name, buses, num_buses);
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