var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function throttle(callback, limit) {
    var wait = false;                 // Initially, we're not waiting
    return function () {              // We return a throttled function
        if (!wait) {                  // If we're not waiting
            callback.call();          // Execute users function
            wait = true;              // Prevent future invocations
            setTimeout(function () {  // After a period of time
                wait = false;         // And allow future invocations
            }, limit);
        }
    };
}

var lastPosition = null;

function refreshStopInfo() {
  //var url = 'http://ec2-54-194-210-241.eu-west-1.compute.amazonaws.com:16000/?lon=23.773009&lat=61.498434'; // Rautatieasema
  //var url = 'http://ec2-54-194-210-241.eu-west-1.compute.amazonaws.com:16000/?lon=23.8498138&lat=61.4514546'; // Kapina
  var url = 'http://ec2-54-194-210-241.eu-west-1.compute.amazonaws.com:16000/?lat=' + lastPosition.coords.latitude + '&lon=' + lastPosition.coords.longitude; // Real
  
  console.log('Getting stop info at ' + lastPosition.coords.latitude + ',' + lastPosition.coords.longitude);
  
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with bus stop info
      var stops = JSON.parse(responseText);
      
      if (stops.length === 0) {
        console.log('No stops in range!');
        return;
      }
      
      var closestStop = stops[0];
      console.log('Closest stop is ' + closestStop.name + ', ' + closestStop.dist + ' meters away, with ' + closestStop.buses.length + ' buses departing');
    
      //"13,Ylöjärvi Matkatie,8,25\n20,Pyynikintori,3,32\n";
      var csvBuses = closestStop.buses.slice(0, 8).map(function (bus) {
        return bus.id + ',' + bus.dest.substr(0, 64) + ',' + bus.min1 + ',' + bus.min2 + '\n';
      }).join('');
      
      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_STOP_NAME': closestStop.name,
        'KEY_BUSES': csvBuses
      };
    
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Stop info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending stop info to Pebble!');
        }
      );
    }      
  );
}

var refreshThrottled = throttle(refreshStopInfo, 10000);
var stationaryRefreshTimer = null;

function locationSuccess(pos) {
  console.log('New position ' + pos.coords.latitude + ',' + pos.coords.longitude);
  lastPosition = pos;
  refreshThrottled();
  
  // Update stop info every 30s, starting now, even if we don't get more location updates
  if (stationaryRefreshTimer !== null) {
    clearInterval(stationaryRefreshTimer);
  }
  stationaryRefreshTimer = setInterval(refreshThrottled, 30000);
}

function locationError(err) {
  console.log('Error requesting location!');
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');

    // Start tracking user position
    var watch = navigator.geolocation.watchPosition(
      locationSuccess,
      locationError,
      {timeout: 27000, maximumAge: 30000, enableHighAccuracy: true}
    );
  }
);