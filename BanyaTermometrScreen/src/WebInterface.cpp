#include "WebInterface.h"

#include "AppConfig.h"

namespace
{
const char CONTROL_PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html lang="ru">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Banya Thermometer Screen</title>
  <style>
    :root{color-scheme:dark;font-family:Arial,sans-serif;background:#101418;color:#eef3f7}
    body{margin:0 auto;padding:16px;max-width:980px}
    header{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:14px}
    h1{font-size:24px;margin:0}h2{font-size:17px;margin:0 0 10px}
    .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(270px,1fr));gap:12px}
    .card{border:1px solid #2b3945;border-radius:8px;padding:14px;background:#17202a}
    .wide{grid-column:1/-1}
    .row{display:flex;align-items:center;justify-content:space-between;gap:12px;margin:8px 0}
    .value{font-weight:700;text-align:right}.muted{opacity:.72;font-size:13px}
    .reading{font-size:34px;font-weight:700;margin:8px 0}.unit{font-size:18px;opacity:.7}
    .pill{border-radius:999px;padding:4px 9px;background:#2d3946}.on{background:#0d8f54}.off{background:#65313a}
    .fields{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}
    .field label{display:block;margin-bottom:5px;font-size:13px;opacity:.75}
    input,select{box-sizing:border-box;width:100%;padding:9px;border-radius:6px;border:1px solid #3f5368;background:#0f141a;color:#eef3f7}
    input[type=checkbox]{width:auto}.buttons{display:flex;flex-wrap:wrap;gap:8px;margin-top:12px}
    button{border:0;border-radius:6px;padding:10px 12px;background:#2b7cff;color:#fff;font-weight:700;cursor:pointer}
    button.warn{background:#a75a00}button:disabled{opacity:.5}
    #message,#mqttMessage,#displayMessage,#boostMessage{min-height:18px;margin-top:10px}
    @media(max-width:700px){.fields{grid-template-columns:1fr}.row{align-items:flex-start}}
  </style>
</head>
<body>
  <header><h1>Дублирующее табло бани</h1><span id="networkBadge" class="pill">...</span></header>
  <div class="grid">
    <section class="card">
      <h2>На табло</h2>
      <div class="reading"><span id="temperature">--</span> <span class="unit">°C</span></div>
      <div class="row"><span>Фактически</span><span id="rawTemperature" class="value">--</span></div>
      <div class="row"><span>Влажность</span><span id="humidity" class="value">--</span></div>
    </section>
    <section class="card">
      <h2>Источник</h2>
      <div class="row"><span>Активный</span><span id="activeSource" class="value">...</span></div>
      <div class="row"><span>Возраст данных</span><span id="sourceAge" class="value">...</span></div>
      <div class="row"><span>HA topics</span><span id="haFresh" class="value">...</span></div>
      <div class="row"><span>MQTT fallback</span><span id="mqttFresh" class="value">...</span></div>
      <div class="row"><span>ESP-NOW</span><span id="espnowFresh" class="value">...</span></div>
    </section>
    <section class="card">
      <h2>Связь</h2>
      <div class="row"><span>Wi-Fi</span><span id="wifiState" class="value">...</span></div>
      <div class="row"><span>SSID</span><span id="currentSsid" class="value">...</span></div>
      <div class="row"><span>IP</span><span id="networkIp" class="value">...</span></div>
      <div class="row"><span>MQTT HA</span><span id="mqttHaState" class="value">...</span></div>
      <div class="row"><span>MQTT источник</span><span id="mqttSourceState" class="value">...</span></div>
    </section>
    <section class="card">
      <h2>Система</h2>
      <div class="row"><span>Версия</span><span id="version" class="value">...</span></div>
      <div class="row"><span>Время работы</span><span id="uptime" class="value">...</span></div>
      <div class="row"><span>Свободная память</span><span id="heap" class="value">...</span></div>
      <div class="row"><span>Точка настройки</span><span id="setupSsid" class="value">...</span></div>
    </section>
    <section class="card">
      <h2>Завышение температуры</h2>
      <div class="row"><span>Включено</span><input id="boostEnabled" type="checkbox"></div>
      <div class="row"><span>Коэффициент</span><span id="boostFactorValue" class="value">...</span></div>
      <div class="field"><label for="boostFactor">0.25 даст 100°C -> 120°C</label><input id="boostFactor" type="range" min="0" max="1" step="0.01"></div>
      <div class="field"><label for="boostFactorNumber">Точное значение</label><input id="boostFactorNumber" type="number" min="0" max="1" step="0.01"></div>
      <div class="buttons"><button id="saveBoostButton">Сохранить завышение</button></div>
      <div id="boostMessage" class="muted"></div>
    </section>
    <section class="card">
      <h2>Экран</h2>
      <div class="row"><span>Экран включен</span><input id="displayEnabled" type="checkbox"></div>
      <div class="row"><span>Яркость</span><span id="displayBrightnessValue" class="value">...</span></div>
      <div class="field"><label for="displayBrightness">MAX7219, 0-15</label><input id="displayBrightness" type="range" min="0" max="15" step="1"></div>
      <div class="field"><label for="displayBrightnessNumber">Точное значение</label><input id="displayBrightnessNumber" type="number" min="0" max="15" step="1"></div>
      <div class="buttons"><button id="saveDisplayButton">Сохранить яркость</button></div>
      <div id="displayMessage" class="muted"></div>
    </section>
    <section class="card wide">
      <h2>Настройка сети</h2>
      <div class="fields">
        <div class="field"><label for="networks">Доступные сети</label><select id="networks"><option value="">Нажмите «Сканировать»</option></select></div>
        <div class="field"><label for="ssid">Название сети</label><input id="ssid" autocomplete="off"></div>
        <div class="field"><label for="password">Пароль Wi-Fi</label><input id="password" type="password" autocomplete="new-password"></div>
      </div>
      <div class="row"><span>Получать адрес автоматически (DHCP)</span><input id="dhcp" type="checkbox" checked></div>
      <div id="staticFields" class="fields">
        <div class="field"><label for="staticIp">Статический IP</label><input id="staticIp" placeholder="192.168.1.51"></div>
        <div class="field"><label for="gateway">Шлюз</label><input id="gateway" placeholder="192.168.1.1"></div>
        <div class="field"><label for="subnet">Маска</label><input id="subnet" placeholder="255.255.255.0"></div>
        <div class="field"><label for="dns1">DNS 1</label><input id="dns1" placeholder="192.168.1.1"></div>
        <div class="field"><label for="dns2">DNS 2</label><input id="dns2" placeholder="1.1.1.1"></div>
      </div>
      <div class="buttons">
        <button id="scanButton">Сканировать</button>
        <button id="saveButton">Сохранить Wi-Fi</button>
        <button id="resetButton" class="warn">Сеть из AppConfig.h</button>
      </div>
      <div id="message" class="muted"></div>
    </section>
    <section class="card wide">
      <h2>MQTT</h2>
      <div class="fields">
        <div class="field"><label for="mqttServer">Сервер</label><input id="mqttServer" placeholder="192.168.1.100"></div>
        <div class="field"><label for="mqttPort">Порт</label><input id="mqttPort" type="number" min="1" max="65535" step="1"></div>
        <div class="field"><label for="mqttUser">Пользователь</label><input id="mqttUser" autocomplete="off"></div>
        <div class="field"><label for="mqttPassword">Пароль MQTT</label><input id="mqttPassword" type="password" autocomplete="new-password"></div>
      </div>
      <div class="row"><span>Очистить сохраненный пароль MQTT</span><input id="mqttClearPassword" type="checkbox"></div>
      <div class="row"><span>Состояние</span><span id="mqttDetails" class="value">...</span></div>
      <div class="buttons">
        <button id="saveMqttButton">Сохранить MQTT</button>
        <button id="resetMqttButton" class="warn">MQTT из AppConfig.h</button>
      </div>
      <div id="mqttMessage" class="muted"></div>
    </section>
  </div>
  <script>
    const el=id=>document.getElementById(id);
    let wifiEditing=false,wifiLoaded=false,mqttEditing=false,mqttLoaded=false,displayEditing=false,displayLoaded=false,boostEditing=false,boostLoaded=false;
    ['ssid','password','dhcp','staticIp','gateway','subnet','dns1','dns2'].forEach(id=>el(id).addEventListener('input',()=>wifiEditing=true));
    ['mqttServer','mqttPort','mqttUser','mqttPassword','mqttClearPassword'].forEach(id=>el(id).addEventListener('input',()=>mqttEditing=true));
    ['displayBrightness','displayBrightnessNumber','displayEnabled'].forEach(id=>el(id).addEventListener('input',()=>displayEditing=true));
    ['boostEnabled','boostFactor','boostFactorNumber'].forEach(id=>el(id).addEventListener('input',()=>boostEditing=true));
    el('networks').addEventListener('change',()=>{if(el('networks').value){el('ssid').value=el('networks').value;wifiEditing=true}});
    el('dhcp').addEventListener('change',toggleStatic);
    el('scanButton').addEventListener('click',scanWifi);
    el('saveButton').addEventListener('click',saveWifi);
    el('resetButton').addEventListener('click',resetWifi);
    el('saveMqttButton').addEventListener('click',saveMqtt);
    el('resetMqttButton').addEventListener('click',resetMqtt);
    el('saveDisplayButton').addEventListener('click',saveDisplay);
    el('saveBoostButton').addEventListener('click',saveBoost);
    el('displayBrightness').addEventListener('input',()=>syncDisplay(el('displayBrightness').value));
    el('displayBrightnessNumber').addEventListener('input',()=>syncDisplay(el('displayBrightnessNumber').value));
    el('boostFactor').addEventListener('input',()=>syncBoost(el('boostFactor').value));
    el('boostFactorNumber').addEventListener('input',()=>syncBoost(el('boostFactorNumber').value));
    function toggleStatic(){const disabled=el('dhcp').checked;['staticIp','gateway','subnet','dns1','dns2'].forEach(id=>el(id).disabled=disabled)}
    function formatDuration(seconds){const h=Math.floor(seconds/3600);const m=Math.floor((seconds%3600)/60);return h+' ч '+m+' мин'}
    function age(ms){return ms>1000000000?'--':Math.round(ms/1000)+' с'}
    function yes(v){return v?'да':'нет'}
    function syncDisplay(v){el('displayBrightness').value=v;el('displayBrightnessNumber').value=v;el('displayBrightnessValue').textContent=v;displayEditing=true}
    function syncBoost(v){const n=parseFloat(v||'0').toFixed(2);el('boostFactor').value=n;el('boostFactorNumber').value=n;el('boostFactorValue').textContent=n;boostEditing=true}
    function loadWifi(n){if(wifiEditing||wifiLoaded)return;wifiLoaded=true;el('ssid').value=n.configuredSsid||'';el('dhcp').checked=!n.static;el('staticIp').value=n.config.ip==='0.0.0.0'?'':n.config.ip;el('gateway').value=n.config.gateway==='0.0.0.0'?'':n.config.gateway;el('subnet').value=n.config.subnet==='0.0.0.0'?'':n.config.subnet;el('dns1').value=n.config.dns1==='0.0.0.0'?'':n.config.dns1;el('dns2').value=n.config.dns2==='0.0.0.0'?'':n.config.dns2;toggleStatic()}
    function loadMqtt(m){if(mqttEditing||mqttLoaded)return;mqttLoaded=true;el('mqttServer').value=m.server||'';el('mqttPort').value=m.port||1883;el('mqttUser').value=m.user||'';el('mqttPassword').value='';el('mqttClearPassword').checked=false}
    function loadDisplay(d){if(displayEditing||displayLoaded)return;displayLoaded=true;el('displayEnabled').checked=d.enabled;el('displayBrightness').min=d.min;el('displayBrightness').max=d.max;el('displayBrightnessNumber').min=d.min;el('displayBrightnessNumber').max=d.max;syncDisplay(d.brightness);displayEditing=false}
    function loadBoost(b){if(boostEditing||boostLoaded)return;boostLoaded=true;el('boostEnabled').checked=b.enabled;el('boostFactor').min=b.min;el('boostFactor').max=b.max;el('boostFactor').step=b.step;el('boostFactorNumber').min=b.min;el('boostFactorNumber').max=b.max;el('boostFactorNumber').step=b.step;syncBoost(b.factor);boostEditing=false}
    async function refresh(){
      try{
        const d=await(await fetch('/api/status')).json();
        el('temperature').textContent=d.reading.valid?d.reading.temperature.toFixed(1):'--';
        el('rawTemperature').textContent=d.reading.valid?d.reading.rawTemperature.toFixed(1)+' °C':'--';
        el('humidity').textContent=d.reading.valid?d.reading.humidity.toFixed(1)+' %':'--';
        el('activeSource').textContent=d.reading.source;
        el('sourceAge').textContent=d.reading.valid?age(d.reading.ageMs):'--';
        el('haFresh').textContent=yes(d.sources.ha.fresh)+' / '+age(d.sources.ha.ageMs);
        el('mqttFresh').textContent=yes(d.sources.mqtt.fresh)+' / '+age(d.sources.mqtt.ageMs);
        el('espnowFresh').textContent=(d.sources.espnow.ready?'ready, ':'off, ')+yes(d.sources.espnow.fresh)+' / '+age(d.sources.espnow.ageMs);
        el('wifiState').textContent=d.network.connected?'подключен':'точка настройки';
        el('currentSsid').textContent=d.network.connected?d.network.ssid:d.network.setupSsid;
        el('networkIp').textContent=d.network.ip;
        el('mqttHaState').textContent=d.mqtt.haConnected?'подключен':'нет связи';
        el('mqttSourceState').textContent=d.mqtt.sourceConnected?'подключен':'нет связи';
        el('mqttDetails').textContent=d.mqtt.server+':'+d.mqtt.port+(d.mqtt.passwordSet?' / пароль задан':' / без пароля');
        el('version').textContent=d.system.version;
        el('uptime').textContent=formatDuration(d.system.uptimeSec);
        el('heap').textContent=Math.round(d.system.freeHeap/1024)+' KB';
        el('setupSsid').textContent=d.network.setupSsid;
        el('networkBadge').textContent=d.network.connected?'Wi-Fi подключен':'Точка настройки';
        el('networkBadge').className='pill '+(d.network.connected?'on':'off');
        loadWifi(d.network);loadMqtt(d.mqtt);loadDisplay(d.display);loadBoost(d.boost)
      }catch(e){}
    }
    async function scanWifi(){
      el('scanButton').disabled=true;el('message').textContent='Сканирование...';
      try{
        for(let i=0;i<20;i++){
          const d=await(await fetch('/api/wifi/scan')).json();
          if(!d.scanning){
            el('networks').innerHTML='<option value="">Выберите сеть</option>';
            (d.networks||[]).forEach(n=>{const o=document.createElement('option');o.value=n.ssid;o.textContent=n.ssid+' ('+n.rssi+' dBm)'+(n.secure?' [пароль]':'');el('networks').appendChild(o)});
            el('message').textContent='Найдено сетей: '+d.networks.length;return
          }
          await new Promise(r=>setTimeout(r,500))
        }
      }catch(e){el('message').textContent='Ошибка сканирования'}finally{el('scanButton').disabled=false}
    }
    async function saveWifi(){
      if(!el('ssid').value){el('message').textContent='Укажите сеть';return}
      const body=new URLSearchParams({ssid:el('ssid').value,password:el('password').value,mode:el('dhcp').checked?'dhcp':'static',ip:el('staticIp').value,gateway:el('gateway').value,subnet:el('subnet').value,dns1:el('dns1').value,dns2:el('dns2').value});
      try{const r=await fetch('/api/wifi/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});el('message').textContent=r.ok?'Wi-Fi сохранен':'Ошибка Wi-Fi'}catch(e){el('message').textContent='Соединение прервано'}
    }
    async function resetWifi(){if(!confirm('Вернуть Wi-Fi из AppConfig.h?'))return;await fetch('/api/wifi/reset',{method:'POST'});wifiEditing=false;wifiLoaded=false;el('password').value='';setTimeout(refresh,1000)}
    async function saveMqtt(){
      const port=parseInt(el('mqttPort').value||'0',10);if(!el('mqttServer').value||!port){el('mqttMessage').textContent='Укажите сервер и порт';return}
      const body=new URLSearchParams({server:el('mqttServer').value,port:String(port),user:el('mqttUser').value,password:el('mqttPassword').value,clearPassword:el('mqttClearPassword').checked?'1':'0'});
      try{const r=await fetch('/api/mqtt/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});el('mqttMessage').textContent=r.ok?'MQTT сохранен':'Ошибка MQTT';mqttEditing=false;mqttLoaded=false;el('mqttPassword').value='';el('mqttClearPassword').checked=false;setTimeout(refresh,1200)}catch(e){el('mqttMessage').textContent='Ошибка MQTT'}
    }
    async function resetMqtt(){if(!confirm('Вернуть MQTT из AppConfig.h?'))return;await fetch('/api/mqtt/reset',{method:'POST'});mqttEditing=false;mqttLoaded=false;setTimeout(refresh,1000)}
    async function saveDisplay(){const body=new URLSearchParams({brightness:el('displayBrightnessNumber').value,enabled:el('displayEnabled').checked?'1':'0'});const r=await fetch('/api/display/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});el('displayMessage').textContent=r.ok?'Экран сохранен':'Ошибка экрана';displayEditing=false;displayLoaded=false;setTimeout(refresh,600)}
    async function saveBoost(){const body=new URLSearchParams({enabled:el('boostEnabled').checked?'1':'0',factor:el('boostFactorNumber').value});const r=await fetch('/api/boost/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});el('boostMessage').textContent=r.ok?'Настройки сохранены':'Ошибка сохранения';boostEditing=false;boostLoaded=false;setTimeout(refresh,600)}
    toggleStatic();refresh();setInterval(refresh,1000);
  </script>
</body>
</html>
)HTML";
}

WebInterface::WebInterface(NetworkManager &network,
                           RemoteClimateSource &source,
                           HomeAssistantBridge &homeAssistant,
                           MqttSettings &mqttSettings,
                           MatrixDisplay &display,
                           DisplaySettings &displaySettings,
                           TemperatureBoostSettings &boostSettings)
    : _server(80),
      _network(network),
      _source(source),
      _homeAssistant(homeAssistant),
      _mqttSettings(mqttSettings),
      _display(display),
      _displaySettings(displaySettings),
      _boostSettings(boostSettings)
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
  _server.on("/api/wifi/scan", HTTP_GET, [this]() { handleWifiScan(); });
  _server.on("/api/wifi/save", HTTP_POST, [this]() { handleWifiSave(); });
  _server.on("/api/wifi/reset", HTTP_POST, [this]() { handleWifiReset(); });
  _server.on("/api/mqtt/save", HTTP_POST, [this]() { handleMqttSave(); });
  _server.on("/api/mqtt/reset", HTTP_POST, [this]() { handleMqttReset(); });
  _server.on("/api/display/save", HTTP_POST, [this]() { handleDisplaySave(); });
  _server.on("/api/boost/save", HTTP_POST, [this]() { handleBoostSave(); });
  _server.onNotFound([this]() {
    _server.sendHeader("Location", "/", true);
    _server.send(302, "text/plain", "");
  });
}

void WebInterface::handleRoot()
{
  _server.send_P(200, "text/html; charset=utf-8", CONTROL_PAGE);
}

void WebInterface::handleStatus()
{
  const ClimateReading reading = _source.current(_boostSettings);
  String json;
  json.reserve(1800);
  json += "{\"network\":{\"connected\":";
  json += _network.connected() ? "true" : "false";
  json += ",\"provisioning\":";
  json += _network.provisioningMode() ? "true" : "false";
  json += ",\"static\":";
  json += _network.staticIpEnabled() ? "true" : "false";
  json += ",\"ip\":\"";
  json += _network.address().toString();
  json += "\",\"ssid\":\"";
  json += jsonEscape(_network.connectedSsid());
  json += "\",\"configuredSsid\":\"";
  json += jsonEscape(_network.configuredSsid());
  json += "\",\"setupSsid\":\"";
  json += jsonEscape(_network.setupApSsid());
  json += "\",\"rssi\":";
  json += String(_network.rssi());
  json += ",\"config\":{\"ip\":\"";
  json += _network.configuredIp().toString();
  json += "\",\"gateway\":\"";
  json += _network.configuredGateway().toString();
  json += "\",\"subnet\":\"";
  json += _network.configuredSubnet().toString();
  json += "\",\"dns1\":\"";
  json += _network.configuredDns1().toString();
  json += "\",\"dns2\":\"";
  json += _network.configuredDns2().toString();
  json += "\"}},\"mqtt\":{\"haConnected\":";
  json += _homeAssistant.connected() ? "true" : "false";
  json += ",\"sourceConnected\":";
  json += _source.mqttConnected() ? "true" : "false";
  json += ",\"server\":\"";
  json += jsonEscape(_mqttSettings.server());
  json += "\",\"port\":";
  json += String(_mqttSettings.port());
  json += ",\"user\":\"";
  json += jsonEscape(_mqttSettings.user());
  json += "\",\"passwordSet\":";
  json += _mqttSettings.passwordSet() ? "true" : "false";
  json += "},\"reading\":{\"valid\":";
  json += reading.valid ? "true" : "false";
  json += ",\"temperature\":";
  json += String(reading.displayTemperature, 2);
  json += ",\"rawTemperature\":";
  json += String(reading.rawTemperature, 2);
  json += ",\"humidity\":";
  json += String(reading.humidity, 2);
  json += ",\"source\":\"";
  json += climateSourceName(reading.source);
  json += "\",\"ageMs\":";
  json += String(reading.ageMs);
  json += "},\"sources\":{\"ha\":{\"fresh\":";
  json += _source.sourceFresh(ClimateSourceKind::HomeAssistant) ? "true" : "false";
  json += ",\"ageMs\":";
  json += String(_source.sourceAgeMs(ClimateSourceKind::HomeAssistant));
  json += "},\"mqtt\":{\"fresh\":";
  json += _source.sourceFresh(ClimateSourceKind::Mqtt) ? "true" : "false";
  json += ",\"ageMs\":";
  json += String(_source.sourceAgeMs(ClimateSourceKind::Mqtt));
  json += "},\"espnow\":{\"ready\":";
  json += _source.espNowReady() ? "true" : "false";
  json += ",\"fresh\":";
  json += _source.sourceFresh(ClimateSourceKind::EspNow) ? "true" : "false";
  json += ",\"ageMs\":";
  json += String(_source.sourceAgeMs(ClimateSourceKind::EspNow));
  json += "}},\"display\":{\"enabled\":";
  json += _display.enabled() ? "true" : "false";
  json += ",\"brightness\":";
  json += String(_display.intensity());
  json += ",\"min\":";
  json += String(_displaySettings.minBrightness());
  json += ",\"max\":";
  json += String(_displaySettings.maxBrightness());
  json += "},\"boost\":{\"enabled\":";
  json += _boostSettings.enabled() ? "true" : "false";
  json += ",\"factor\":";
  json += String(_boostSettings.factor(), 2);
  json += ",\"min\":";
  json += String(_boostSettings.minFactor(), 2);
  json += ",\"max\":";
  json += String(_boostSettings.maxFactor(), 2);
  json += ",\"step\":";
  json += String(Config::TEMPERATURE_BOOST_STEP, 2);
  json += ",\"baseline\":";
  json += String(_boostSettings.baseline(), 1);
  json += "},\"system\":{\"version\":\"";
  json += Config::SOFTWARE_VERSION;
  json += "\",\"uptimeSec\":";
  json += String(millis() / 1000UL);
  json += ",\"freeHeap\":";
  json += String(ESP.getFreeHeap());
  json += "}}";
  _server.send(200, "application/json", json);
}

void WebInterface::handleWifiScan()
{
  const int count = WiFi.scanComplete();
  if (count == WIFI_SCAN_RUNNING)
  {
    _server.send(200, "application/json", "{\"scanning\":true,\"networks\":[]}");
    return;
  }
  if (count == WIFI_SCAN_FAILED)
  {
    WiFi.scanNetworks(true, true);
    _server.send(200, "application/json", "{\"scanning\":true,\"networks\":[]}");
    return;
  }

  String json = "{\"scanning\":false,\"networks\":[";
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
  json += "]}";
  WiFi.scanDelete();
  _server.send(200, "application/json", json);
}

void WebInterface::handleWifiSave()
{
  if (!_server.hasArg("ssid") || _server.arg("ssid").isEmpty())
  {
    _server.send(400, "application/json", "{\"ok\":false,\"error\":\"ssid_required\"}");
    return;
  }

  const bool useStatic = _server.arg("mode") == "static";
  IPAddress localIp;
  IPAddress gateway;
  IPAddress subnet;
  IPAddress dns1;
  IPAddress dns2;

  if (useStatic)
  {
    if (!parseIp(_server.arg("ip"), localIp) ||
        !parseIp(_server.arg("gateway"), gateway) ||
        !parseIp(_server.arg("subnet"), subnet) ||
        localIp == IPAddress() ||
        gateway == IPAddress() ||
        subnet == IPAddress())
    {
      _server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid_static_ip\"}");
      return;
    }

    if (!parseIp(_server.arg("dns1"), dns1))
    {
      dns1 = gateway;
    }
    parseIp(_server.arg("dns2"), dns2);
  }

  const bool saved = _network.saveSettings(
      _server.arg("ssid"),
      _server.arg("password"),
      useStatic,
      localIp,
      gateway,
      subnet,
      dns1,
      dns2);

  if (!saved)
  {
    _server.send(500, "application/json", "{\"ok\":false,\"error\":\"save_failed\"}");
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
  if (!_server.hasArg("server") || _server.arg("server").isEmpty())
  {
    _server.send(400, "application/json", "{\"ok\":false,\"error\":\"server_required\"}");
    return;
  }
  const int port = _server.arg("port").toInt();
  if (port <= 0 || port > 65535)
  {
    _server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid_port\"}");
    return;
  }

  const String password = _server.arg("clearPassword") == "1"
                              ? String()
                              : (_server.arg("password").isEmpty()
                                     ? _mqttSettings.password()
                                     : _server.arg("password"));
  const bool saved = _mqttSettings.saveSettings(
      _server.arg("server"),
      static_cast<uint16_t>(port),
      _server.arg("user"),
      password);
  if (!saved)
  {
    _server.send(500, "application/json", "{\"ok\":false,\"error\":\"save_failed\"}");
    return;
  }

  _homeAssistant.reconfigure(
      _mqttSettings.server().c_str(),
      _mqttSettings.port(),
      _mqttSettings.user().isEmpty() ? nullptr : _mqttSettings.user().c_str(),
      _mqttSettings.password().isEmpty() ? nullptr : _mqttSettings.password().c_str());
  _source.reconfigure(
      _mqttSettings.server().c_str(),
      _mqttSettings.port(),
      _mqttSettings.user().isEmpty() ? nullptr : _mqttSettings.user().c_str(),
      _mqttSettings.password().isEmpty() ? nullptr : _mqttSettings.password().c_str());
  sendOk();
}

void WebInterface::handleMqttReset()
{
  _mqttSettings.restoreDefaults();
  _homeAssistant.reconfigure(
      _mqttSettings.server().c_str(),
      _mqttSettings.port(),
      _mqttSettings.user().isEmpty() ? nullptr : _mqttSettings.user().c_str(),
      _mqttSettings.password().isEmpty() ? nullptr : _mqttSettings.password().c_str());
  _source.reconfigure(
      _mqttSettings.server().c_str(),
      _mqttSettings.port(),
      _mqttSettings.user().isEmpty() ? nullptr : _mqttSettings.user().c_str(),
      _mqttSettings.password().isEmpty() ? nullptr : _mqttSettings.password().c_str());
  sendOk();
}

void WebInterface::handleDisplaySave()
{
  const int brightness = _server.arg("brightness").toInt();
  if (brightness < _displaySettings.minBrightness() ||
      brightness > _displaySettings.maxBrightness())
  {
    _server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid_brightness\"}");
    return;
  }

  const bool enabled = !_server.hasArg("enabled") || _server.arg("enabled") == "1";
  if (!_displaySettings.save(static_cast<uint16_t>(brightness), enabled))
  {
    _server.send(500, "application/json", "{\"ok\":false,\"error\":\"save_failed\"}");
    return;
  }
  _display.setEnabled(_displaySettings.enabled());
  _display.setIntensity(static_cast<uint8_t>(_displaySettings.brightness()));
  sendOk();
}

void WebInterface::handleBoostSave()
{
  const bool enabled = _server.arg("enabled") == "1";
  const float factor = _server.arg("factor").toFloat();
  if (!_boostSettings.save(enabled, factor))
  {
    _server.send(500, "application/json", "{\"ok\":false,\"error\":\"save_failed\"}");
    return;
  }
  sendOk();
}

void WebInterface::sendOk()
{
  _server.send(200, "application/json", "{\"ok\":true}");
}

bool WebInterface::parseIp(const String &text, IPAddress &address)
{
  if (text.isEmpty())
  {
    address = IPAddress();
    return false;
  }
  return address.fromString(text);
}

String WebInterface::jsonEscape(const String &value)
{
  String escaped;
  escaped.reserve(value.length() + 8);
  for (size_t i = 0; i < value.length(); i++)
  {
    const char c = value[i];
    if (c == '"' || c == '\\')
    {
      escaped += '\\';
      escaped += c;
    }
    else if (c == '\n' || c == '\r')
    {
      escaped += ' ';
    }
    else
    {
      escaped += c;
    }
  }
  return escaped;
}
