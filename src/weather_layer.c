#include <pebble.h>
#include "network.h"
#include "weather_layer.h"
#include "debug_layer.h"

static Layer *weather_layer;

static uint8_t WEATHER_ICONS[] = {
  RESOURCE_ID_ICON_CLEAR_DAY,
  RESOURCE_ID_ICON_CLEAR_NIGHT,
  RESOURCE_ID_ICON_RAIN,
  RESOURCE_ID_ICON_SNOW,
  RESOURCE_ID_ICON_SLEET,
  RESOURCE_ID_ICON_WIND,
  RESOURCE_ID_ICON_FOG,
  RESOURCE_ID_ICON_CLOUDY,
  RESOURCE_ID_ICON_PARTLY_CLOUDY_DAY,
  RESOURCE_ID_ICON_PARTLY_CLOUDY_NIGHT,
  RESOURCE_ID_ICON_THUNDER,
  RESOURCE_ID_ICON_RAIN_SNOW,
  RESOURCE_ID_ICON_RAIN_SLEET,
  RESOURCE_ID_ICON_SNOW_SLEET,
  RESOURCE_ID_ICON_COLD,
  RESOURCE_ID_ICON_HOT,
  RESOURCE_ID_ICON_DRIZZLE,
  RESOURCE_ID_ICON_NOT_AVAILABLE,
  RESOURCE_ID_ICON_PHONE_ERROR,
  RESOURCE_ID_ICON_CLOUD_ERROR,
  RESOURCE_ID_ICON_LOADING1,
  RESOURCE_ID_ICON_LOADING2,
  RESOURCE_ID_ICON_LOADING3,
  RESOURCE_ID_ICON_FAIR_DAY,
  RESOURCE_ID_ICON_FAIR_NIGHT,
  RESOURCE_ID_ICON_RAIN_SUN,
  RESOURCE_ID_ICON_THUNDER_SUN,
};

// Buffer the day / night time switch around sunrise & sunset
const int CIVIL_TWILIGHT_BUFFER = 900; // 15 minutes
const int WEATHER_ANIMATION_REFRESH = 1000; // 1 second animation 
const int WEATHER_ANIMATION_TIMEOUT = 60; // 60 * WEATHER_ANIMATION_REFRESH = 60s
const int WEATHER_STALE_TIMEOUT = 60 * 60 * 2; // 2 hours in seconds

// Keep pointers to the two fonts we use.
static GFont large_font, small_font;

// Why do the work if we're already displaying what we want
static WeatherIcon current_icon;

// Initial animation dots
static AppTimer *weather_animation_timer;
static bool animation_timer_enabled = true;
static int  animation_step = 0;

void weather_animate(void *context)
{
  WeatherData *weather_data = (WeatherData*) context;

  if (weather_data->updated == 0 && weather_data->error == WEATHER_E_OK) {    
    // 'Animate' loading icon until the first successful weather request
    if (animation_step % 3 <= 0) {
      weather_layer_set_icon(WEATHER_ICON_LOADING1);
    }
    else if (animation_step % 3 == 1) {
      weather_layer_set_icon(WEATHER_ICON_LOADING2);
    }
    else if (animation_step % 3 >= 2) {
      weather_layer_set_icon(WEATHER_ICON_LOADING3);
    }
    animation_step = animation_step + 1;
    if (animation_step > WEATHER_ANIMATION_TIMEOUT) {
      weather_data->error = WEATHER_E_NETWORK;
    }
    weather_animation_timer = app_timer_register(WEATHER_ANIMATION_REFRESH, weather_animate, weather_data);
  } 
  else if (weather_data->error != WEATHER_E_OK) {
    animation_step = 0;
    weather_layer_set_icon(WEATHER_ICON_PHONE_ERROR);
  }
}

void weather_layer_create(GRect frame, Window *window)
{
  // Create a new layer with some extra space to save our custom Layer infos
  weather_layer = layer_create_with_data(frame, sizeof(WeatherLayerData));
  WeatherLayerData *wld = layer_get_data(weather_layer);

  large_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUTURA_40));
  small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUTURA_35));

  // Add background layer
  wld->temp_layer_background = text_layer_create(GRect(0, 10, 144, 68));
  text_layer_set_background_color(wld->temp_layer_background, GColorWhite);
  layer_add_child(weather_layer, text_layer_get_layer(wld->temp_layer_background));

  // Add temperature layer
  wld->temp_layer = text_layer_create(GRect(70, 19, 72, 80));
  text_layer_set_background_color(wld->temp_layer, GColorClear);
  text_layer_set_text_alignment(wld->temp_layer, GTextAlignmentCenter);
  text_layer_set_font(wld->temp_layer, large_font);
  layer_add_child(weather_layer, text_layer_get_layer(wld->temp_layer));

  // Add bitmap layer
  wld->icon_layer = bitmap_layer_create(GRect(9, 13, 60, 60));
  layer_add_child(weather_layer, bitmap_layer_get_layer(wld->icon_layer));

  wld->icon = NULL;

  layer_add_child(window_get_root_layer(window), weather_layer);
}

void weather_layer_set_icon(WeatherIcon icon) 
{
  if (current_icon == icon) {
    return;
  }
  current_icon = icon;

  WeatherLayerData *wld = layer_get_data(weather_layer);

  GBitmap *new_icon =  gbitmap_create_with_resource(WEATHER_ICONS[icon]);
  // Display the new bitmap
  bitmap_layer_set_bitmap(wld->icon_layer, new_icon);

  // Destroy the ex-current icon if it existed
  if (wld->icon != NULL) {
    // A cast is needed here to get rid of the const-ness
    gbitmap_destroy(wld->icon);
  }
  wld->icon = new_icon;
}

void weather_layer_clear_temperature()
{
  WeatherLayerData *wld = layer_get_data(weather_layer);
  text_layer_set_text(wld->temp_layer, "");
}

void weather_layer_set_temperature(int16_t t, bool is_stale) 
{
  WeatherLayerData *wld = layer_get_data(weather_layer);

  snprintf(wld->temp_str, sizeof(wld->temp_str), "%i%s", t, is_stale ? " " : "°");

  // Temperature between -9° -> 9° or 20° -> 99°
  if ((t >= -9 && t <= 9) || (t >= 20 && t < 100)) {
    text_layer_set_font(wld->temp_layer, large_font);
    text_layer_set_text_alignment(wld->temp_layer, GTextAlignmentCenter);

  	// Is the temperature below zero?
  	if (wld->temp_str[0] == '-') {
  	  memmove(
            wld->temp_str + 1 + 1,
            wld->temp_str + 1,
            5 - (1 + 1)
        );
  	  memcpy(&wld->temp_str[1], " ", 1);
  	}
  }
  // Temperature between 10° -> 19°
  else if (t >= 10 && t < 20) {
    text_layer_set_font(wld->temp_layer, large_font);
    text_layer_set_text_alignment(wld->temp_layer, GTextAlignmentLeft);
  }
  // Temperature above 99° or below -9°
  else {
    text_layer_set_font(wld->temp_layer, small_font);
    text_layer_set_text_alignment(wld->temp_layer, GTextAlignmentCenter);
  }
  text_layer_set_text(wld->temp_layer, wld->temp_str);
}

// Update the bottom half of the screen: icon and temperature
void weather_layer_update(WeatherData *weather_data) 
{
  // We have no weather data yet... don't update until we do
  if (weather_data->updated == 0) {
    return;
  }

  if (weather_animation_timer && animation_timer_enabled) {
    app_timer_cancel(weather_animation_timer);
    // this is only needed to stop the error message when cancelling an already cancelled timer... 
    animation_timer_enabled = false;
  }

  time_t current_time = time(NULL);
  bool stale = false;
  if (current_time - weather_data->updated > WEATHER_STALE_TIMEOUT) {
    stale = true;
  }

  // APP_LOG(APP_LOG_LEVEL_DEBUG, "ct:%i wup:%i, stale:%i", 
  //   (int)current_time, (int)weather_data->updated, (int)WEATHER_STALE_TIMEOUT);

  // Update the weather icon and temperature
  if (weather_data->error) {
    // Only update the error icon if the weather data is stale
    if (stale) {
      weather_layer_clear_temperature();
      switch (weather_data->error) {
        case WEATHER_E_NETWORK:
          weather_layer_set_icon(WEATHER_ICON_CLOUD_ERROR);
          debug_update_message("Network error");
          break;
        case WEATHER_E_DISCONNECTED:
        case WEATHER_E_PHONE:
        default:
          weather_layer_set_icon(WEATHER_ICON_PHONE_ERROR);
          debug_update_message("Phone disco / error");
          break;
      }
    }
  } else {

    // Show the temperature as 'stale' if it has not been updated in WEATHER_STALE_TIMEOUT
    weather_layer_set_temperature(weather_data->temperature, stale);

    // Day/night check
    time_t utc = current_time + weather_data->tzoffset;
    bool night_time = false;
    if (utc < (weather_data->sunrise - CIVIL_TWILIGHT_BUFFER) || 
        utc > (weather_data->sunset  + CIVIL_TWILIGHT_BUFFER)) {
      night_time = true;
    }

    /*
    APP_LOG(APP_LOG_LEVEL_DEBUG, 
       "ct:%i, utc:%i sr:%i, ss:%i, nt:%i", 
       (int)current_time, (int)utc, weather_data->sunrise, weather_data->sunset, night_time);
    */

    if (strcmp(weather_data->service, SERVICE_OPEN_WEATHER) == 0) {
      weather_layer_set_icon(open_weather_icon_for_condition(weather_data->condition, night_time));
    } else {
      weather_layer_set_icon(yahoo_weather_icon_for_condition(weather_data->condition, night_time));
    }
  }
}

void weather_layer_destroy() 
{
  if (weather_animation_timer && animation_timer_enabled) {
    app_timer_cancel(weather_animation_timer);
    animation_timer_enabled = false;
  }

  WeatherLayerData *wld = layer_get_data(weather_layer);

  text_layer_destroy(wld->temp_layer);
  text_layer_destroy(wld->temp_layer_background);
  bitmap_layer_destroy(wld->icon_layer);

  // Destroy the previous bitmap if we have one
  if (wld->icon != NULL) {
    gbitmap_destroy(wld->icon);
  }
  layer_destroy(weather_layer);

  fonts_unload_custom_font(large_font);
  fonts_unload_custom_font(small_font);
}

/*
 * Converts an API Weather Condition into one of our icons.
 * Refer to: http://bugs.openweathermap.org/projects/api/wiki/Weather_Condition_Codes
 */
uint8_t open_weather_icon_for_condition(int c, bool night_time) 
{
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "In Open Weather icon selection. c: %i, night: %i", c, night_time);

  if (c < 100) {
    return WEATHER_ICON_NOT_AVAILABLE;
  }
  // Thunderstorm
  else if (c < 300) {
    return WEATHER_ICON_THUNDER;
  }
  // Drizzle
  else if (c < 500) {
    return WEATHER_ICON_DRIZZLE;
  }
  // Freezing Rain
  else if (c == 511) {
    return WEATHER_ICON_RAIN_SLEET;
  }
  // Rain / Freezing rain / Shower rain
  else if (c < 600) {
    return WEATHER_ICON_RAIN;
  }
  // Sleet
  else if (c == 611 || c == 612) {
    return WEATHER_ICON_SNOW_SLEET;
  }
  // Rain and snow
  else if (c == 615 || c == 616) {
    return WEATHER_ICON_RAIN_SNOW;
  }
  // Snow
  else if (c < 700) {
    return WEATHER_ICON_SNOW;
  }
  // Fog / Mist / Haze / etc.
  else if (c < 771) {
    return WEATHER_ICON_FOG;
  }
  // Tornado / Squalls
  else if (c < 800) {
    return WEATHER_ICON_WIND;
  }
  // Sky is clear
  else if (c == 800) {
    if (night_time)
      return WEATHER_ICON_CLEAR_NIGHT;
    else
      return WEATHER_ICON_CLEAR_DAY;
  }
    // Few clouds
  else if (c == 801) {
    if (night_time)
      return WEATHER_ICON_FAIR_NIGHT;
    else
      return WEATHER_ICON_FAIR_DAY;
  }
  // few/scattered/broken clouds
  else if (c < 804) {
    if (night_time)
      return WEATHER_ICON_PARTLY_CLOUDY_NIGHT;
    else
      return WEATHER_ICON_PARTLY_CLOUDY_DAY;
  }
  // overcast clouds
  else if (c == 804) {
    return WEATHER_ICON_CLOUDY;
  }
  // Hail
  else if (c == 906) {
    return WEATHER_ICON_SLEET;
  }
  // Extreme
  else if ((c >= 900 && c <= 902) || c == 905 || (c >= 957 && c <= 962)) {
    return WEATHER_ICON_WIND;
  }
  // Cold
  else if (c == 903) {
      return WEATHER_ICON_COLD;
  }
  // Hot
  else if (c == 904) {
      return WEATHER_ICON_HOT;
  }
  // Gentle to strong breeze
  else if (c >= 950 && c <= 956) {
    if (night_time)
      return WEATHER_ICON_FAIR_NIGHT;
    else
      return WEATHER_ICON_FAIR_DAY;
  }
  else {
    // Weather condition not available
    return WEATHER_ICON_NOT_AVAILABLE;
  }
}

/*
 * Converts the Yahoo API Weather Condition into one of our icons.
 * Refer to: https://developer.yahoo.com/weather/#codes
 */
uint8_t yahoo_weather_icon_for_condition(int c, bool night_time) 
{
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "In Yahoo Weather icon selection. c: %i, night: %i", c, night_time);
  
  // Tornado / Hurricane / Wind
  if ((c >= 0 && c <= 2) || c == 23 || c == 24) {
    return WEATHER_ICON_WIND;
  }
  // Thunderstorm
  else if (c == 3 || c == 4 || c == 38 || c == 39) {
    return WEATHER_ICON_THUNDER;
  }
  // Rain & Snow
  else if (c == 5) {
    return WEATHER_ICON_RAIN_SNOW;
  }
  // Rain & Sleet / Mixed 
  else if (c == 6 || c == 8 || c == 10 || c == 35) {
    return WEATHER_ICON_RAIN_SLEET;
  }
  // Snow & Sleet
  else if (c == 7) {
    return WEATHER_ICON_SNOW_SLEET;
  }
  // Drizzle // Showers
  else if (c == 9 || c == 11 || c == 40) {
    return WEATHER_ICON_DRIZZLE;
  }
  // Rain / Scattered Showers / Thundershowers
  else if (c == 12 || c == 45) {
    return WEATHER_ICON_RAIN;
  }
  // Snow / Heavy Snow / Scattered Snow
  else if ((c >= 13 && c <= 16) || (c >= 41 && c <= 43) || c == 46) {
    return WEATHER_ICON_SNOW;
  }
  // Sleet
  else if (c == 17 || c == 18) {
    return WEATHER_ICON_SLEET;
  }
  // Fog / Mist / Haze / etc.
  else if (c >= 19 && c <= 22) {
    return WEATHER_ICON_FOG;
  }
  // Cold
  else if (c == 25) {
    return WEATHER_ICON_COLD;
  }
  // Cloudy, Mostly Cloudy
  else if (c >= 26 && c <= 28) {
    return WEATHER_ICON_CLOUDY;
  }
  // Partly Cloudy or Fair
  else if (c == 29 || c == 30 || c == 44) {
    if (night_time)
      return WEATHER_ICON_PARTLY_CLOUDY_NIGHT;
    else
      return WEATHER_ICON_PARTLY_CLOUDY_DAY;
  }
  // Clear Day Night
  else if (c == 31 || c == 32) {
    if (night_time)
      return WEATHER_ICON_CLEAR_NIGHT;
    else
      return WEATHER_ICON_CLEAR_DAY;
  }
  // Fair Day Night
  else if (c == 33 || c == 34) {
    if (night_time)
      return WEATHER_ICON_FAIR_NIGHT;
    else
      return WEATHER_ICON_FAIR_DAY;
  }
  // Hot
  else if (c == 36) {
      return WEATHER_ICON_HOT;
  }
  // Isolated / Scattered Thunderstorm
  else if ((c >= 37 && c <= 39) || c == 47) {
    if (night_time)
      return WEATHER_ICON_THUNDER;
    else
      return WEATHER_ICON_THUNDER_SUN;
  }
  // Scattered Showers
  else if (c == 40) {
    if (night_time)
      return WEATHER_ICON_RAIN;
    else
      return WEATHER_ICON_RAIN_SUN;
  }
  // Weather condition not available
  else if (c == 3200) {
      return WEATHER_ICON_NOT_AVAILABLE;
  }
  else {
    return WEATHER_ICON_NOT_AVAILABLE;
  }
}

