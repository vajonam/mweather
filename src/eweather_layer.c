#include <pebble.h>
#include "network.h"
#include "weather_layer.h"
#include "eweather_layer.h"

static Layer *eweather_layer;
static GFont *small_font;

static char sunrise_text[] = "00:00XX";
static char sunset_text[] = "00:00XX";

void eweather_layer_create(GRect frame, Window *window)
{

APP_LOG(APP_LOG_LEVEL_DEBUG, "creaing");

  eweather_layer = layer_create_with_data(frame,sizeof(EWeatherLayerData));
  EWeatherLayerData *ewd = layer_get_data(eweather_layer);


  APP_LOG(APP_LOG_LEVEL_DEBUG, "creating bg");

  ewd->main_layer_background = text_layer_create(GRect(0, 0, 144, 168));
  text_layer_set_background_color(ewd->main_layer_background, GColorWhite);
  layer_add_child(eweather_layer, text_layer_get_layer(ewd->main_layer_background));
  
	small_font = fonts_load_custom_font(
			resource_get_handle(RESOURCE_ID_FUTURA_17));

  APP_LOG(APP_LOG_LEVEL_DEBUG, "setting  icon");

  ewd->sunrise_icon = gbitmap_create_with_resource(RESOURCE_ID_SUNRISE);
	ewd->sunrise_icon_layer = bitmap_layer_create(GRect(3, 3, 33, 33));
	layer_add_child(eweather_layer,
			bitmap_layer_get_layer(ewd->sunrise_icon_layer));
	bitmap_layer_set_bitmap(ewd->sunrise_icon_layer, ewd->sunrise_icon);

	ewd->sunset_icon = gbitmap_create_with_resource(RESOURCE_ID_SUNSET);
	ewd->sunset_icon_layer = bitmap_layer_create(GRect(3, 33, 33, 33));
	layer_add_child(eweather_layer,
			bitmap_layer_get_layer(ewd->sunset_icon_layer));
	bitmap_layer_set_bitmap(ewd->sunset_icon_layer, ewd->sunset_icon);

	ewd->sunrise_time_layer = text_layer_create(GRect(36, 8, 100, 36));
	text_layer_set_background_color(ewd->sunrise_time_layer, GColorClear);
	text_layer_set_text_alignment(ewd->sunrise_time_layer, GTextAlignmentLeft);
	text_layer_set_font(ewd->sunrise_time_layer, small_font);
	layer_add_child(eweather_layer,
			text_layer_get_layer(ewd->sunrise_time_layer));

	ewd->sunset_time_layer = text_layer_create(GRect(36, 40, 100, 36));
	text_layer_set_background_color(ewd->sunset_time_layer, GColorClear);
	text_layer_set_text_alignment(ewd->sunset_time_layer, GTextAlignmentLeft);
	text_layer_set_font(ewd->sunset_time_layer, small_font);
	layer_add_child(eweather_layer,
			text_layer_get_layer(ewd->sunset_time_layer));



  layer_add_child(window_get_root_layer(window), eweather_layer);
  eweather_layer_hide(true);


}

void eweather_layer_update(WeatherData *weather_data) {

	EWeatherLayerData *ewd = layer_get_data(eweather_layer);

	time_t sunrise_t = weather_data->sunrise - weather_data->tzoffset;

	struct tm *sunriseTime = localtime(&sunrise_t);

	strftime(sunrise_text, sizeof(sunrise_text),
			clock_is_24h_style() ? "%R" : "%l:%M",  sunriseTime);
  	text_layer_set_text( ewd->sunrise_time_layer,sunrise_text);


	time_t sunset_t = weather_data->sunset - weather_data->tzoffset;

	struct tm *sunsetTime = localtime(&sunset_t);

	strftime(sunset_text, sizeof(sunset_text),
			clock_is_24h_style() ? "%R" : "%l:%M",  sunsetTime);
  	text_layer_set_text( ewd->sunset_time_layer,sunset_text);

}

void eweather_layer_hide(bool hide){ 

   layer_set_hidden(eweather_layer,hide);

}

void eweather_layer_destroy() {

	EWeatherLayerData *ewd = layer_get_data(eweather_layer);

	text_layer_destroy(ewd->main_layer_background);
	gbitmap_destroy(ewd->sunrise_icon);
	gbitmap_destroy(ewd->sunset_icon);
	bitmap_layer_destroy(ewd->sunrise_icon_layer);
	bitmap_layer_destroy(ewd->sunset_icon_layer);
	text_layer_destroy(ewd->sunrise_time_layer);
	text_layer_destroy(ewd->sunset_time_layer);

	fonts_unload_custom_font(small_font);
	layer_destroy(eweather_layer);

}


