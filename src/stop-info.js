var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  //var url = 'http://192.168.1.15:16000/?lon=23.773009&lat=61.498434'; // Rautatieasema
  //var url = 'http://192.168.1.15:16000/?lon=23.8498138&lat=61.4514546'; // Kapina
  var url = 'http://192.168.1.15:16000/?lat=' + pos.coords.latitude + '&lon=' + pos.coords.longitude; // Real
  
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with bus stop info
      var stops = JSON.parse(responseText);
      
      if (stops.length === 0) {
        console.log('No stops in range!');
        return;
      }
      
      var closestStop = stops[0];
      console.log('Closest stop is', closestStop.name, ',', closestStop.dist, 'meters away, with', closestStop.buses.length, 'buses departing');
    
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

function locationError(err) {
  console.log('Error requesting location!');
}

function getStopInfo() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');

    // Get the initial stop info
    getStopInfo();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getStopInfo();
  }
);