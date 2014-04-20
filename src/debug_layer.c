#include <pebble.h>
#include "network.h"
#include "debug_layer.h"

static TextLayer *debug_layer;

static char last_update_text[] = "00:00";
static char debug_msg[200];

TextLayer *debug_layer_create(GRect frame)
{
  debug_layer = text_layer_create(frame);
  text_layer_set_text_color(debug_layer, GColorBlack);
  text_layer_set_background_color(debug_layer, GColorClear);
  text_layer_set_font(debug_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(debug_layer, GTextAlignmentRight);

  return debug_layer;
}

void debug_layer_update(time_t currentTime, WeatherData *weather_data)
{
  if (weather_data->debug) {

    if (weather_data->updated != 0) {

      time_t lastUpdated = weather_data->updated;
      struct tm *updatedTime = localtime(&lastUpdated);
      strftime(last_update_text, sizeof(last_update_text), "%R", updatedTime);
      snprintf(debug_msg, sizeof(debug_msg), 
        "L%s, P%s, %s", last_update_text, weather_data->pub_date, weather_data->neighborhood);

      // reset localtime, critical as localtime modifies a shared object!
      localtime(&currentTime);

    } else {
      snprintf(debug_msg, sizeof(debug_msg), "Weather N/A");
    }

  } else {
     strcpy(debug_msg, "");
  }

  text_layer_set_text(debug_layer, debug_msg);
}


void debug_layer_destroy() 
{
  text_layer_destroy(debug_layer);
}


