
Weather My Way (Pebble SDK 2.0)
=================================

This is my attempt at both learning about the Pebble SDK and adjusting an app for my own needs. Give it a try! 

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
 - Config settings are persisted 
 - No longer subscribed to second changes, minute & day
 - Minor rewrites, separation of concerns

## TODO
- Better network link monitoring, including bluetooth service
- Refactor global WeatherData object

### Based on work by:
 - "Futura Weather 2" by Niknam - https://github.com/Niknam/futura-weather-sdk2.0
 - "WeatherWatch" by Katharine - https://github.com/Katharine/WeatherWatch
 - "Roboto Weather" by Martin Rosinski - http://www.mypebblefaces.com/apps/3601/3408/
