#include "WebInterface.h"

namespace
{
const char CONTROL_PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html lang="ru">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Door and Gates</title>
  <style>
    :root{color-scheme:dark;font-family:Arial,sans-serif;background:#101418;color:#eef3f7}
    body{margin:0 auto;padding:16px;max-width:980px}
    header{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:14px}
    h1{font-size:24px;margin:0} h2{font-size:17px;margin:0 0 10px}
    .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(270px,1fr));gap:12px}
    .card{border:1px solid #2b3945;border-radius:8px;padding:14px;background:#17202a}
    .wide{grid-column:1/-1}
    .row{display:flex;align-items:center;justify-content:space-between;gap:12px;margin:8px 0}
    .value{font-weight:700}.muted{opacity:.7;font-size:13px}
    .buttons{display:flex;flex-wrap:wrap;gap:8px;margin-top:12px}
    button,a.button{border:0;border-radius:6px;padding:10px 12px;background:#2b7cff;color:#fff;font-weight:700;cursor:pointer;text-decoration:none}
    button.stop{background:#747b84}button.warn{background:#a75a00}button.light{background:#0b8f62}
    input,select{width:120px;padding:8px;border-radius:6px;border:1px solid #3f5368;background:#0f141a;color:#eef3f7}
    input[type=checkbox]{width:auto}
    .pill{border-radius:999px;padding:4px 9px;background:#2d3946}.on{background:#0d8f54}.off{background:#65313a}
    .wifi-fields,.settings-fields{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}
    .field label{display:block;margin-bottom:5px;font-size:13px;opacity:.75}
    .field input,.field select{box-sizing:border-box;width:100%}
    #wifiMessage,#mqttMessage,#displayMessage{min-height:18px;margin-top:10px}
    @media(max-width:700px){.wifi-fields,.settings-fields{grid-template-columns:1fr}.row{align-items:flex-start}}
  </style>
</head>
<body>
  <header><h1>Контроллер ворот</h1><span id="networkBadge" class="pill">...</span></header>
  <div class="grid">
    <section class="card">
      <h2>Ворота</h2>
      <div class="row"><span>Состояние</span><span id="gateState" class="value">...</span></div>
      <div class="row"><span>Мотор</span><span id="gateMotor" class="value">...</span></div>
      <div class="buttons">
        <button onclick="post('/api/gate/open')">Открыть</button>
        <button onclick="post('/api/gate/close')">Закрыть</button>
        <button class="stop" onclick="post('/api/gate/stop')">Стоп</button>
      </div>
    </section>
    <section class="card">
      <h2>Калитка</h2>
      <div class="row"><span>Геркон</span><span id="wicketState" class="value">...</span></div>
      <div class="row"><span>Реле</span><span id="wicketRelay" class="value">...</span></div>
      <div class="buttons"><button onclick="post('/api/wicket/open')">Открыть калитку</button></div>
    </section>
    <section class="card">
      <h2>Освещение</h2>
      <div class="row"><span>Состояние</span><span id="lightState" class="value">...</span></div>
      <div class="buttons"><button class="light" onclick="post('/api/light/toggle')">Переключить</button></div>
    </section>
    <section class="card">
      <h2>SHT31</h2>
      <div class="row"><span>Температура</span><span id="temperature" class="value">...</span></div>
      <div class="row"><span>Влажность</span><span id="humidity" class="value">...</span></div>
    </section>
    <section class="card">
      <h2>Верхний MLX90393</h2>
      <div class="row"><span>Связь</span><span id="upperConnected" class="value">...</span></div>
      <div class="row"><span>Датчик</span><span id="upperSensorOk" class="value">...</span></div>
      <div class="row"><span>MAC</span><span id="upperMac" class="value">...</span></div>
      <div class="row"><span>Интерфейс</span><span id="upperTransport" class="value">...</span></div>
      <div class="row"><span>Последний пакет</span><span id="upperAge" class="value">...</span></div>
      <div class="row"><span>Ось</span><select id="upperAxis"><option>x</option><option>y</option><option>z</option></select></div>
      <div class="row"><span>Инверсия вверх/вниз</span><input id="upperInvert" type="checkbox"></div>
      <div class="row"><span>Чувствительность</span><input id="upperSensitivity" type="number" min="1" step="1"></div>
      <div class="row"><span>Направление</span><span id="upperDirection" class="value">...</span></div>
      <div class="row"><span>Движение</span><span id="upperMoving" class="value">...</span></div>
      <div class="row"><span>Значение</span><span id="upperValue" class="value">...</span></div>
      <div class="row"><span>Скорость</span><span id="upperVelocity" class="value">...</span></div>
      <div class="row"><span>Delta от базы</span><span id="upperDelta" class="value">...</span></div>
      <div class="buttons">
        <button onclick="saveSettings()">Сохранить</button>
        <button class="warn" onclick="post('/api/sensor/upper/baseline')">Запомнить базу</button>
      </div>
    </section>
    <section class="card">
      <h2>Нижний MLX90393</h2>
      <div class="row"><span>Связь</span><span id="lowerConnected" class="value">...</span></div>
      <div class="row"><span>Датчик</span><span id="lowerSensorOk" class="value">...</span></div>
      <div class="row"><span>MAC</span><span id="lowerMac" class="value">...</span></div>
      <div class="row"><span>Интерфейс</span><span id="lowerTransport" class="value">...</span></div>
      <div class="row"><span>Последний пакет</span><span id="lowerAge" class="value">...</span></div>
      <div class="row"><span>Ось</span><select id="lowerAxis"><option>x</option><option>y</option><option>z</option></select></div>
      <div class="row"><span>Инверсия вверх/вниз</span><input id="lowerInvert" type="checkbox"></div>
      <div class="row"><span>Чувствительность</span><input id="lowerSensitivity" type="number" min="1" step="1"></div>
      <div class="row"><span>Направление</span><span id="lowerDirection" class="value">...</span></div>
      <div class="row"><span>Движение</span><span id="lowerMoving" class="value">...</span></div>
      <div class="row"><span>Значение</span><span id="lowerValue" class="value">...</span></div>
      <div class="row"><span>Скорость</span><span id="lowerVelocity" class="value">...</span></div>
      <div class="row"><span>Delta от базы</span><span id="lowerDelta" class="value">...</span></div>
      <div class="buttons">
        <button onclick="saveSettings()">Сохранить</button>
        <button class="warn" onclick="post('/api/sensor/lower/baseline')">Запомнить базу</button>
      </div>
    </section>
    <section class="card wide">
      <h2>Wi-Fi</h2>
      <div class="row"><span>Режим</span><span id="networkMode" class="value">...</span></div>
      <div class="row"><span>Текущая сеть</span><span id="currentSsid" class="value">...</span></div>
      <div class="row"><span>IP-адрес</span><span id="networkIp" class="value">...</span></div>
      <div class="wifi-fields">
        <div class="field">
          <label for="wifiNetworks">Доступные сети</label>
          <select id="wifiNetworks" onchange="wifiSsid.value=this.value">
            <option value="">Нажмите «Сканировать»</option>
          </select>
        </div>
        <div class="field">
          <label for="wifiSsid">Название сети</label>
          <input id="wifiSsid" autocomplete="off">
        </div>
        <div class="field">
          <label for="wifiPassword">Пароль</label>
          <input id="wifiPassword" type="password" autocomplete="new-password">
        </div>
      </div>
      <div class="buttons">
        <button id="scanWifiButton" onclick="scanWifi()">Сканировать</button>
        <button onclick="saveWifi()">Сохранить Wi-Fi</button>
      </div>
      <div id="wifiMessage" class="muted"></div>
    </section>
    <section class="card wide">
      <h2>MQTT и Home Assistant</h2>
      <div class="settings-fields">
        <div class="field"><label for="mqttServer">Сервер</label><input id="mqttServer" placeholder="192.168.1.100"></div>
        <div class="field"><label for="mqttPort">Порт</label><input id="mqttPort" type="number" min="1" max="65535" step="1" placeholder="1883"></div>
        <div class="field"><label for="mqttUser">Пользователь</label><input id="mqttUser" autocomplete="off"></div>
        <div class="field"><label for="mqttPassword">Пароль (пусто = оставить текущий)</label><input id="mqttPassword" type="password" autocomplete="new-password"></div>
      </div>
      <div class="row"><span>Очистить сохраненный пароль MQTT</span><input id="mqttClearPassword" type="checkbox"></div>
      <div class="row"><span>Состояние</span><span id="mqttDetails" class="value">...</span></div>
      <div class="buttons">
        <button id="saveMqttButton" onclick="saveMqtt()">Сохранить MQTT</button>
        <button id="resetMqttButton" class="warn" onclick="resetMqtt()">MQTT из AppConfig.h</button>
      </div>
      <div id="mqttMessage" class="muted"></div>
    </section>
    <section class="card">
      <h2>Экран OLED</h2>
      <div class="row"><span>Экран</span><span id="displayAvailable" class="value">...</span></div>
      <div class="row"><span>Яркость</span><span id="displayBrightnessValue" class="value">...</span></div>
      <div class="field">
        <label for="displayBrightness">Контраст SSD1306, диапазон 0-255</label>
        <input id="displayBrightness" type="range" min="0" max="255" step="1" oninput="syncDisplayBrightness(this.value)">
      </div>
      <div class="field">
        <label for="displayBrightnessNumber">Точное значение</label>
        <input id="displayBrightnessNumber" type="number" min="0" max="255" step="1" oninput="syncDisplayBrightness(this.value)">
      </div>
      <div class="buttons"><button onclick="saveDisplay()">Сохранить яркость</button></div>
      <div id="displayMessage" class="muted"></div>
    </section>
  </div>
  <p class="muted">IP: <span id="ip">...</span> | WiFi: <span id="wifi">...</span> | MQTT: <span id="mqtt">...</span></p>
  <script>
    const gateText={open:'открыты',closed:'закрыты',opening:'открываются',closing:'закрываются',stopped:'стоп',unknown:'неизвестно'};
    const dirText={up:'вверх',down:'вниз',none:'нет'};
    let editing=false;
    let mqttEditing=false;
    let mqttLoaded=false;
    let displayEditing=false;
    let displayLoaded=false;
    document.addEventListener('input',e=>{
      if(e.target.id.startsWith('upper')||e.target.id.startsWith('lower'))editing=true;
      if(e.target.id.startsWith('mqtt'))mqttEditing=true;
      if(e.target.id.startsWith('displayBrightness'))displayEditing=true
    });
    async function post(path,body=''){await fetch(path,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});await refresh()}
    async function saveSettings(){
      const p=new URLSearchParams({
        upperSensitivity:upperSensitivity.value,lowerSensitivity:lowerSensitivity.value,
        upperAxis:upperAxis.value,lowerAxis:lowerAxis.value,
        upperInvert:upperInvert.checked?'1':'0',lowerInvert:lowerInvert.checked?'1':'0'
      });
      editing=false;await post('/api/settings',p.toString())
    }
    function sensor(prefix,s){
      document.getElementById(prefix+'Connected').textContent=s.connected?'подключен':'не подключен';
      document.getElementById(prefix+'SensorOk').textContent=s.sensorOk?'работает':'ошибка';
      document.getElementById(prefix+'Mac').textContent=s.mac;
      document.getElementById(prefix+'Transport').textContent=s.transport.toUpperCase();
      document.getElementById(prefix+'Age').textContent=s.connected?s.ageMs+' ms':'--';
      document.getElementById(prefix+'Direction').textContent=dirText[s.direction]||s.direction;
      document.getElementById(prefix+'Moving').textContent=s.moving?'да':'нет';
      document.getElementById(prefix+'Value').textContent=s.filtered.toFixed(1)+' uT';
      document.getElementById(prefix+'Velocity').textContent=s.velocity.toFixed(2)+' uT/цикл';
      document.getElementById(prefix+'Delta').textContent=s.delta.toFixed(1)+' uT';
      if(!editing){
        document.getElementById(prefix+'Sensitivity').value=s.sensitivity.toFixed(0);
        document.getElementById(prefix+'Axis').value=s.axis;
        document.getElementById(prefix+'Invert').checked=s.inverted;
      }
    }
    function renderNetworks(networks){
      const selected=wifiSsid.value;
      wifiNetworks.innerHTML='<option value="">Выберите сеть</option>';
      networks.forEach(n=>{
        const o=document.createElement('option');
        o.value=n.ssid;
        o.textContent=n.ssid+' ('+n.rssi+' dBm)'+(n.secure?' [пароль]':'');
        wifiNetworks.appendChild(o)
      });
      if(selected){wifiNetworks.value=selected}
      if(!wifiSsid.value&&networks.length){wifiSsid.value=networks[0].ssid;wifiNetworks.value=networks[0].ssid}
    }
    async function scanWifi(){
      scanWifiButton.disabled=true;
      wifiMessage.textContent='Сканирование сетей...';
      try{
        for(let attempt=0;attempt<20;attempt++){
          const d=await(await fetch('/api/wifi/scan')).json();
          if(!d.scanning){
            renderNetworks(d.networks||[]);
            wifiMessage.textContent=d.networks.length?'Найдено сетей: '+d.networks.length:'Сети не найдены';
            return
          }
          await new Promise(resolve=>setTimeout(resolve,500))
        }
        wifiMessage.textContent='Сканирование занимает слишком много времени'
      }catch(e){
        wifiMessage.textContent='Ошибка сканирования'
      }finally{
        scanWifiButton.disabled=false
      }
    }
    async function saveWifi(){
      if(!wifiSsid.value){wifiMessage.textContent='Укажите название сети';return}
      const p=new URLSearchParams({ssid:wifiSsid.value,password:wifiPassword.value});
      wifiMessage.textContent='Сохранение и подключение...';
      try{
        const r=await fetch('/api/wifi/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p});
        wifiMessage.textContent=r.ok?'Данные сохранены, устройство подключается':'Ошибка сохранения'
      }catch(e){
        wifiMessage.textContent='Соединение прервано. Проверьте новый IP или 192.168.4.1'
      }
    }
    function loadMqtt(m){
      if(mqttEditing||mqttLoaded)return;
      mqttLoaded=true;
      mqttServer.value=m.server||'';
      mqttPort.value=m.port||1883;
      mqttUser.value=m.user||'';
      mqttPassword.value='';
      mqttClearPassword.checked=false
    }
    async function saveMqtt(){
      if(!mqttServer.value){mqttMessage.textContent='Укажите MQTT сервер';return}
      const port=parseInt(mqttPort.value||'0',10);
      if(!port||port<1||port>65535){mqttMessage.textContent='Порт должен быть 1-65535';return}
      const p=new URLSearchParams({
        server:mqttServer.value,port:String(port),
        user:mqttUser.value,password:mqttPassword.value,
        clearPassword:mqttClearPassword.checked?'1':'0'
      });
      mqttMessage.textContent='Сохранение MQTT...';
      try{
        const r=await fetch('/api/mqtt/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p});
        const result=await r.json();
        if(r.ok){
          mqttEditing=false;mqttLoaded=false;mqttPassword.value='';mqttClearPassword.checked=false;
          mqttMessage.textContent='MQTT сохранен, переподключаюсь...';
          setTimeout(refresh,1200)
        }else{
          mqttMessage.textContent='Ошибка: '+(result.error||'неизвестная')
        }
      }catch(e){mqttMessage.textContent='Ошибка сохранения MQTT'}
    }
    async function resetMqtt(){
      if(!confirm('Вернуть MQTT настройки из AppConfig.h?'))return;
      await fetch('/api/mqtt/reset',{method:'POST'});
      mqttEditing=false;mqttLoaded=false;mqttPassword.value='';mqttClearPassword.checked=false;
      mqttMessage.textContent='MQTT настройки сброшены';
      setTimeout(refresh,1200)
    }
    function syncDisplayBrightness(value){
      displayBrightness.value=value;
      displayBrightnessNumber.value=value;
      displayBrightnessValue.textContent=value;
      displayEditing=true
    }
    function loadDisplay(d){
      if(displayEditing||displayLoaded)return;
      displayLoaded=true;
      displayBrightness.min=d.min;displayBrightness.max=d.max;
      displayBrightnessNumber.min=d.min;displayBrightnessNumber.max=d.max;
      displayBrightness.value=d.brightness;
      displayBrightnessNumber.value=d.brightness;
      displayBrightnessValue.textContent=d.brightness
    }
    async function saveDisplay(){
      const value=parseInt(displayBrightnessNumber.value||'0',10);
      const min=parseInt(displayBrightnessNumber.min||'0',10);
      const max=parseInt(displayBrightnessNumber.max||'255',10);
      if(value<min||value>max){displayMessage.textContent='Яркость должна быть '+min+'-'+max;return}
      const p=new URLSearchParams({brightness:String(value)});
      displayMessage.textContent='Сохранение яркости...';
      try{
        const r=await fetch('/api/display/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p});
        const result=await r.json();
        if(r.ok){
          displayEditing=false;displayLoaded=false;
          displayMessage.textContent='Яркость сохранена';
          setTimeout(refresh,600)
        }else{
          displayMessage.textContent='Ошибка: '+(result.error||'неизвестная')
        }
      }catch(e){displayMessage.textContent='Ошибка сохранения яркости'}
    }
    async function refresh(){
      let d;
      try{d=await(await fetch('/api/status')).json()}catch(e){return}
      gateState.textContent=gateText[d.gate.state]||d.gate.state;
      gateMotor.textContent=gateText[d.gate.motor]||d.gate.motor;
      wicketState.textContent=d.wicket.open?'открыта':'закрыта';
      wicketRelay.textContent=d.wicket.relay?'ON':'OFF';
      lightState.textContent=d.light.on?'включено':'выключено';
      temperature.textContent=d.climate.valid?d.climate.temperature.toFixed(1)+' C':'--';
      humidity.textContent=d.climate.valid?d.climate.humidity.toFixed(1)+' %':'--';
      ip.textContent=d.network.ip;wifi.textContent=d.network.connected?'online':'setup AP';mqtt.textContent=d.mqtt.connected?'online':'offline';
      mqttDetails.textContent=d.mqtt.server+':'+d.mqtt.port+' / '+(d.mqtt.connected?'подключен':'нет связи')+(d.mqtt.passwordSet?' / пароль задан':' / без пароля');
      networkBadge.textContent=d.network.connected?'Wi-Fi подключен':'Точка настройки';
      networkBadge.className='pill '+(d.network.connected?'on':'off');
      networkMode.textContent=d.network.connected?'Домашняя сеть':'Точка доступа';
      currentSsid.textContent=d.network.connected?d.network.ssid:d.network.setupSsid;
      networkIp.textContent=d.network.ip;
      displayAvailable.textContent=d.display.available?'найден':'нет связи';
      sensor('upper',d.upperSensor);sensor('lower',d.lowerSensor);
      loadMqtt(d.mqtt);loadDisplay(d.display)
    }
    refresh();setInterval(refresh,750);
  </script>
</body>
</html>
)HTML";

}

WebInterface::WebInterface(NetworkManager &network,
                           GateController &gate,
                           MagnetSensorController &magnets,
                           ClimateController &climate,
                           HomeAssistantBridge &homeAssistant,
                           MqttSettings &mqttSettings,
                           DisplayController &display,
                           DisplaySettings &displaySettings)
    : _server(80),
      _network(network),
      _gate(gate),
      _magnets(magnets),
      _climate(climate),
      _homeAssistant(homeAssistant),
      _mqttSettings(mqttSettings),
      _display(display),
      _displaySettings(displaySettings)
{
}

void WebInterface::begin()
{
  setupRoutes();
  _server.begin();
}

void WebInterface::loop()
{
  _server.handleClient();
}

void WebInterface::setupRoutes()
{
  _server.on("/", HTTP_GET, [this]()
             { handleRoot(); });
  _server.on("/wifi", HTTP_GET, [this]()
             { handleRoot(); });
  _server.on("/api/status", HTTP_GET, [this]()
             { handleStatus(); });
  _server.on("/api/settings", HTTP_POST, [this]()
             { handleSettings(); });
  _server.on("/api/wifi/scan", HTTP_GET, [this]()
             { handleWifiScan(); });
  _server.on("/api/wifi/save", HTTP_POST, [this]()
             { handleWifiSave(); });
  _server.on("/api/mqtt/save", HTTP_POST, [this]()
             { handleMqttSave(); });
  _server.on("/api/mqtt/reset", HTTP_POST, [this]()
             { handleMqttReset(); });
  _server.on("/api/display/save", HTTP_POST, [this]()
             { handleDisplaySave(); });

  _server.on("/api/gate/open", HTTP_POST, [this]()
             {
               _gate.commandOpen();
               sendOk();
             });
  _server.on("/api/gate/close", HTTP_POST, [this]()
             {
               _gate.commandClose();
               sendOk();
             });
  _server.on("/api/gate/stop", HTTP_POST, [this]()
             {
               _gate.commandStop();
               sendOk();
             });
  _server.on("/api/wicket/open", HTTP_POST, [this]()
             {
               _gate.triggerWicket();
               sendOk();
             });
  _server.on("/api/light/toggle", HTTP_POST, [this]()
             {
               _gate.toggleLight();
               sendOk();
             });
  _server.on("/api/sensor/upper/baseline", HTTP_POST, [this]()
             {
               _magnets.rememberUpperBaseline();
               sendOk();
             });
  _server.on("/api/sensor/lower/baseline", HTTP_POST, [this]()
             {
               _magnets.rememberLowerBaseline();
               sendOk();
             });
}

void WebInterface::handleRoot()
{
  _server.send_P(200, "text/html; charset=utf-8", CONTROL_PAGE);
}

void WebInterface::handleStatus()
{
  String json;
  json.reserve(1300);
  json += "{\"network\":{\"connected\":";
  json += _network.connected() ? "true" : "false";
  json += ",\"provisioning\":";
  json += _network.provisioningMode() ? "true" : "false";
  json += ",\"ip\":\"";
  json += _network.address().toString();
  json += "\",\"ssid\":\"";
  json += jsonEscape(_network.connectedSsid());
  json += "\",\"setupSsid\":\"";
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
  json += "},\"display\":{\"available\":";
  json += _display.available() ? "true" : "false";
  json += ",\"brightness\":";
  json += String(_display.brightness());
  json += ",\"min\":";
  json += String(_displaySettings.minBrightness());
  json += ",\"max\":";
  json += String(_displaySettings.maxBrightness());
  json += "},\"gate\":{\"state\":\"";
  json += gateStateToText(_gate.state());
  json += "\",\"motor\":\"";
  json += gateMotorStateToText(_gate.motorState());
  json += "\"},\"wicket\":{\"open\":";
  json += _gate.wicketOpen() ? "true" : "false";
  json += ",\"relay\":";
  json += _gate.wicketRelayActive() ? "true" : "false";
  json += "},\"light\":{\"on\":";
  json += _gate.lightOn() ? "true" : "false";
  json += "},\"climate\":{\"valid\":";
  json += _climate.valid() ? "true" : "false";
  json += ",\"temperature\":";
  json += _climate.valid() ? String(_climate.temperature(), 2) : "0";
  json += ",\"humidity\":";
  json += _climate.valid() ? String(_climate.humidity(), 2) : "0";
  json += "},";
  appendSensorJson(json, "upperSensor", _magnets.upperState(), _magnets.upperLink());
  json += ",";
  appendSensorJson(json, "lowerSensor", _magnets.lowerState(), _magnets.lowerLink());
  json += "}";

  _server.send(200, "application/json", json);
}

void WebInterface::handleSettings()
{
  if (_server.hasArg("upperSensitivity"))
  {
    _magnets.setUpperSensitivity(_server.arg("upperSensitivity").toFloat());
  }
  if (_server.hasArg("lowerSensitivity"))
  {
    _magnets.setLowerSensitivity(_server.arg("lowerSensitivity").toFloat());
  }
  if (_server.hasArg("upperAxis"))
  {
    _magnets.setUpperAxis(MagnetSensorController::axisFromText(_server.arg("upperAxis")));
  }
  if (_server.hasArg("lowerAxis"))
  {
    _magnets.setLowerAxis(MagnetSensorController::axisFromText(_server.arg("lowerAxis")));
  }

  _magnets.setUpperInverted(_server.arg("upperInvert") == "1");
  _magnets.setLowerInverted(_server.arg("lowerInvert") == "1");
  _magnets.saveSettings();
  sendOk();
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

  String json = "{\"networks\":[";

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

  json += "],\"scanning\":false}";
  WiFi.scanDelete();
  _server.send(200, "application/json", json);
}

void WebInterface::handleWifiSave()
{
  if (!_server.hasArg("ssid") || _server.arg("ssid").length() == 0)
  {
    _server.send(400, "application/json", "{\"ok\":false,\"error\":\"ssid_required\"}");
    return;
  }

  _network.saveCredentials(_server.arg("ssid"), _server.arg("password"));
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
  sendOk();
}

void WebInterface::handleDisplaySave()
{
  if (!_server.hasArg("brightness"))
  {
    _server.send(400, "application/json", "{\"ok\":false,\"error\":\"brightness_required\"}");
    return;
  }

  const int brightness = _server.arg("brightness").toInt();
  if (brightness < _displaySettings.minBrightness() ||
      brightness > _displaySettings.maxBrightness())
  {
    _server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid_brightness\"}");
    return;
  }

  if (!_displaySettings.saveBrightness(static_cast<uint16_t>(brightness)))
  {
    _server.send(500, "application/json", "{\"ok\":false,\"error\":\"save_failed\"}");
    return;
  }

  _display.setBrightness(static_cast<uint8_t>(_displaySettings.brightness()));
  sendOk();
}

void WebInterface::sendOk()
{
  _server.send(200, "application/json", "{\"ok\":true}");
}

void WebInterface::appendSensorJson(String &json,
                                    const char *name,
                                    const MagneticPresenceSensorState &state,
                                    const RemoteSensorLinkStatus &link)
{
  json += "\"";
  json += name;
  json += "\":{\"connected\":";
  json += link.connected ? "true" : "false";
  json += ",\"sensorOk\":";
  json += link.sensorOk ? "true" : "false";
  json += ",\"mac\":\"";
  json += formatMac(link.mac);
  json += "\",\"transport\":\"";
  json += MagnetSensorController::transportToText(link.transport);
  json += "\",\"ageMs\":";
  json += String(link.ageMs);
  json += ",\"packets\":";
  json += String(link.packetsReceived);
  json += ",\"valid\":";
  json += state.valid ? "true" : "false";
  json += ",\"calibrated\":";
  json += state.calibrated ? "true" : "false";
  json += ",\"active\":";
  json += state.active ? "true" : "false";
  json += ",\"moving\":";
  json += state.moving ? "true" : "false";
  json += ",\"motionQualified\":";
  json += state.motionQualified ? "true" : "false";
  json += ",\"direction\":\"";
  json += directionText(state.direction);
  json += "\",\"settledDirection\":\"";
  json += directionText(state.settledDirection);
  json += "\",\"axis\":\"";
  json += MagnetSensorController::axisToText(state.axis);
  json += "\",\"inverted\":";
  json += state.invertDirection ? "true" : "false";
  json += ",\"filtered\":";
  json += String(state.filtered, 2);
  json += ",\"velocity\":";
  json += String(state.velocity, 3);
  json += ",\"baseline\":";
  json += String(state.baseline, 2);
  json += ",\"delta\":";
  json += String(state.delta, 2);
  json += ",\"sensitivity\":";
  json += String(state.sensitivity, 2);
  json += "}";
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
    }
    if (c == '\n' || c == '\r')
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

String WebInterface::formatMac(const uint8_t mac[6])
{
  char buffer[18];
  snprintf(buffer,
           sizeof(buffer),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0],
           mac[1],
           mac[2],
           mac[3],
           mac[4],
           mac[5]);
  return String(buffer);
}

const char *WebInterface::directionText(int8_t direction)
{
  if (direction > 0)
  {
    return "up";
  }
  if (direction < 0)
  {
    return "down";
  }
  return "none";
}
