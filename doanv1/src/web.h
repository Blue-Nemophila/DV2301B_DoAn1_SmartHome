#pragma once
#include <WebServer.h>

extern WebServer server;

const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 Smart Home</title>

<style>
:root {
  --bg: #0b0f1a;
  --card: rgba(255,255,255,0.08);
  --glass: blur(10px);
  --primary: #00f2ff;
  --success: #00ff9d;
  --danger: #ff4d4d;
  --text: #eaeaea;
  --muted: #9aa4b2;
}

* {
  box-sizing: border-box;
  font-family: 'Segoe UI', sans-serif;
}

body {
  margin: 0;
  background: radial-gradient(circle at top, #1a2340, var(--bg));
  color: var(--text);
}

header {
  padding: 20px;
  text-align: center;
  font-size: 26px;
  font-weight: bold;
  letter-spacing: 1px;
}

.dashboard {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
  gap: 18px;
  padding: 20px;
}

.card {
  background: var(--card);
  backdrop-filter: var(--glass);
  border-radius: 18px;
  padding: 18px;
  box-shadow: 0 20px 40px rgba(0,0,0,0.4);
}

.card h3 {
  margin: 0 0 10px;
  font-weight: 600;
}

.status {
  font-size: 18px;
  font-weight: bold;
}

.on { color: var(--success); }
.off { color: var(--danger); }

button {
  width: 100%;
  margin-top: 10px;
  padding: 12px;
  border-radius: 12px;
  border: none;
  cursor: pointer;
  background: linear-gradient(135deg, var(--primary), #0077ff);
  color: #000;
  font-weight: 600;
  font-size: 15px;
}

button:active {
  transform: scale(0.97);
}

.env {
  font-size: 32px;
  font-weight: bold;
}

.env span {
  font-size: 16px;
  color: var(--muted);
}

.uid {
  background: #000;
  padding: 10px;
  border-radius: 10px;
  font-family: monospace;
  text-align: center;
  letter-spacing: 1px;
}

footer {
  text-align: center;
  padding: 15px;
  font-size: 13px;
  color: var(--muted);
}
</style>
</head>

<body>

<header>🏠 ESP32 SMART HOME</header>

<div class="dashboard">

  <div class="card">
    <h3>💡 Lighting</h3>
    <p>LED 1: <span id="led" class="status">---</span></p>
    <button onclick="toggle('/led/toggle')">Toggle LED 1</button>

    <p>LED 2: <span id="led2" class="status">---</span></p>
    <button onclick="toggle('/led2/toggle')">Toggle LED 2</button>
  </div>

  <div class="card">
    <h3>🌀 Fan Control</h3>
    <p>Fan: <span id="fan" class="status">---</span></p>
    <p>Auto Mode: <span id="auto" class="status">---</span></p>
    <button onclick="toggle('/fan/toggle')">Toggle Fan</button>
    <button onclick="toggle('/auto/toggle')">Toggle Auto</button>
  </div>

  <div class="card">
    <h3>🌡 Environment</h3>
    <div class="env"><span>Temp</span><br><span id="temp">--</span> °C</div>
    <div class="env"><span>Humidity</span><br><span id="hum">--</span> %</div>
  </div>

  <div class="card">
    <h3>🚪 Door Access</h3>
    <p>Status: <span id="door" class="status">---</span></p>
    <button onclick="toggle('/door/open')">OPEN DOOR</button>
    <p style="margin-top:12px;">Last RFID</p>
    <div class="uid" id="rfid">---</div>
  </div>

</div>

<footer>
ESP32 Smart Home Dashboard • Real-time Control
</footer>

<script>
function toggle(url) {
  fetch(url, { method: 'POST' })
    .then(() => updateStatus());
}

function setBool(id, val) {
  const el = document.getElementById(id);
  el.textContent = val ? "ON" : "OFF";
  el.className = "status " + (val ? "on" : "off");
}

function updateStatus() {
  fetch('/status')
    .then(r => r.json())
    .then(d => {
      setBool('led', d.led);
      setBool('led2', d.led2);
      setBool('fan', d.fan);
      setBool('auto', d.auto);
      setBool('door', d.door);
      temp.textContent = d.temp;
      hum.textContent = d.hum;
      rfid.textContent = d.rfid;
    });
}

setInterval(updateStatus, 1000);
updateStatus();
</script>

</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

void setupWeb() {
  server.on("/", HTTP_GET, handleRoot);
}
