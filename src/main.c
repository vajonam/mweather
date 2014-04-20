#include <pebble.h>

#include "network.h"
#include "weather_layer.h"
#include "debug_layer.h"
#include "config.h"

#define TIME_FRAME      (GRect(0, 2, 144, 168-6))
#define DATE_FRAME      (GRect(1, 66, 144, 168-62))
#define WEATHER_FRAME   (GRect(0, 90, 144, 80))
#define DEBUG_FRAME     (GRect(0, 168-15, 144, 15))

/* Keep a pointer to the current weather data as a global variable */
static WeatherData *weather_data;

/* Global variables to keep track of the UI elements */
static Window *window;
static TextLayer *date_layer;
static TextLayer *time_layer;

static char date_text[] = "XXX 00";
static char time_text[] = "00:00";

/* Preload the fonts */
GFont font_date;
GFont font_time;

/* Need to wait for JS to be ready */
static bool initial_request = true;

static void handle_tick(struct tm *tick_time, TimeUnits units_changed)
{

  time_t currentTime = time(NULL);
  if (units_changed & MINUTE_UNIT) {

    // Update the time - Fix to deal with 12 / 24 centering bug
    struct tm *currentLocalTime = localtime(&currentTime);

    // Manually format the time as 12 / 24 hour, as specified
    strftime(   time_text, 
                sizeof(time_text), 
                clock_is_24h_style() ? "%R" : "%I:%M", 
                currentLocalTime);

    // Drop the first char of time_text if needed
    if (!clock_is_24h_style() && (time_text[0] == '0')) {
      memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    }

    text_layer_set_text(time_layer, time_text);
  }

  if (units_changed & DAY_UNIT) {
    // Update the date - Without a leading 0 on the day of the month
    char day_text[4];
    strftime(day_text, sizeof(day_text), "%a", tick_time);
    snprintf(date_text, sizeof(date_text), "%s %i", day_text, tick_time->tm_mday);
    text_layer_set_text(date_layer, date_text);
  }

  weather_layer_update(currentTime, weather_data);
  debug_layer_update(currentTime, weather_data);

  // Refresh the weather info every half hour, at 18 and 48 mins after the hour (Yahoo updates around then)
  if ((units_changed & MINUTE_UNIT) && 
      (tick_time->tm_min == 18 || tick_time->tm_min == 48) &&
      initial_request == false) {
    request_weather(weather_data);
  }

  // Wait for JS to tell us it's ready before firing the first request
  if (weather_data->js_ready && initial_request) {
    initial_request = false;
    request_weather(weather_data);
  } 
}

static void init(void) {

  APP_LOG(APP_LOG_LEVEL_DEBUG, "init started");

  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  weather_data = malloc(sizeof(WeatherData));
  init_network(weather_data);

  font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUTURA_18));
  font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUTURA_CONDENSED_53));

  time_layer = text_layer_create(TIME_FRAME);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, font_time);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));

  date_layer = text_layer_create(DATE_FRAME);
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, font_date);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));

  layer_add_child(window_get_root_layer(window), weather_layer_create(WEATHER_FRAME));

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(debug_layer_create(DEBUG_FRAME)));

  // Load persisted values
  weather_data->debug = persist_exists(KEY_DEBUG_MODE) ? persist_read_bool(KEY_DEBUG_MODE) : DEFAULT_DEBUG_MODE;
    
  char service[20];
  if (persist_exists(KEY_WEATHER_SERVICE)) {
    persist_read_string(KEY_WEATHER_SERVICE, service, sizeof(service));
  } else {
    snprintf(service, sizeof(service), "%s", DEFAULT_WEATHER_SERVICE);
  }
  weather_data->service = service;

  // Update the screen right away
  time_t now = time(NULL);
  handle_tick(localtime(&now), SECOND_UNIT | MINUTE_UNIT | HOUR_UNIT | DAY_UNIT );

  // And then every second
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
}

static void deinit(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "deinit started");

  window_destroy(window);
  tick_timer_service_unsubscribe();

  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);

  weather_layer_destroy();
  debug_layer_destroy();

  fonts_unload_custom_font(font_date);
  fonts_unload_custom_font(font_time);

  free(weather_data);

  persist_write_bool(KEY_DEBUG_MODE, weather_data->debug);
  persist_write_string(KEY_WEATHER_SERVICE, weather_data->service);

  close_network();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
