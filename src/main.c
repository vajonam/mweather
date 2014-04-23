#include <pebble.h>
#include "network.h"
#include "weather_layer.h"
#include "debug_layer.h"
#include "battery_layer.h"
#include "datetime_layer.h"
#include "config.h"

#define TIME_FRAME      (GRect(0, 3, 144, 168-6))
#define DATE_FRAME      (GRect(1, 66, 144, 168-62))
#define WEATHER_FRAME   (GRect(0, 90, 144, 80))
#define DEBUG_FRAME     (GRect(0, 168-15, 144, 15))
#define BATTERY_FRAME   (GRect(110, 0, 144, 8))

/* Keep a pointer to the current weather data as a global variable */
static WeatherData *weather_data;

/* Global variables to keep track of the UI elements */
static Window *window;

/* Need to wait for JS to be ready */
static bool initial_request = true;
static int  initial_count   = 0;
const  int  MAX_JS_READY_WAIT_SECS = 5;

static void handle_tick(struct tm *tick_time, TimeUnits units_changed)
{
  if (units_changed & MINUTE_UNIT) {
    time_layer_update();
  }

  if (units_changed & DAY_UNIT) {
    date_layer_update(tick_time);
  }

  weather_layer_update(weather_data);
  debug_layer_update(weather_data);
  
  // Refresh the weather info every half hour, at 18 and 48 mins after the hour (Yahoo updates around then)
  if ((units_changed & MINUTE_UNIT) && 
      (tick_time->tm_min == 18 || tick_time->tm_min == 48) &&
      initial_request == false) {
    request_weather(weather_data);
  }

  // Wait for JS to tell us it's ready before firing the first request
  if (initial_request || weather_data->js_ready) {

    // We're all set!
    if (weather_data->js_ready) {
      initial_request = false;
      weather_data->js_ready = false;
      request_weather(weather_data);
    } 
    else {
      initial_count++;
      // If we've waited too long let's try anyways. This will inform the user via error icon if it fails
      if (initial_count >= MAX_JS_READY_WAIT_SECS) {
        initial_request = false;
        weather_data->js_ready = true; // let's assume we just missed it, and if we're wrong it's ok
      }
    }
  }
} 

void load_persisted_values() 
{
  // Debug
  weather_data->debug = persist_exists(KEY_DEBUG_MODE) ? persist_read_bool(KEY_DEBUG_MODE) : DEFAULT_DEBUG_MODE;

  // Battery
  weather_data->battery = persist_exists(KEY_DISPLAY_BATTERY) ? persist_read_bool(KEY_DISPLAY_BATTERY) : DEFAULT_DISPLAY_BATTERY;

  if (weather_data->battery) {
    battery_enable_display();
  } else {
    battery_disable_display(true);
  }
  
  // Weather Service
  static char service[10];
  if (persist_exists(KEY_WEATHER_SERVICE)) {
    persist_read_string(KEY_WEATHER_SERVICE, service, sizeof(service));
  } else {
    strcpy(service, DEFAULT_WEATHER_SERVICE);
  }
  weather_data->service = service;

  // Weather Scale
  static char scale[5];
  if (persist_exists(KEY_WEATHER_SCALE)) {
    persist_read_string(KEY_WEATHER_SCALE, scale, sizeof(scale));
  } else {
    strcpy(scale, DEFAULT_WEATHER_SCALE);
  }
  weather_data->scale = scale;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "PersistLoad:  d:%d b:%d s:%s u:%s", 
      weather_data->debug, weather_data->battery, weather_data->service, weather_data->scale);
}

void store_persistant_values() 
{
  persist_write_bool(KEY_DEBUG_MODE, weather_data->debug);
  persist_write_bool(KEY_DISPLAY_BATTERY, weather_data->battery);
  persist_write_string(KEY_WEATHER_SERVICE, weather_data->service);
  persist_write_string(KEY_WEATHER_SCALE, weather_data->scale);


  APP_LOG(APP_LOG_LEVEL_DEBUG, "PersistStore:  d:%d b:%d s:%s u:%s", 
      weather_data->debug, weather_data->battery, weather_data->service, weather_data->scale);
}

static void init(void) {

  APP_LOG(APP_LOG_LEVEL_DEBUG, "init started");

  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  weather_data = malloc(sizeof(WeatherData));
  init_network(weather_data);

  // Setup our layers
  time_layer_create(TIME_FRAME, window);
  date_layer_create(DATE_FRAME, window);
  weather_layer_create(WEATHER_FRAME, window);
  debug_layer_create(DEBUG_FRAME, window);
  battery_layer_create(BATTERY_FRAME, window);

  // Load persisted values
  load_persisted_values();

  // Update the screen right away
  time_t now = time(NULL);
  handle_tick(localtime(&now), SECOND_UNIT | MINUTE_UNIT | HOUR_UNIT | DAY_UNIT );

  // And then every second
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
}

static void deinit(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "deinit started");

  tick_timer_service_unsubscribe();

  window_destroy(window);

  time_layer_destroy();
  date_layer_destroy();
  weather_layer_destroy();
  debug_layer_destroy();
  battery_layer_destroy();

  free(weather_data);

  close_network();

  store_persistant_values();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
