#include "WebInterface.h"

#include <WiFi.h>

#include "AppConfig.h"

namespace
{
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="ru">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Water Control</title>
  <style>
    :root{color-scheme:dark;--bg:#0f172a;--card:#111827;--muted:#94a3b8;--text:#e5e7eb;--accent:#38bdf8;--ok:#22c55e;--bad:#ef4444;--line:#1f2937}
    *{box-sizing:border-box}body{margin:0;font-family:system-ui,-apple-system,Segoe UI,sans-serif;background:linear-gradient(135deg,#0f172a,#020617);color:var(--text)}
    main{width:min(1100px,100%);margin:auto;padding:22px}h1{margin:0 0 6px;font-size:30px}h2{margin:0 0 14px;font-size:19px}.muted{color:var(--muted)}
    .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:14px;margin-top:16px}.card{background:rgba(17,24,39,.88);border:1px solid var(--line);border-radius:18px;padding:16px;box-shadow:0 16px 40px rgba(0,0,0,.22)}
    .value{font-size:28px;font-weight:700;margin:8px 0}.row{display:flex;gap:10px;align-items:center;justify-content:space-between;margin:8px 0}.pill{padding:4px 10px;border-radius:999px;background:#334155;color:#cbd5e1;font-size:13px}.pill.on{background:rgba(34,197,94,.18);color:#86efac}.pill.off{background:rgba(239,68,68,.18);color:#fca5a5}
    button{border:0;border-radius:12px;padding:10px 12px;background:var(--accent);color:#082f49;font-weight:700;cursor:pointer}button.secondary{background:#334155;color:#e5e7eb}button.danger{background:#ef4444;color:white}
    input,select{width:100%;border:1px solid #334155;border-radius:12px;background:#020617;color:#e5e7eb;padding:10px;margin:5px 0 10px}label{font-size:13px;color:#cbd5e1}.actions{display:flex;gap:8px;flex-wrap:wrap}.small{font-size:13px}.status{margin-top:12px;min-height:20px;color:#bae6fd}
  </style>
</head>
<body>
<main>
  <h1>Water Control</h1>
  <div class="muted" id="device">loading...</div>

  <section class="grid">
    <div class="card">
      <h2>Relays</h2>
      <div id="relays"></div>
    </div>

    <div class="card">
      <h2>Sensors</h2>
      <div class="row"><span>DHT</span><span id="climateState" class="pill">unknown</span></div>
      <div class="value"><span id="temperature">--</span> C</div>
      <div class="muted">Humidity: <span id="humidity">--</span> %</div>
      <hr style="border-color:#1f2937;border-style:solid;border-width:1px 0 0;margin:14px 0">
      <div class="row"><span>BME280 pressure</span><span id="pressureState" class="pill">unknown</span></div>
      <div class="value"><span id="pressure">--</span> mmHg</div>
    </div>

    <div class="card">
      <h2>Network</h2>
      <div class="row"><span>Wi-Fi</span><span id="wifiState" class="pill">unknown</span></div>
      <div class="muted">SSID: <span id="wifiSsid">--</span></div>
      <div class="muted">IP: <span id="wifiIp">--</span></div>
      <div class="muted">RSSI: <span id="wifiRssi">--</span></div>
      <div class="muted">Setup AP: <span id="setupAp">--</span></div>
    </div>

    <div class="card">
      <h2>MQTT / HA</h2>
      <div class="row"><span>Home Assistant</span><span id="haState" class="pill">unknown</span></div>
      <div class="muted">Server: <span id="mqttServerView">--</span></div>
      <div class="muted">Port: <span id="mqttPortView">--</span></div>
    </div>
  </section>

  <section class="grid">
    <form class="card" id="wifiForm">
      <h2>Wi-Fi settings</h2>
      <label>Network</label>
      <div class="actions">
        <select id="ssidSelect"></select>
        <button type="button" class="secondary" onclick="scanWifi()">Scan</button>
      </div>
      <input id="wifiSsidInput" name="ssid" placeholder="SSID">
      <label>Password</label>
      <input id="wifiPasswordInput" name="password" type="password" placeholder="leave empty to keep current">
      <label><input id="staticIpInput" type="checkbox" style="width:auto;margin-right:8px"> Static IP</label>
      <input id="localIpInput" placeholder="Local IP">
      <input id="gatewayInput" placeholder="Gateway">
      <input id="subnetInput" placeholder="Subnet">
      <input id="dns1Input" placeholder="DNS 1">
      <input id="dns2Input" placeholder="DNS 2">
      <div class="actions">
        <button type="submit">Save Wi-Fi</button>
        <button type="button" class="danger" onclick="resetWifi()">Defaults</button>
      </div>
    </form>

    <form class="card" id="mqttForm">
      <h2>MQTT settings</h2>
      <label>Server</label>
      <input id="mqttServerInput" placeholder="192.168.1.100">
      <label>Port</label>
      <input id="mqttPortInput" type="number" min="1" max="65535" placeholder="1883">
      <label>User</label>
      <input id="mqttUserInput" placeholder="user">
      <label>Password</label>
      <input id="mqttPasswordInput" type="password" placeholder="leave empty to keep current">
      <div class="actions">
        <button type="submit">Save MQTT</button>
        <button type="button" class="danger" onclick="resetMqtt()">Defaults</button>
      </div>
    </form>
  </section>
  <div class="status" id="status"></div>
</main>
<script>
let formsReady=false;
const relayNames=["Valve 1","Valve 2","Pump","Reserve"];
function pill(el,on){el.className="pill "+(on?"on":"off");el.textContent=on?"online":"offline"}
function post(path,data){return fetch(path,{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded"},body:new URLSearchParams(data)}).then(async r=>{if(!r.ok)throw new Error(await r.text());return r.text()})}
function setStatus(text){document.getElementById("status").textContent=text}
function relayRow(r,i){return `<div class="row"><div><b>${relayNames[i]||("Relay "+(i+1))}</b><div class="small muted">GPIO ${r.pin}</div></div><span class="pill ${r.state?"on":"off"}">${r.state?"ON":"OFF"}</span></div><div class="actions" style="margin-bottom:12px"><button onclick="setRelay(${i},1)">On</button><button class="secondary" onclick="setRelay(${i},0)">Off</button><button class="secondary" onclick="toggleRelay(${i})">Toggle</button></div>`}
async function refresh(){
  const s=await fetch("/api/status").then(r=>r.json());
  document.getElementById("device").textContent=`${s.device.name} v${s.device.version}`;
  document.getElementById("relays").innerHTML=s.relays.map(relayRow).join("");
  pill(document.getElementById("climateState"),s.climate.valid);
  document.getElementById("temperature").textContent=s.climate.valid?s.climate.temperature.toFixed(1):"--";
  document.getElementById("humidity").textContent=s.climate.valid?s.climate.humidity.toFixed(1):"--";
  pill(document.getElementById("pressureState"),s.pressure.valid);
  document.getElementById("pressure").textContent=s.pressure.valid?s.pressure.value.toFixed(1):"--";
  pill(document.getElementById("wifiState"),s.network.connected);
  document.getElementById("wifiSsid").textContent=s.network.ssid||s.network.configuredSsid||"--";
  document.getElementById("wifiIp").textContent=s.network.ip;
  document.getElementById("wifiRssi").textContent=s.network.rssi;
  document.getElementById("setupAp").textContent=s.network.setupAp;
  pill(document.getElementById("haState"),s.mqtt.connected);
  document.getElementById("mqttServerView").textContent=s.mqtt.server;
  document.getElementById("mqttPortView").textContent=s.mqtt.port;
  if(!formsReady){
    formsReady=true;
    wifiSsidInput.value=s.network.configuredSsid||"";
    staticIpInput.checked=s.network.staticIp;
    localIpInput.value=s.network.configuredIp||"";
    gatewayInput.value=s.network.configuredGateway||"";
    subnetInput.value=s.network.configuredSubnet||"";
    dns1Input.value=s.network.configuredDns1||"";
    dns2Input.value=s.network.configuredDns2||"";
    mqttServerInput.value=s.mqtt.server||"";
    mqttPortInput.value=s.mqtt.port||1883;
    mqttUserInput.value=s.mqtt.user||"";
  }
}
async function setRelay(index,state){await post("/api/relay/set",{index,state});refresh()}
async function toggleRelay(index){await post("/api/relay/toggle",{index});refresh()}
async function scanWifi(){
  setStatus("Scanning Wi-Fi...");
  const networks=await fetch("/api/wifi/scan").then(r=>r.json());
  ssidSelect.innerHTML="<option value=''>select scanned network</option>"+networks.map(n=>`<option value="${n.ssid}">${n.ssid} (${n.rssi} dBm)</option>`).join("");
  setStatus(`Found ${networks.length} networks`);
}
ssidSelect.addEventListener("change",()=>{if(ssidSelect.value)wifiSsidInput.value=ssidSelect.value});
wifiForm.addEventListener("submit",async e=>{
  e.preventDefault();setStatus("Saving Wi-Fi...");
  await post("/api/wifi/save",{ssid:wifiSsidInput.value,password:wifiPasswordInput.value,static_ip:staticIpInput.checked?1:0,ip:localIpInput.value,gateway:gatewayInput.value,subnet:subnetInput.value,dns1:dns1Input.value,dns2:dns2Input.value});
  wifiPasswordInput.value="";setStatus("Wi-Fi saved");setTimeout(refresh,1200);
});
mqttForm.addEventListener("submit",async e=>{
  e.preventDefault();setStatus("Saving MQTT...");
  await post("/api/mqtt/save",{server:mqttServerInput.value,port:mqttPortInput.value,user:mqttUserInput.value,password:mqttPasswordInput.value});
  mqttPasswordInput.value="";setStatus("MQTT saved");setTimeout(refresh,1200);
});
async function resetWifi(){await post("/api/wifi/reset",{});formsReady=false;setStatus("Wi-Fi defaults restored");setTimeout(refresh,1200)}
async function resetMqtt(){await post("/api/mqtt/reset",{});formsReady=false;setStatus("MQTT defaults restored");setTimeout(refresh,1200)}
refresh();setInterval(refresh,3000);
</script>
</body>
</html>
)rawliteral";
}

WebInterface::WebInterface(NetworkManager &network,
                           RelayController &relays,
                           ClimateSensor &climate,
                           PressureSensor &pressure,
                           HomeAssistantBridge &homeAssistant,
                           MqttSettings &mqttSettings)
    : _server(80),
      _network(network),
      _relays(relays),
      _climate(climate),
      _pressure(pressure),
      _homeAssistant(homeAssistant),
      _mqttSettings(mqttSettings)
{
}

void WebInterface::begin()
{
  setupRoutes();
  _server.begin();
  Serial.println("Web interface started");
}

void WebInterface::loop()
{
  _server.handleClient();
}

void WebInterface::setupRoutes()
{
  _server.on("/", HTTP_GET, [this]() { handleRoot(); });
  _server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
  _server.on("/api/relay/set", HTTP_POST, [this]() { handleRelaySet(); });
  _server.on("/api/relay/toggle", HTTP_POST, [this]() { handleRelayToggle(); });
  _server.on("/api/wifi/scan", HTTP_GET, [this]() { handleWifiScan(); });
  _server.on("/api/wifi/save", HTTP_POST, [this]() { handleWifiSave(); });
  _server.on("/api/wifi/reset", HTTP_POST, [this]() { handleWifiReset(); });
  _server.on("/api/mqtt/save", HTTP_POST, [this]() { handleMqttSave(); });
  _server.on("/api/mqtt/reset", HTTP_POST, [this]() { handleMqttReset(); });
}

void WebInterface::handleRoot()
{
  _server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void WebInterface::handleStatus()
{
  String json;
  json.reserve(1400);
  json += "{\"device\":{\"name\":\"";
  json += jsonEscape(Config::DEVICE_NAME);
  json += "\",\"version\":\"";
  json += Config::SOFTWARE_VERSION;
  json += "\"},\"network\":{\"connected\":";
  json += _network.connected() ? "true" : "false";
  json += ",\"ssid\":\"";
  json += jsonEscape(_network.connectedSsid());
  json += "\",\"configuredSsid\":\"";
  json += jsonEscape(_network.configuredSsid());
  json += "\",\"ip\":\"";
  json += _network.address().toString();
  json += "\",\"gateway\":\"";
  json += _network.gateway().toString();
  json += "\",\"subnet\":\"";
  json += _network.subnet().toString();
  json += "\",\"dns1\":\"";
  json += _network.dns1().toString();
  json += "\",\"dns2\":\"";
  json += _network.dns2().toString();
  json += "\",\"configuredIp\":\"";
  json += _network.configuredIp().toString();
  json += "\",\"configuredGateway\":\"";
  json += _network.configuredGateway().toString();
  json += "\",\"configuredSubnet\":\"";
  json += _network.configuredSubnet().toString();
  json += "\",\"configuredDns1\":\"";
  json += _network.configuredDns1().toString();
  json += "\",\"configuredDns2\":\"";
  json += _network.configuredDns2().toString();
  json += "\",\"staticIp\":";
  json += _network.staticIpEnabled() ? "true" : "false";
  json += ",\"rssi\":";
  json += String(_network.rssi());
  json += ",\"setupAp\":\"";
  json += jsonEscape(_network.setupApSsid());
  json += "\"},\"mqtt\":{\"connected\":";
  json += _homeAssistant.connected() ? "true" : "false";
  json += ",\"server\":\"";
  json += jsonEscape(_mqttSettings.server());
  json += "\",\"port\":";
  json += String(_mqttSettings.port());
  json += ",\"user\":\"";
  json += jsonEscape(_mqttSettings.user());
  json += "\",\"passwordSet\":";
  json += _mqttSettings.passwordSet() ? "true" : "false";
  json += "},\"relays\":[";
  for (uint8_t i = 0; i < _relays.count(); i++)
  {
    if (i > 0)
    {
      json += ",";
    }
    json += "{\"index\":";
    json += String(i);
    json += ",\"name\":\"";
    json += relayName(i);
    json += "\",\"pin\":";
    json += String(Config::RELAY_PINS[i]);
    json += ",\"state\":";
    json += _relays.state(i) ? "true" : "false";
    json += "}";
  }
  json += "],\"climate\":{\"valid\":";
  json += _climate.valid() ? "true" : "false";
  json += ",\"temperature\":";
  json += _climate.valid() ? String(_climate.temperature(), 1) : "null";
  json += ",\"humidity\":";
  json += _climate.valid() ? String(_climate.humidity(), 1) : "null";
  json += ",\"error\":\"";
  json += jsonEscape(_climate.lastError());
  json += "\"},\"pressure\":{\"enabled\":";
  json += _pressure.enabled() ? "true" : "false";
  json += ",\"detected\":";
  json += _pressure.detected() ? "true" : "false";
  json += ",\"valid\":";
  json += _pressure.valid() ? "true" : "false";
  json += ",\"value\":";
  json += _pressure.valid() ? String(_pressure.pressureMmHg(), 1) : "null";
  json += ",\"error\":\"";
  json += jsonEscape(_pressure.lastError());
  json += "\"}}";

  _server.send(200, "application/json", json);
}

void WebInterface::handleRelaySet()
{
  if (!_server.hasArg("index") || !_server.hasArg("state"))
  {
    sendError(400, "missing index or state");
    return;
  }

  const uint8_t index = static_cast<uint8_t>(_server.arg("index").toInt());
  if (index >= _relays.count())
  {
    sendError(400, "invalid relay index");
    return;
  }

  _relays.set(index, _server.arg("state").toInt() != 0);
  sendOk();
}

void WebInterface::handleRelayToggle()
{
  if (!_server.hasArg("index"))
  {
    sendError(400, "missing index");
    return;
  }

  const uint8_t index = static_cast<uint8_t>(_server.arg("index").toInt());
  if (index >= _relays.count())
  {
    sendError(400, "invalid relay index");
    return;
  }

  _relays.toggle(index);
  sendOk();
}

void WebInterface::handleWifiScan()
{
  const int count = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < count; i++)
  {
    if (i > 0)
    {
      json += ",";
    }
    json += "{\"ssid\":\"";
    json += jsonEscape(WiFi.SSID(i));
    json += "\",\"rssi\":";
    json += String(WiFi.RSSI(i));
    json += ",\"secure\":";
    json += WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "false" : "true";
    json += "}";
  }
  json += "]";
  WiFi.scanDelete();
  _server.send(200, "application/json", json);
}

void WebInterface::handleWifiSave()
{
  const String ssid = _server.arg("ssid");
  const String password = _server.arg("password");
  const bool useStaticIp = _server.arg("static_ip") == "1";

  IPAddress localIp;
  IPAddress gateway;
  IPAddress subnet;
  IPAddress dns1;
  IPAddress dns2;

  if (useStaticIp)
  {
    if (!parseIp(_server.arg("ip"), localIp) ||
        !parseIp(_server.arg("gateway"), gateway) ||
        !parseIp(_server.arg("subnet"), subnet))
    {
      sendError(400, "invalid static IP settings");
      return;
    }

    if (!parseIp(_server.arg("dns1"), dns1))
    {
      dns1 = gateway;
    }
    parseIp(_server.arg("dns2"), dns2);
  }

  if (!_network.saveSettings(ssid, password, useStaticIp, localIp, gateway, subnet, dns1, dns2))
  {
    sendError(400, "cannot save Wi-Fi settings");
    return;
  }

  sendOk();
}

void WebInterface::handleWifiReset()
{
  _network.restoreDefaults();
  sendOk();
}

void WebInterface::handleMqttSave()
{
  const String server = _server.arg("server");
  const uint16_t port = static_cast<uint16_t>(_server.arg("port").toInt());
  const String user = _server.arg("user");
  String password = _server.arg("password");

  if (password.isEmpty() &&
      _mqttSettings.passwordSet() &&
      server == _mqttSettings.server() &&
      port == _mqttSettings.port() &&
      user == _mqttSettings.user())
  {
    password = _mqttSettings.password();
  }

  if (!_mqttSettings.saveSettings(server, port, user, password))
  {
    sendError(400, "cannot save MQTT settings");
    return;
  }

  _homeAssistant.reconfigure(_mqttSettings.server().c_str(),
                             _mqttSettings.port(),
                             _mqttSettings.user().isEmpty() ? nullptr : _mqttSettings.user().c_str(),
                             _mqttSettings.password().isEmpty() ? nullptr : _mqttSettings.password().c_str());
  sendOk();
}

void WebInterface::handleMqttReset()
{
  _mqttSettings.restoreDefaults();
  _homeAssistant.reconfigure(_mqttSettings.server().c_str(),
                             _mqttSettings.port(),
                             _mqttSettings.user().isEmpty() ? nullptr : _mqttSettings.user().c_str(),
                             _mqttSettings.password().isEmpty() ? nullptr : _mqttSettings.password().c_str());
  sendOk();
}

void WebInterface::sendOk()
{
  _server.send(200, "application/json", "{\"ok\":true}");
}

void WebInterface::sendError(uint16_t code, const char *message)
{
  String json = "{\"ok\":false,\"error\":\"";
  json += jsonEscape(message);
  json += "\"}";
  _server.send(code, "application/json", json);
}

String WebInterface::jsonEscape(const String &value)
{
  String escaped;
  escaped.reserve(value.length() + 8);
  for (size_t i = 0; i < value.length(); i++)
  {
    const char c = value[i];
    if (c == '\\' || c == '"')
    {
      escaped += '\\';
      escaped += c;
    }
    else if (c == '\n')
    {
      escaped += "\\n";
    }
    else if (c == '\r')
    {
      escaped += "\\r";
    }
    else
    {
      escaped += c;
    }
  }
  return escaped;
}

bool WebInterface::parseIp(const String &value, IPAddress &address)
{
  if (value.isEmpty())
  {
    return false;
  }
  return address.fromString(value);
}

const char *WebInterface::relayName(uint8_t index)
{
  switch (index)
  {
  case 0:
    return "Valve 1";
  case 1:
    return "Valve 2";
  case 2:
    return "Pump";
  case 3:
    return "Reserve";
  default:
    return "Relay";
  }
}
