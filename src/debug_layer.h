#ifndef DEBUG_LAYER_H
#define DEBUG_LAYER_H

void debug_layer_create(GRect frame, Window *window);
void debug_layer_update(WeatherData *weather_data);
void debug_layer_destroy();

#endif