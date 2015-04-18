#pragma once

#include <pebble.h>

#include "common.h"

void view_init();
void view_deinit();

void view_show_time(struct tm *t);
void view_show_buses(const char *stop_name, const Bus *buses, int num_buses);