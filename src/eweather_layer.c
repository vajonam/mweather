#include <pebble.h>
#include "network.h"
#include "weather_layer.h"
#include "eweather_layer.h"
#include "gbitmap_tools.h"

#define EWEATHER_FRAME   (GRect(144, 0, 288, 120))

static Layer *eweather_layer;
static GFont small_font;

static char *direction[] = { "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
		"S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW" };

static char sunrise_text[] = "00:00XX";
static char sunset_text[] = "00:00XX";
static char windspeed_text[] = "000XX";
static char humidity_text[] = "000X";
static char temp_high_text[] = "0000X";
static char temp_low_text[] = "0000X";

void eweather_layer_create(Layer *weather_layer) {


	eweather_layer = layer_create_with_data(EWEATHER_FRAME, sizeof(EWeatherLayerData));
	EWeatherLayerData *ewd = layer_get_data(eweather_layer);


	ewd->main_layer_background = text_layer_create(GRect(0, 0, 144, 90));
	text_layer_set_background_color(ewd->main_layer_background, GColorWhite);
	layer_add_child(eweather_layer,
			text_layer_get_layer(ewd->main_layer_background));

	small_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);



	ewd->sunrise_icon = gbitmap_create_with_resource(RESOURCE_ID_SUNRISE);
	ewd->sunrise_icon_layer = bitmap_layer_create(GRect(1, 3, 20, 20));
	layer_add_child(eweather_layer,
			bitmap_layer_get_layer(ewd->sunrise_icon_layer));
	bitmap_layer_set_bitmap(ewd->sunrise_icon_layer, ewd->sunrise_icon);

	ewd->sunrise_time_layer = text_layer_create(GRect(22, 3, 54, 36));
	text_layer_set_background_color(ewd->sunrise_time_layer, GColorClear);
	text_layer_set_text_alignment(ewd->sunrise_time_layer, GTextAlignmentLeft);
	text_layer_set_font(ewd->sunrise_time_layer, small_font);
	layer_add_child(eweather_layer,
			text_layer_get_layer(ewd->sunrise_time_layer));

	ewd->sunset_icon = gbitmap_create_with_resource(RESOURCE_ID_SUNSET);
	ewd->sunset_icon_layer = bitmap_layer_create(GRect(1,24, 20, 20));
	layer_add_child(eweather_layer,
			bitmap_layer_get_layer(ewd->sunset_icon_layer));
	bitmap_layer_set_bitmap(ewd->sunset_icon_layer, ewd->sunset_icon);

	ewd->sunset_time_layer = text_layer_create(GRect(22, 24, 54, 36));
	text_layer_set_background_color(ewd->sunset_time_layer, GColorClear);
	text_layer_set_text_alignment(ewd->sunset_time_layer, GTextAlignmentLeft);
	text_layer_set_font(ewd->sunset_time_layer, small_font);
	layer_add_child(eweather_layer,
			text_layer_get_layer(ewd->sunset_time_layer));

	ewd->wind_icon = gbitmap_create_with_resource(RESOURCE_ID_WINDSPEED);
	ewd->wind_icon_layer = bitmap_layer_create(GRect(1, 46, 20, 20));
	layer_add_child(eweather_layer,
			bitmap_layer_get_layer(ewd->wind_icon_layer));
	bitmap_layer_set_bitmap(ewd->wind_icon_layer, ewd->wind_icon);


	ewd->wind_speeddir_layer = text_layer_create(GRect(25, 46,  54, 36));
	text_layer_set_background_color(ewd->wind_speeddir_layer, GColorClear);
	text_layer_set_text_alignment(ewd->wind_speeddir_layer,
			GTextAlignmentLeft);
	text_layer_set_font(ewd->wind_speeddir_layer, small_font);
	layer_add_child(eweather_layer,
			text_layer_get_layer(ewd->wind_speeddir_layer));



	ewd->humidity_icon = gbitmap_create_with_resource(RESOURCE_ID_HUMIDITY);
	ewd->humidity_icon_layer = bitmap_layer_create(GRect(75+5, 3, 20, 20));
	layer_add_child(eweather_layer,
			bitmap_layer_get_layer(ewd->humidity_icon_layer));
	bitmap_layer_set_bitmap(ewd->humidity_icon_layer, ewd->humidity_icon);

	ewd->humidity_layer = text_layer_create(GRect(97+5, 3,  37+5, 36));
	text_layer_set_background_color(ewd->humidity_layer, GColorClear);
	text_layer_set_text_alignment(ewd->humidity_layer, GTextAlignmentLeft);
	text_layer_set_font(ewd->humidity_layer, small_font);
	layer_add_child(eweather_layer, text_layer_get_layer(ewd->humidity_layer));


	ewd->temp_high_icon = gbitmap_create_with_resource(RESOURCE_ID_HIGH);
	ewd->temp_high_icon_layer = bitmap_layer_create(GRect(75+5, 24, 20, 20));
	layer_add_child(eweather_layer,
			bitmap_layer_get_layer(ewd->temp_high_icon_layer));
	bitmap_layer_set_bitmap(ewd->temp_high_icon_layer, ewd->temp_high_icon);

	ewd->temp_high_layer = text_layer_create(GRect(100, 24,  144-100, 36));
	text_layer_set_background_color(ewd->temp_high_layer, GColorClear);
	text_layer_set_text_alignment(ewd->temp_high_layer, GTextAlignmentLeft);
	text_layer_set_font(ewd->temp_high_layer, small_font);
	layer_add_child(eweather_layer, text_layer_get_layer(ewd->temp_high_layer));


	ewd->temp_low_icon = gbitmap_create_with_resource(RESOURCE_ID_LOW);
	ewd->temp_low_icon_layer = bitmap_layer_create(GRect(75+5, 46, 20, 20));
	layer_add_child(eweather_layer,
			bitmap_layer_get_layer(ewd->temp_low_icon_layer));
	bitmap_layer_set_bitmap(ewd->temp_low_icon_layer, ewd->temp_low_icon);

	ewd->temp_low_layer = text_layer_create(GRect(100, 46,  144-100, 36));
	text_layer_set_background_color(ewd->temp_low_layer, GColorClear);
	text_layer_set_text_alignment(ewd->temp_low_layer, GTextAlignmentLeft);
	text_layer_set_font(ewd->temp_low_layer, small_font);
	text_layer_set_text_color(ewd->temp_low_layer, GColorBlack);
	layer_add_child(eweather_layer, text_layer_get_layer(ewd->temp_low_layer));

	ewd->locale_layer = text_layer_create(GRect(2, 68, 144 , 20));
	text_layer_set_text_color(ewd->locale_layer, GColorBlack);
	text_layer_set_background_color(ewd->locale_layer, GColorClear);
	text_layer_set_text_alignment(ewd->locale_layer, GTextAlignmentCenter);
	text_layer_set_font(ewd->locale_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(eweather_layer, text_layer_get_layer(ewd->locale_layer));


	layer_add_child(weather_layer, eweather_layer);



}

void eweather_layer_update(WeatherData *weather_data) {

	EWeatherLayerData *ewd = layer_get_data(eweather_layer);

	time_t sunrise_t = weather_data->sunrise - weather_data->tzoffset;

	struct tm *sunriseTime = localtime(&sunrise_t);

	strftime(sunrise_text, sizeof(sunrise_text),
			clock_is_24h_style() ? "%R" : "%l:%M%p", sunriseTime);
	text_layer_set_text(ewd->sunrise_time_layer, sunrise_text);

	time_t sunset_t = weather_data->sunset - weather_data->tzoffset;

	struct tm *sunsetTime = localtime(&sunset_t);

	strftime(sunset_text, sizeof(sunset_text),
			clock_is_24h_style() ? "%R" : "%l:%M%p", sunsetTime);
	text_layer_set_text(ewd->sunset_time_layer, sunset_text);

	snprintf(windspeed_text, sizeof(windspeed_text), "%i %s",
			weather_data->wind_speed, direction[weather_data->wind_dir % 16]);
	text_layer_set_text(ewd->wind_speeddir_layer, windspeed_text);

	snprintf(humidity_text, sizeof(humidity_text), "%i%%",
			weather_data->humidity);
	text_layer_set_text(ewd->humidity_layer, humidity_text);

	snprintf(temp_high_text, sizeof(temp_high_text), "%i°",
			weather_data->temp_high);
	text_layer_set_text(ewd->temp_high_layer, temp_high_text);

	snprintf(temp_low_text, sizeof(temp_low_text), "%i°",
			weather_data->temp_low);
	
	text_layer_set_text(ewd->temp_low_layer, temp_low_text);

	time_t pubdate_t = weather_data->pub_date - weather_data->tzoffset;

	struct tm *pubdateTime = gmtime(&pubdate_t);
	char pubdate_str[] = "00:00XX";

	strftime(pubdate_str, sizeof(pubdate_str),
			clock_is_24h_style() ? "%R" : "%l:%M%p", pubdateTime);

	snprintf(ewd->locale_pubDate_str, sizeof(ewd->locale_pubDate_str), "%s @ %s",
						weather_data->locale, pubdate_str );


	text_layer_set_text(ewd->locale_layer, ewd->locale_pubDate_str);
}


void eweather_layer_destroy() {

	EWeatherLayerData *ewd = layer_get_data(eweather_layer);

	text_layer_destroy(ewd->main_layer_background);
	gbitmap_destroy(ewd->sunrise_icon);
	gbitmap_destroy(ewd->sunset_icon);
	gbitmap_destroy(ewd->wind_icon);
	gbitmap_destroy(ewd->humidity_icon);
	gbitmap_destroy(ewd->temp_high_icon);
	gbitmap_destroy(ewd->temp_low_icon);

	bitmap_layer_destroy(ewd->sunrise_icon_layer);
	bitmap_layer_destroy(ewd->wind_icon_layer);
	bitmap_layer_destroy(ewd->sunset_icon_layer);
	bitmap_layer_destroy(ewd->humidity_icon_layer);
	bitmap_layer_destroy(ewd->temp_high_icon_layer);
	bitmap_layer_destroy(ewd->temp_low_icon_layer);

	text_layer_destroy(ewd->sunrise_time_layer);
	text_layer_destroy(ewd->sunset_time_layer);
	text_layer_destroy(ewd->humidity_layer);
	text_layer_destroy(ewd->wind_speeddir_layer);
	text_layer_destroy(ewd->temp_high_layer);
	text_layer_destroy(ewd->temp_low_layer);
	text_layer_destroy(ewd->locale_layer);

	layer_destroy(eweather_layer);
}

