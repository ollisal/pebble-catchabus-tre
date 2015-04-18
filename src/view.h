#pragma once

#include <pebble.h>
  
void view_init();
void view_deinit();

void view_show_time(struct tm *t);
void view_show_weather(int temperature, const char *conditions);