#pragma once
  
#define NUM_BUSES_MAX (8)

typedef struct {
  int id;
  char dest[64];
  int min1;
  int min2;
} Bus;