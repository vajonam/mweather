
Weather My Way (Pebble SDK 2.0)
=================================

This is my attempt at learning about the Pebble SDK by adjusting an app for my own needs. The original Futura Weather watchface was the cleanest and best looking design I had seen but I prefer YAHOO! Weather data. So, I added the option to switch between both. 

I make no claims about the icons nor the design of this app, all credit for that goes to those listed below. 

Give it a shot and let me know what you think: [download pbw here](https://github.com/jaredbiehler/weather-my-way/releases/download/1.0.2/weather-my-way-v1.0.2.pbw)

![pebble screen1](https://raw.githubusercontent.com/jaredbiehler/weather-my-way/master/screenshots/pebble-screenshot1.png)&nbsp;
![pebble screen2](https://raw.githubusercontent.com/jaredbiehler/weather-my-way/master/screenshots/pebble-screenshot2.png)&nbsp;
![pebble screen3](https://raw.githubusercontent.com/jaredbiehler/weather-my-way/master/screenshots/pebble-screenshot3.png)&nbsp;
![pebble screen4](https://raw.githubusercontent.com/jaredbiehler/weather-my-way/master/screenshots/pebble-screenshot4.png)

## Configuration 

[Try it yourself here](http://jaredbiehler.github.io/weather-my-way/config/)

 Query string variables: 
```
s=[yahoo|open]  // weather service
u=[F|C]         // weather scale
d=[true|false]  // debug enabled
b=[on|off]      // battery enabled
```

![config screen](https://raw.githubusercontent.com/jaredbiehler/weather-my-way/master/screenshots/weather-my-way-config.png)

## Progress
 - Configurable weather provider (YAHOO!, Open Weather map)
  - More granularity in weather condition expression (via YAHOO!)
 - Configurable minimal battery display
 - Configurable weather scale (°F / °C)
 - Configurable debug mode ((L)ast updated, (P)ublish Date, Neighborhood)
 - JQuery Mobile configuration screen [here](http://jaredbiehler.github.io/weather-my-way/config/)
 - Config settings are persisted 
 - No longer subscribed to second ticks, minute & day for better battery performance
 - Tested on iOS 7.1 & Android 4.3
 - Improved network link monitoring (limited retries on both Pebble and JS sides)
 - many rewrites, separation of concerns

## TODO
- Reach: refactor global WeatherData object

### Based on work by:
 - "Futura Weather 2" by Niknam - https://github.com/Niknam/futura-weather-sdk2.0
 - "WeatherWatch" by Katharine - https://github.com/Katharine/WeatherWatch
 - "Roboto Weather" by Martin Rosinski - http://www.mypebblefaces.com/apps/3601/3408/
