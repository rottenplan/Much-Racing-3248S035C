#ifndef WEB_STATIC_H
#define WEB_STATIC_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>MuchRacing GPS</title>
  <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" 
     integrity="sha256-p4NxAoJBhIIN+hmNHrzRCf9tD/miZyoHS5obTRR9BMY=" 
     crossorigin=""/>
  <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js" 
     integrity="sha256-20nQCchB9co0qIjJZRGuk2/Z9VM+kNiyxNV1lvTlZBo=" 
     crossorigin=""></script>
  <style>
    body { background: #111; color: #eee; font-family: sans-serif; margin: 0; padding: 0; display: flex; flex-direction: column; height: 100vh; }
    #map { flex-grow: 1; min-height: 50%; width: 100%; z-index: 1; }
    #stats { background: #222; padding: 10px; display: grid; grid-template-columns: 1fr 1fr; gap: 10px; border-top: 2px solid #04DF00; }
    .card { background: #333; padding: 10px; border-radius: 8px; text-align: center; }
    .label { font-size: 12px; color: #aaa; }
    .value { font-size: 24px; font-weight: bold; color: #fff; }
    .unit { font-size: 12px; color: #04DF00; }
    .offline-warning { position: absolute; top: 10px; left: 50%; transform: translateX(-50%); background: rgba(255,0,0,0.8); padding: 5px 15px; border-radius: 20px; font-size: 12px; z-index: 1000; display: none; }
  </style>
</head>
<body>
  <div id="offline" class="offline-warning">No Internet - Map Tiles May Missing</div>
  <div style="background:#000; padding:5px; text-align:center; font-size:12px; color:#888;">
    Status: <span id="connection-status" style="color:yellow;">Connecting...</span>
  </div>
  <div id="map"></div>
  <div id="stats">
    <div class="card">
      <div class="label">SPEED</div>
      <div><span id="speed" class="value">0</span> <span class="unit">KM/H</span></div>
    </div>
    <div class="card">
      <div class="label">RPM</div>
      <div><span id="rpm" class="value">0</span> <span class="unit">RPM</span></div>
    </div>
    <div class="card">
      <div class="label">TRIP</div>
      <div><span id="trip" class="value">0000</span> <span class="unit">KM</span></div>
    </div>
    <div class="card">
      <div class="label">GPS SATS</div>
      <div><span id="sats" class="value">0</span> <span class="unit">SAT</span></div>
    </div>
  </div>

  <script>
    var map = null;
    var marker = null;
    var firstFix = false;

    // 1. Initialize Map safely (It might fail if CDN is unreachable)
    if (typeof L !== 'undefined') {
      try {
        map = L.map('map').setView([0, 0], 15);
        
        // Satelite View Layer
        L.tileLayer('https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
            attribution: 'Tiles &copy; Esri'
        }).addTo(map);

        marker = L.marker([0, 0]).addTo(map);
      } catch (e) {
        console.error("Map Init Failed:", e);
        document.getElementById('map').innerHTML = '<div style="display:flex;justify-content:center;align-items:center;height:100%;color:#888;">Map Unavailable (Offline)</div>';
      }
    } else {
       console.log("Leaflet library not loaded (Offline mode)");
       document.getElementById('map').innerHTML = '<div style="display:flex;justify-content:center;align-items:center;height:100%;color:#888;">Map Unavailable (No Internet for CDN)</div>';
       document.getElementById('offline').style.display = 'block';
    }

    // Check online status for map tiles
    if (!navigator.onLine) document.getElementById('offline').style.display = 'block';

    function updateStats() {
      fetch('/api/live')
        .then(response => {
            if (!response.ok) throw new Error("HTTP " + response.status);
            return response.json();
        })
        .then(data => {
          document.getElementById('connection-status').innerText = "Connected (Live)";
          document.getElementById('connection-status').style.color = "#04DF00"; // Green
          
          document.getElementById('speed').innerText = Math.round(data.speed);
          document.getElementById('rpm').innerText = data.rpm;
          document.getElementById('trip').innerText = data.trip.toFixed(0).padStart(4, '0');
          document.getElementById('sats').innerText = data.sats;

          // Map Update (Only if Map exists)
          if (map && marker && data.lat != 0 && data.lng != 0) {
            var newLatLng = new L.LatLng(data.lat, data.lng);
            marker.setLatLng(newLatLng);
            
            // Auto Follow
            if (!firstFix || document.activeElement !== map.getContainer()) {
                map.panTo(newLatLng);
                firstFix = true;
            }
          }
        })
        .catch(err => {
            console.error(err);
            document.getElementById('connection-status').innerText = "Err: " + err.message;
            document.getElementById('connection-status').style.color = "red";
        });
    }

    // Start Loop regardless of map status
    setInterval(updateStats, 500); // 2Hz Update
  </script>
</body>
</html>
)rawliteral";

#endif
