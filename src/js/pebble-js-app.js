var SERVICE_OPEN_WEATHER  = "open";
var SERVICE_YAHOO_WEATHER = "yahoo";
var EXTERNAL_DEBUG_URL    = '';
var CONFIGURATION_URL     = 'http://jaredbiehler.github.io/weather-my-way/config/';

var Global = {
  'externalDebug':     false, // POST logs to external server - dangerous! lat lon recorded
  'updateInProgress':  false,
  'updateWaitTimeout': 1 * 60 * 1000, // one minute in ms
  'lastUpdateAttempt': new Date(),
  'maxRetry':          3,
  'latestConfig': {
    'debugEnabled':   false,
    'batteryEnabled': true,
    'weatherService': SERVICE_YAHOO_WEATHER,
    'weatherScale':   'F'
  },
};

// Allow console messages to be turned on / off 
(function(){
    var original = console.log;
    var logMessage = function (message) {
        if (Global.latestConfig.debugEnabled) {
          original.apply(console, arguments);
        }  
    };
    console.log  = logMessage;
    console.warn = logMessage;
})();


// Setup Pebble Event Listeners
Pebble.addEventListener("ready", function(e) {
    console.log("Starting ...");
    var data = { "js_ready": true };
    Pebble.sendAppMessage(data, ack, function(e){
      nack(data);
    });
});

Pebble.addEventListener("appmessage", function(data) {
    console.log("Got a message - Starting weather request ... " + JSON.stringify(data));
    try {
      var config = {};
      config.weatherService = data.payload.service === SERVICE_OPEN_WEATHER ? SERVICE_OPEN_WEATHER : SERVICE_YAHOO_WEATHER;
      config.debugEnabled   = data.payload.debug   === 1;
      config.batteryEnabled = data.payload.battery === 1;
      config.weatherScale   = data.payload.scale   === 'C' ? 'C' : 'F';
      Global.latestConfig = config;
      updateWeather();
    } catch (e) {
      console.warn("Could not retrieve data sent from Pebble: "+e.message);
    }
});

/**
 * This is the reason for the Global.latestConfig variable - I couldn't think of a way (short of asking Pebble)
 * for the latest config settings. So I persist them in a rather ugly Global variable. 
 */
Pebble.addEventListener("showConfiguration", function (e) {
    var options = {
      's': Global.latestConfig.weatherService,
      'd': Global.latestConfig.debugEnabled,
      'u': Global.latestConfig.weatherScale,
      'b': Global.latestConfig.batteryEnabled ? 'on' : 'off'
    }
    var url = CONFIGURATION_URL+'?'+serialize(options);
    console.log('Configuration requested using url: '+url);
    Pebble.openURL(url);
});

Pebble.addEventListener("webviewclosed", function(e) {
    /*
     * Android Hack - for whatever reason this event is always firing on Android with a message of 'CANCELLED'
     */
    if (e.response && e.response !== 'CANCELLED') {
      try {
        var settings = JSON.parse(decodeURIComponent(e.response));

        // Android 'cancel' sends a blank object
        if (Object.keys(settings).length <= 0) {
          return; 
        }

        var config = {};
        config.weatherService = settings.service === SERVICE_OPEN_WEATHER ? SERVICE_OPEN_WEATHER : SERVICE_YAHOO_WEATHER;
        config.weatherScale   = settings.scale   === 'C' ? 'C' : 'F';
        config.debugEnabled   = settings.debug   === 'true';
        config.batteryEnabled = settings.battery === 'on';
        Global.latestConfig = config;

        console.log("Settings received: "+JSON.stringify(settings));
        updateWeather();
      } catch(e) {
        console.warn("Unable to parse response from configuration:"+e.message);
      }
    }
});

var ack  = function ()  { console.log("Pebble ACK sendAppMessage");}
var nack = function (data, retry) {
  retry = typeof retry !== 'undefined' ? retry : 0;
  retry++;
  if (retry >= Global.maxRetry) {
    console.warn("Pebble NACK sendAppMessage max exceeded");
    return;
  } 
  console.warn("Pebble NACK sendAppMessage retryCount:"+retry+" data:"+JSON.stringify(data));
  if (data) {
    Pebble.sendAppMessage(data, ack, function(e){
      nack(data, retry);
    });
  }
}

var updateWeather = function () {
  if (Global.updateInProgress && 
      new Date().getTime() < Global.lastUpdateAttempt.getTime() + Global.updateWaitTimeout) {
    console.log("Update already started in the last "+(Global.updateWaitTimeout/60000)+" minutes");
    return;
  }
  Global.updateInProgress  = true;
  Global.lastUpdateAttempt = new Date();

  var locationOptions = { "timeout": 15000, "maximumAge": 60000 };
  navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
}

var locationSuccess = function (pos) {
    var coordinates = pos.coords;
    console.log("Got coordinates: " + JSON.stringify(coordinates));
    if (Global.latestConfig.weatherService === SERVICE_OPEN_WEATHER) {
      fetchOpenWeather(coordinates.latitude, coordinates.longitude);
    } else {
      fetchYahooWeather(coordinates.latitude, coordinates.longitude);
    }
}

var locationError = function (err) {
    var message = 'Location error (' + err.code + '): ' + err.message;
    console.warn(message);
    Pebble.sendAppMessage({ "error": "Loc unavailable" }, ack, nack);
    postDebugMessage({"error": message});
    Global.updateInProgress = false;
}

var fetchYahooWeather = function(latitude, longitude) {

  var subselect, neighbor, query, multi
    , options = {};

  subselect   = 'SELECT woeid FROM geo.placefinder WHERE text="'+latitude+','+longitude+'" AND gflags="R"';
  neighbor    = 'SELECT * FROM geo.placefinder WHERE text="'+latitude+','+longitude+'" AND gflags="R";';
  query       = 'SELECT * FROM weather.forecast WHERE woeid IN ('+subselect+') AND u="'+Global.latestConfig.weatherScale.toLowerCase()+'";';
  multi       = "SELECT * FROM yql.query.multi WHERE queries='"+query+" "+neighbor+"'";
  options.url = "https://query.yahooapis.com/v1/public/yql?format=json&q="+encodeURIComponent(multi)+"&nocache="+new Date().getTime();

  options.parse = function(response) {
      var sunrise, sunset, pubdate, locale;
      sunrise = response.query.results.results[0].channel.astronomy.sunrise;
      sunset  = response.query.results.results[0].channel.astronomy.sunset;
      pubdate = new Date(Date.parse(response.query.results.results[0].channel.item.pubDate));
      locale  = response.query.results.results[1].Result.neighborhood;
      if (locale === null) {
        locale = response.query.results.results[1].Result.city;
      }
      if (locale === null) {
        locale = 'unknown';
      }

      return {
        'condition':   parseInt(response.query.results.results[0].channel.item.condition.code),
        'temperature': parseInt(response.query.results.results[0].channel.item.condition.temp),
        'sunrise':     Date.parse(new Date().toDateString()+" "+sunrise) / 1000,
        'sunset':      Date.parse(new Date().toDateString()+" "+sunset) / 1000,
        'locale':      locale,
        'pubdate':     pubdate.getHours()+':'+('0'+pubdate.getMinutes()).slice(-2),
        'tzoffset':    new Date().getTimezoneOffset() * 60,
        'service':     Global.latestConfig.weatherService,
        'scale':       Global.latestConfig.weatherScale,
        'debug':       Global.latestConfig.debugEnabled   ? 1 : 0,
        'battery':     Global.latestConfig.batteryEnabled ? 1 : 0
      };
  };

  fetchWeather(options); 
};

var fetchOpenWeather = function(latitude, longitude) {

  var options = {};
  options.url = "http://api.openweathermap.org/data/2.5/weather?lat=" + latitude + "&lon=" + longitude + "&cnt=1";

  options.parse = function(response) {
      var temperature, sunrise, sunset, condition, pubdate;

      var tempResult = response.main.temp;
      if (Global.latestConfig.weatherScale === 'C') {
        // Convert temperature to Celsius
        temperature = Math.round(tempResult - 273.15);
      } 
      else {
        // Otherwise, convert temperature to Fahrenheit 
        temperature = Math.round(((tempResult - 273.15) * 1.8) + 32);
      }

      condition = response.weather[0].id;
      sunrise   = response.sys.sunrise;
      sunset    = response.sys.sunset;
      pubdate   = new Date(response.dt*1000); 

      return {
        'condition':   condition,
        'temperature': temperature, 
        'sunrise':     sunrise,
        'sunset':      sunset,
        'locale':      response.name,
        'pubdate':     pubdate.getHours()+':'+('0'+pubdate.getMinutes()).slice(-2),
        'tzoffset':    new Date().getTimezoneOffset() * 60,
        'service':     Global.latestConfig.weatherService,
        'scale':       Global.latestConfig.weatherScale,
        'debug':       Global.latestConfig.debugEnabled   ? 1 : 0,
        'battery':     Global.latestConfig.batteryEnabled ? 1 : 0
      };
  };
  fetchWeather(options);
};

var fetchWeather = function(options) {

  console.log('URL: ' + options.url);

  getJson(options.url, function(err, response) {

    try {
      if (err) {
        throw err;
      }

      var weather = options.parse(response);

      console.log('Weather Data: ' + JSON.stringify(weather));

      Pebble.sendAppMessage(weather, ack, function(e){
        nack(weather);
      });
      postDebugMessage(weather);
    
    } catch (e) {
      console.warn("Could not find weather data in response: " + e.message);
      var error = { "error": "HTTP Error" };
      Pebble.sendAppMessage(error, ack, nack);
      postDebugMessage(error);
    }
    Global.updateInProgress = false;
  });
};

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
};

var postDebugMessage = function (data) {
  if (!Global.externalDebug || EXTERNAL_DEBUG_URL === null) {
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
};

var post = function(url, data) {
  try {
    var req = new XMLHttpRequest();
    req.open('POST', url, true);
    req.send(data);
  } catch(e) {
    console.warn('POST failed: ' + e.message);
  }
};

var serialize = function(obj) {
  var str = [];
  for(var p in obj)
    if (obj.hasOwnProperty(p)) {
      str.push(encodeURIComponent(p) + "=" + encodeURIComponent(obj[p]));
    }
  return str.join("&");
}