
MWeather (Pebble SDK 2.0)
=================================

This is my attempt at learning about the Pebble SDK by adjusting an app for my own needs. The original Weather My Way and Futura watchface was the cleanest and best looking design I had seen, I needed configureability in the hourly forecasts, as well other information a golfer would need to see if can golf the next morning or that afternoon. So I added another screen is contains the entended weather information

![pebble screen1](https://raw.githubusercontent.com/vajonam/mweather/stable/screenshots/pebble-screenshot_1.png)&nbsp;
![pebble screen2](https://raw.githubusercontent.com/vajonam/mweather/stable/screenshots/pebble-screenshot_2.png)&nbsp;
![pebble screen3](https://raw.githubusercontent.com/vajonam/mweather/stable/screenshots/pebble-screenshot_3.png)
![pebble screen4](https://raw.githubusercontent.com/vajonam/mweather/stable/screenshots/pebble-screenshot_4.png)

## Requirements

The hourly data for this app comes from the [Weather Underground API](http://www.wunderground.com/weather/api/). Unfortunately, their API is not free. However, Weather Underground does provide a free developer API key (500 hits / day) which more than suffices for this app. Don't share your API key, as once the quota has been reached the key will stop working. 

## Configuration 

[Try it yourself here](http://vajonam.github.io/vajonam/mweather/config/)

 Query string variables: 
```
s=[yahoo|open]  // weather service
u=[F|C]         // weather scale
d=[true|false]  // debug enabled
b=[on|off]      // battery enabled
a=[apikey]      // Weather Underground API Key 
h1=[0-23]       // offset for 1st hourly forecast
h2=[0-23]       // offser for 2nd hourly forecast
```

![config screen](https://raw.githubusercontent.com/vajonam/mweather/stable/screenshots/weather-my-way-config.png)

## Progress

- hourly forecasts are now configureable
- updated time and forecast publication date
- additonal info, like Probablity of Precipitation
- added config option to show feels like (humidex,windchill)
- extended weather info on double share (2 tap/shakes in 10 seconds)
- animated transitions
- included resizing api gbitmap_tools

### Based on work by:
 - "Weather My Way" by jaredbiehler - https://github.com/jaredbiehler/weather-my-way
 - "Futura Weather 2" by Niknam - https://github.com/Niknam/futura-weather-sdk2.0
 - "WeatherWatch" by Katharine - https://github.com/Katharine/WeatherWatch
 - "Roboto Weather" by Martin Rosinski - http://www.mypebblefaces.com/apps/3601/3408/
