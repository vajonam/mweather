#include <pebble.h>
#include "network.h"
#include "weather_layer.h"
#include "eweather_layer.h"
#include "gbitmap_tools.h"


static Layer *eweather_layer;
static GFont *small_font;

static char *direction[] =  { "N", "NNE", "NE", "ENE", "E",
    "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};

static char sunrise_text[] = "00:00XX";
static char sunset_text[] = "00:00XX";
static char windspeed_text[] = "0000 XXX";
static char humidity_text[] = "000X";



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
	ewd->sunrise_icon_layer = bitmap_layer_create(GRect(1, 3, 33, 33));
	layer_add_child(eweather_layer,
			bitmap_layer_get_layer(ewd->sunrise_icon_layer));
	bitmap_layer_set_bitmap(ewd->sunrise_icon_layer, ewd->sunrise_icon);

	ewd->sunset_icon = gbitmap_create_with_resource(RESOURCE_ID_SUNSET);
	ewd->sunset_icon_layer = bitmap_layer_create(GRect(1, 33, 33, 33));
	layer_add_child(eweather_layer,
			bitmap_layer_get_layer(ewd->sunset_icon_layer));
	bitmap_layer_set_bitmap(ewd->sunset_icon_layer, ewd->sunset_icon);



	ewd->wind_icon  = gbitmap_create_with_resource(RESOURCE_ID_WINDSPEED);
	ewd->wind_icon_layer = bitmap_layer_create(GRect(66, 3, 33, 33));
	layer_add_child(eweather_layer,
			bitmap_layer_get_layer(ewd->wind_icon_layer));
	bitmap_layer_set_bitmap(ewd->wind_icon_layer, ewd->wind_icon);


	ewd->humidity_icon  = gbitmap_create_with_resource(RESOURCE_ID_HUMIDITY);
	ewd->humidity_icon_layer = bitmap_layer_create(GRect(69,36, 33, 33));
	layer_add_child(eweather_layer,
			bitmap_layer_get_layer(ewd->humidity_icon_layer));
	bitmap_layer_set_bitmap(ewd->humidity_icon_layer, ewd->humidity_icon);


	ewd->sunset_time_layer = text_layer_create(GRect(33, 40, 60, 36));
	text_layer_set_background_color(ewd->sunset_time_layer, GColorClear);
	text_layer_set_text_alignment(ewd->sunset_time_layer, GTextAlignmentLeft);
	text_layer_set_font(ewd->sunset_time_layer, small_font);
	layer_add_child(eweather_layer,
			text_layer_get_layer(ewd->sunset_time_layer));



	ewd->sunrise_time_layer = text_layer_create(GRect(33, 8, 60, 36));
	text_layer_set_background_color(ewd->sunrise_time_layer, GColorClear);
	text_layer_set_text_alignment(ewd->sunrise_time_layer, GTextAlignmentLeft);
	text_layer_set_font(ewd->sunrise_time_layer, small_font);
	layer_add_child(eweather_layer,
			text_layer_get_layer(ewd->sunrise_time_layer));

	ewd->wind_speeddir_layer = text_layer_create(GRect(95,8,45,36));
	text_layer_set_background_color(ewd->wind_speeddir_layer,GColorClear);
	text_layer_set_text_alignment(ewd->wind_speeddir_layer, GTextAlignmentRight);
		text_layer_set_font(ewd->wind_speeddir_layer, small_font);
	layer_add_child(eweather_layer,text_layer_get_layer(ewd->wind_speeddir_layer));

	ewd->humidity_layer = text_layer_create(GRect(102,40,38,36));
	text_layer_set_background_color(ewd->humidity_layer,GColorClear);
	text_layer_set_text_alignment(ewd->humidity_layer, GTextAlignmentRight);
		text_layer_set_font(ewd->humidity_layer, small_font);
	layer_add_child(eweather_layer,text_layer_get_layer(ewd->humidity_layer));


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




  	APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting wind speed %s",windspeed_text);



  	snprintf(windspeed_text, sizeof(windspeed_text), "%i %s", weather_data->wind_speed,direction[ weather_data->wind_dir % 16]);
  	text_layer_set_text( ewd->wind_speeddir_layer, windspeed_text);

  	snprintf(humidity_text, sizeof(humidity_text), "%i%%", weather_data->humidity );
	text_layer_set_text( ewd->humidity_layer, humidity_text);

}

void eweather_layer_hide(bool hide){ 

   layer_set_hidden(eweather_layer,hide);

}

void eweather_layer_destroy() {

	EWeatherLayerData *ewd = layer_get_data(eweather_layer);

	text_layer_destroy(ewd->main_layer_background);
	gbitmap_destroy(ewd->sunrise_icon);
	gbitmap_destroy(ewd->sunset_icon);
	gbitmap_destroy(ewd->wind_icon);
	bitmap_layer_destroy(ewd->sunrise_icon_layer);
	bitmap_layer_destroy(ewd->wind_icon_layer);
	bitmap_layer_destroy(ewd->sunset_icon_layer);
	text_layer_destroy(ewd->sunrise_time_layer);
	text_layer_destroy(ewd->sunset_time_layer);
	text_layer_destroy(ewd->wind_speeddir_layer);


	gbitmap_destroy(ewd->humidity_icon);
	bitmap_layer_destroy(ewd->humidity_icon_layer);
	text_layer_destroy(ewd->humidity_layer);

	fonts_unload_custom_font(small_font);
	layer_destroy(eweather_layer);

}


