#ifndef WEATHER_LAYER_H
#define WEATHER_LAYER_H

typedef struct {
	Layer *loading_layer;
	TextLayer *temp_layer_background;
	TextLayer *primary_temp_layer;

	GBitmap *primary_icons;
	GBitmap *hourly_icons;

	int primary_icon_size;
	int hourly_icon_size;

	GBitmap *primary_icon;
	BitmapLayer *primary_icon_layer;

	GBitmap *h1_icon;
	BitmapLayer *h1_icon_layer;
	TextLayer *h1_time_layer;
	TextLayer *h1_temp_layer;
	TextLayer *h1_pop_layer;

    GBitmap *h2_icon;
	BitmapLayer *h2_icon_layer;
	TextLayer *h2_time_layer;
	TextLayer *h2_temp_layer;
	TextLayer *h2_pop_layer;

	char primary_temp_str[6];
	char h1_temp_str[6];
	char h2_temp_str[6];
} WeatherLayerData;

typedef enum {
	WEATHER_ICON_CLEAR_DAY = 0,
    WEATHER_ICON_FAIR_DAY,
	WEATHER_ICON_PARTLY_CLOUDY_DAY,
	WEATHER_ICON_MOSTLY_CLOUDY_DAY,
	WEATHER_ICON_CLOUDY,
	WEATHER_ICON_CLEAR_NIGHT,
    WEATHER_ICON_FAIR_NIGHT,
	WEATHER_ICON_PARTLY_CLOUDY_NIGHT,
	WEATHER_ICON_WIND,
	WEATHER_ICON_FOG,
	WEATHER_ICON_DRIZZLE,
	WEATHER_ICON_RAIN,
	WEATHER_ICON_RAIN_SLEET,
	WEATHER_ICON_SLEET,
	WEATHER_ICON_SNOW_SLEET,
	WEATHER_ICON_HEAVY_SNOW,
	WEATHER_ICON_SNOW,
	WEATHER_ICON_RAIN_SNOW,
    WEATHER_ICON_RAIN_SUN,
    WEATHER_ICON_THUNDER_SUN,
	WEATHER_ICON_THUNDER,
	WEATHER_ICON_COLD,
	WEATHER_ICON_HOT,
	WEATHER_ICON_PHONE_ERROR,
	WEATHER_ICON_NOT_AVAILABLE,
	WEATHER_ICON_COUNT
} WeatherIcon;

typedef enum {
	PERIOD_PRIMARY = 0,
	PERIOD_HOUR1,
	PERIOD_HOUR2
} WeatherPeriod;

void weather_layer_create(GRect frame, Window *window);
void weather_animate(void *context);
void weather_layer_update(WeatherData *weather_data);
void weather_layer_destroy();
void weather_layer_set_temperature(int16_t temperature, bool is_stale);
void weather_layer_clear_temperature();
uint8_t open_weather_icon_for_condition(int condition, bool night_time);
uint8_t yahoo_weather_icon_for_condition(int condition, bool night_time);
uint8_t wunder_weather_icon_for_condition(int c, bool night_time);

#endif
