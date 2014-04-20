#ifndef DEBUG_LAYER_H
#define DEBUG_LAYER_H

TextLayer *debug_layer_create(GRect frame);
void debug_layer_update(time_t currentTime, WeatherData *weather_data);
void debug_layer_destroy();

#endif