var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  var stopName = "Hermiankatu 7";
  
  var buses = [
    { id: 13, dest: 'Alajärvi matkatie', min1: 88, min2: 25 },
    { id: 20, dest: 'Pöönikintori', min1: 13, min2: 32 }
  ];

  //"13,Ylöjärvi Matkatie,8,25\n20,Pyynikintori,3,32\n";
  var csvBuses = buses.map(function (bus) {
    return bus.id + ',' + bus.dest.substr(0, 64) + ',' + bus.min1 + ',' + bus.min2 + '\n';
  }).join('');
  
  // Assemble dictionary using our keys
  var dictionary = {
    'KEY_STOP_NAME': stopName,
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