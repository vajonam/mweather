var SERVICE_OPEN_WEATHER  = "open";
var SERVICE_YAHOO_WEATHER = "yahoo";
var EXTERNAL_DEBUG_URL    = '';
var CONFIGURATION_URL     = '';

var updateInProgress = false;
var externalDebugEnabled = true;  // POST logs to external server - dangerous! lat lon recorded
var watchDebugEnabled = true;     // Display debug info on watch
var weatherService = SERVICE_YAHOO_WEATHER;

Pebble.addEventListener("ready", function(e) {
    console.log("Starting ...");
    updateWeather();
});

Pebble.addEventListener("appmessage", function(e) {
    console.log("Got a message - Starting weather request:" + JSON.stringify(e.payload));
    updateWeather();
});

Pebble.addEventListener("showConfiguration", function (e) {
    console.log('Configuration requested');
    var queryString = '?s='+weatherService+'&d='+watchDebugEnabled;
    Pebble.openURL(CONFIGURATION_URL+queryString);
});

Pebble.addEventListener("webviewclosed", function(e) {
    
    console.log('Response: '+e.response);

    if (e.response) {
      try {
        var responseFromWebView = decodeURIComponent(e.response);
        var settings = JSON.parse(responseFromWebView);

        if (settings.service === SERVICE_YAHOO_WEATHER) {
          weatherService = SERVICE_YAHOO_WEATHER;
        } else {
          weatherService = SERVICE_OPEN_WEATHER;
        }

        if (settings.debug === 'true') {
          watchDebugEnabled = true;
        } else {
          watchDebugEnabled = false;
        }

        console.log("Weather service is "+weatherService+", debug is "+watchDebugEnabled);
        updateWeather();
      } catch(e) {
        console.warn("Unable to parse response from configuration:"+e.message);
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
    var message = 'Location error (' + err.code + '): ' + err.message;
    console.warn(message);
    Pebble.sendAppMessage({ "error": "Loc unavailable" });
    postDebugMessage(message);
    updateInProgress = false;
}

var fetchYahooWeather = function(latitude, longitude) {

  var subselect = 'SELECT woeid FROM geo.placefinder WHERE text="'+latitude+','+longitude+'" AND gflags="R"';
  var neighbor  = 'SELECT neighborhood FROM geo.placefinder WHERE text="'+latitude+','+longitude+'" AND gflags="R";';
  var query     = 'SELECT * FROM weather.forecast WHERE woeid IN ('+subselect+');';
  var multi     = "SELECT * FROM yql.query.multi WHERE queries='"+query+" "+neighbor+"'";
  var url       = "https://query.yahooapis.com/v1/public/yql?format=json&q=" + encodeURIComponent(multi);

  //console.log('Query: ' + url);

  getJson(url, function(err, response) {

    try {
      if (err) {
        throw err;
      }

      var sunrise = response.query.results.results[0].channel.astronomy.sunrise;
      var sunset  = response.query.results.results[0].channel.astronomy.sunset;
      var pubdate = new Date(Date.parse(response.query.results.results[0].channel.item.pubDate));
      var neighborhood = ''+response.query.results.results[1].Result.neighborhood;

      var weather = {
        "condition":    parseInt(response.query.results.results[0].channel.item.condition.code),
        "temperature":  parseInt(response.query.results.results[0].channel.item.condition.temp),
        "sunrise":      Date.parse((new Date()).toDateString()+" "+sunrise) / 1000,
        "sunset":       Date.parse((new Date()).toDateString()+" "+sunset) / 1000,
        "service":      SERVICE_YAHOO_WEATHER,
        "neighborhood": neighborhood,
        "pubdate":      pubdate.getHours()+':'+pubdate.getMinutes(),
        "debug":        watchDebugEnabled
      }

      console.log('Weather Data: ' + JSON.stringify(weather));

      Pebble.sendAppMessage(weather);
      postDebugMessage(weather);
    
    } catch (e) {
      console.warn("Could not find weather data in response: " + e.message);
      var error = { "error": "HTTP Error" };
      Pebble.sendAppMessage(error);
      postDebugMessage(error);
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

      var temperature, sunrise, sunset, condition, pubdate;
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
      sunrise   = response.sys.sunrise;
      sunset    = response.sys.sunset;
      pubdate   = new Date(response.dt);

      var weather = {
        "condition":    condition,
        "temperature":  temperature,
        "sunrise":      sunrise,
        "sunset":       sunset,
        "service":      SERVICE_OPEN_WEATHER,
        "neighborhood": response.name,
        "pubdate":      pubdate.getHours()+':'+pubdate.getMinutes(),
        "debug":        watchDebugEnabled
      };

      console.log('Weather Data: ' + JSON.stringify(weather));

      Pebble.sendAppMessage(weather);

      postDebugMessage(weather);

    } catch (e) {
      console.warn("Could not find weather data in response: " + e.message);
      var error = { "error": "HTTP Error" };
      Pebble.sendAppMessage(error);
      postDebugMessage(error);
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

var postDebugMessage = function (data) {
  if (!externalDebugEnabled || EXTERNAL_DEBUG_URL === null) {
    return;
  }
  try {
    if (typeof data.sunrise !== "undefined") {
      delete data.sunrise;
      delete data.sunset;
    }
    post(EXTERNAL_DEBUG_URL, 'data='+JSON.stringify(data));
  } catch (e) {
    console.warn('Post Debug Message failed:'+e.message);
  }  
}

var post = function(url, data) {
  try {
    var req = new XMLHttpRequest();
    req.open('POST', url, true);
    req.send(data);
  } catch(e) {
    console.log('POST failed: ' + e.message);
  }
}