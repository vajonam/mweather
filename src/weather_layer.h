#ifndef WEATHER_LAYER_H
#define WEATHER_LAYER_H

typedef struct {
	TextLayer *temp_layer_background;
	TextLayer *temp_layer;
	GBitmap *icon;
	BitmapLayer *icon_layer;
	char temp_str[6];
} WeatherLayerData;

typedef enum {
	WEATHER_ICON_CLEAR_DAY = 0,
	WEATHER_ICON_CLEAR_NIGHT,
	WEATHER_ICON_RAIN,
	WEATHER_ICON_SNOW,
	WEATHER_ICON_SLEET,
	WEATHER_ICON_WIND,
	WEATHER_ICON_FOG,
	WEATHER_ICON_CLOUDY,
	WEATHER_ICON_PARTLY_CLOUDY_DAY,
	WEATHER_ICON_PARTLY_CLOUDY_NIGHT,
	WEATHER_ICON_THUNDER,
	WEATHER_ICON_RAIN_SNOW,
	WEATHER_ICON_RAIN_SLEET,
	WEATHER_ICON_SNOW_SLEET,
	WEATHER_ICON_COLD,
	WEATHER_ICON_HOT,
	WEATHER_ICON_DRIZZLE,
	WEATHER_ICON_NOT_AVAILABLE,
	WEATHER_ICON_PHONE_ERROR,
	WEATHER_ICON_CLOUD_ERROR,
	WEATHER_ICON_LOADING1,
	WEATHER_ICON_LOADING2,
	WEATHER_ICON_LOADING3,
	WEATHER_ICON_COUNT
} WeatherIcon;

void weather_layer_create(GRect frame, Window *window);
void weather_animate(void *context);
void weather_layer_update(WeatherData *weather_data);
void weather_layer_destroy();
void weather_layer_set_icon(WeatherIcon icon);
void weather_layer_set_temperature(int16_t temperature, bool is_stale);
uint8_t open_weather_icon_for_condition(int condition, bool night_time);
uint8_t yahoo_weather_icon_for_condition(int condition, bool night_time);

#endif
