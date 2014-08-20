typedef struct {
	TextLayer *main_layer_background;
	GBitmap *sunrise_icon;
	GBitmap *sunset_icon;
	BitmapLayer *sunrise_icon_layer;
	BitmapLayer *sunset_icon_layer;
	TextLayer *sunset_time_layer;
	TextLayer *sunrise_time_layer;

} EWeatherLayerData;

void eweather_layer_create(GRect frame, Window *window);
void eweather_layer_hide(bool hide);
void eweather_layer_update(WeatherData *weather_data);


