var SERVICE_OPEN_WEATHER  = "open";
var SERVICE_YAHOO_WEATHER = "yahoo";

var updateInProgress = false;
var weatherService   = SERVICE_YAHOO_WEATHER;

Pebble.addEventListener("ready", function(e) {
    console.log("Starting ...");
    updateWeather();
});

Pebble.addEventListener("appmessage", function(e) {
    console.log("Got a message - Starting weather request...");
    updateWeather();
});

Pebble.addEventListener("showConfiguration", function (e) {
    console.log('Configuration requested');
    Pebble.openURL("http://ishoneyvegan.com/pebble/?s="+weatherService);
});

Pebble.addEventListener("webviewclosed", function(e) {
    
    console.log('Response: '+e.response);

    if (e.response) {
      try {
        var responseFromWebView = decodeURIComponent(e.response);
        var settings = JSON.parse(responseFromWebView);
        if (settings.radioyahoo) {
          weatherService = SERVICE_YAHOO_WEATHER;
        } else {
          weatherService = SERVICE_OPEN_WEATHER;
        }
        console.log("Weather service is "+weatherService);
        updateWeather();
      } catch(e) {
        console.log("Unable to parse response from configuration:"+e.message);
      }
    }
});

function updateWeather() {
    if (!updateInProgress) {
        updateInProgress = true;
        var locationOptions = { "timeout": 15000, "maximumAge": 60000 };
        navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
    }
    else {
        console.log("Not starting a new request. Another one is in progress...");
    }
}

function locationSuccess(pos) {
    var coordinates = pos.coords;
    console.log("Got coordinates: " + JSON.stringify(coordinates));
    if (weatherService === SERVICE_YAHOO_WEATHER) {
      fetchYahooWeather(coordinates.latitude, coordinates.longitude);
    } else {
      fetchOpenWeather(coordinates.latitude, coordinates.longitude);
    }
}

function locationError(err) {
    console.warn('Location error (' + err.code + '): ' + err.message);
    Pebble.sendAppMessage({ "error": "Loc unavailable" });
    updateInProgress = false;
}

var fetchYahooWeather = function(latitude, longitude) {

  var subselect = 'SELECT woeid FROM geo.placefinder WHERE text="'+latitude+','+longitude+'" AND gflags="R"';
  var query = 'SELECT * FROM weather.forecast WHERE woeid IN ('+subselect+')';
  var url = "https://query.yahooapis.com/v1/public/yql?format=json&q=" + encodeURIComponent(query);

  getJson(url, function(err, response) {

    try {
      if (err) {
        throw err;
      }

      var sunrise = response.query.results.channel.astronomy.sunrise;
      var sunset  = response.query.results.channel.astronomy.sunset;

      var weather = {
        "condition":    parseInt(response.query.results.channel.item.condition.code),
        "temperature":  parseInt(response.query.results.channel.item.condition.temp),
        "sunrise":      Date.parse((new Date()).toDateString()+" "+sunrise) / 1000,
        "sunset":       Date.parse((new Date()).toDateString()+" "+sunset) / 1000,
        "current_time": Date.now() / 1000,
        "service":      SERVICE_YAHOO_WEATHER
      }

      console.log('Weather Data: ' + JSON.stringify(weather));

      Pebble.sendAppMessage(weather);

    } catch (e) {
      console.log("Could not find weather data in response: " + e.message);
      Pebble.sendAppMessage({ "error": "HTTP Error" });
    } finally {
      updateInProgress = false;
    }
  });
}

var fetchOpenWeather = function(latitude, longitude) {

  var url = "http://api.openweathermap.org/data/2.5/weather?" +
    "lat=" + latitude + "&lon=" + longitude + "&cnt=1";

  getJson(url, function(err, response) {

    try {
      if (err) {
        throw err;
      }

      var temperature, sunrise, sunset, condition;
      var current_time = Date.now() / 1000;

      var tempResult = response.main.temp;
      if (response.sys.country === "US") {
        // Convert temperature to Fahrenheit if user is within the US
        temperature = Math.round(((tempResult - 273.15) * 1.8) + 32);
      }
      else {
        // Otherwise, convert temperature to Celsius
        temperature = Math.round(tempResult - 273.15);
      }

      condition = response.weather[0].id;
      sunrise = response.sys.sunrise;
      sunset = response.sys.sunset;

      var weather = {
        "condition":    condition,
        "temperature":  temperature,
        "sunrise":      sunrise,
        "sunset":       sunset,
        "current_time": current_time,
        "service":      SERVICE_OPEN_WEATHER
      };

      console.log('Weather Data: ' + JSON.stringify(weather));

      Pebble.sendAppMessage(weather);

    } catch (e) {
      console.log("Could not find weather data in response: " + e.message);
      Pebble.sendAppMessage({ "error": "HTTP Error" });
    } finally {
      updateInProgress = false;
    }
  });
}


var getJson = function(url, callback) {
  try {
    var req = new XMLHttpRequest();
    req.open('GET', url, true);
    req.onload = function(e) {
      if (req.readyState == 4) {
        if(req.status == 200) {
          try {
            //console.log(req.responseText);
            var response = JSON.parse(req.responseText);
            callback(null, response);
          } catch (e) {
            callback(e.message);
          }
        } else {
          callback("Error request status not 200, status: "+req.status);
        }
      }
    };
    req.send(null);
  } catch(e) {
    callback("Unable to GET JSON: "+e.message);
  }
}