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
  <title>PC Lamp</title>
  <style>
    :root{color-scheme:dark;font-family:Arial,sans-serif;background:#101418;color:#eef3f7}
    body{margin:0 auto;padding:16px;max-width:1040px}
    header{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:14px}
    h1{font-size:24px;margin:0}h2{font-size:17px;margin:0 0 10px}
    .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(270px,1fr));gap:12px}
    .card{border:1px solid #2b3945;border-radius:8px;padding:14px;background:#17202a}
    .wide{grid-column:1/-1}
    .row{display:flex;align-items:center;justify-content:space-between;gap:12px;margin:8px 0}
    .value{font-weight:700;text-align:right}.muted{opacity:.72;font-size:13px}
    .reading{font-size:32px;font-weight:700;margin:8px 0}.unit{font-size:18px;opacity:.72}
    .pill{border-radius:999px;padding:4px 9px;background:#2d3946}.on{background:#0d8f54}.off{background:#65313a}.warnpill{background:#a75a00}
    .fields{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}
    .field label{display:block;margin-bottom:5px;font-size:13px;opacity:.75}
    input,select{box-sizing:border-box;width:100%;padding:9px;border-radius:6px;border:1px solid #3f5368;background:#0f141a;color:#eef3f7}
    input[type=checkbox]{width:auto}input[type=color]{height:40px;padding:3px}
    .buttons{display:flex;flex-wrap:wrap;gap:8px;margin-top:12px}
    button{border:0;border-radius:6px;padding:10px 12px;background:#2b7cff;color:#fff;font-weight:700;cursor:pointer}
    button.secondary{background:#747b84}button.warn{background:#a75a00}button.good{background:#0b8f62}button:disabled{opacity:.5}
    #wifiMessage,#mqttMessage,#lightMessage,#pcSettingsMessage{min-height:18px;margin-top:10px}
    @media(max-width:700px){.fields{grid-template-columns:1fr}.row{align-items:flex-start}}
  </style>
</head>
<body>
  <header><h1>PC Lamp</h1><span id="networkBadge" class="pill">...</span></header>
  <div class="grid">
    <section class="card">
      <h2>ПК</h2>
      <div class="row"><span>Состояние</span><span id="pcState" class="value">...</span></div>
      <div class="row"><span>Команда</span><span id="pcAction" class="value">...</span></div>
      <div class="row"><span>Попытки</span><span id="pcAttempts" class="value">...</span></div>
      <div class="row"><span>Кнопка</span><span id="pcButton" class="value">...</span></div>
      <div class="row"><span>Сырой вход</span><span id="pcRaw" class="value">...</span></div>
      <div class="buttons">
        <button class="good" id="pcOnButton">Включить</button>
        <button class="warn" id="pcOffButton">Выключить</button>
        <button class="secondary" id="pcPulseButton">Импульс</button>
        <button class="secondary" id="pcCancelButton">Отмена</button>
      </div>
    </section>
    <section class="card">
      <h2>Свет</h2>
      <div class="row"><span>Включен</span><input id="lightEnabled" type="checkbox"></div>
      <div class="row"><span>Яркость</span><span id="brightnessValue" class="value">...</span></div>
      <div class="field"><label for="brightness">PWM 0-255</label><input id="brightness" type="range" min="0" max="255" step="1"></div>
      <div class="field"><label for="lightMode">Режим</label><select id="lightMode"><option value="white">Белый CCT</option><option value="rgb">RGB</option></select></div>
      <div class="row"><span>Температура</span><span id="colorTempValue" class="value">...</span></div>
      <div class="field"><label for="colorTemp">Mired</label><input id="colorTemp" type="range" min="153" max="454" step="1"></div>
      <div class="fields">
        <div class="field"><label for="rgbColor">Цвет</label><input id="rgbColor" type="color"></div>
        <div class="field"><label for="red">R</label><input id="red" type="number" min="0" max="255" step="1"></div>
        <div class="field"><label for="green">G</label><input id="green" type="number" min="0" max="255" step="1"></div>
        <div class="field"><label for="blue">B</label><input id="blue" type="number" min="0" max="255" step="1"></div>
      </div>
      <div class="buttons"><button id="saveLightButton">Сохранить свет</button></div>
      <div id="lightMessage" class="muted"></div>
    </section>
    <section class="card">
      <h2>Температура</h2>
      <div class="reading"><span id="temperature">--</span> <span class="unit">°C</span></div>
      <div class="row"><span>Влажность</span><span id="humidity" class="value">...</span></div>
      <div class="row"><span>DHT22</span><span id="sensorState" class="value">...</span></div>
      <div class="row"><span>Последний успех</span><span id="sensorAge" class="value">...</span></div>
      <div class="row"><span>Ошибка</span><span id="sensorError" class="value">...</span></div>
    </section>
    <section class="card">
      <h2>Связь</h2>
      <div class="row"><span>Wi-Fi</span><span id="wifiState" class="value">...</span></div>
      <div class="row"><span>SSID</span><span id="currentSsid" class="value">...</span></div>
      <div class="row"><span>IP</span><span id="networkIp" class="value">...</span></div>
      <div class="row"><span>Сигнал</span><span id="rssi" class="value">...</span></div>
      <div class="row"><span>MQTT</span><span id="mqttState" class="value">...</span></div>
    </section>
    <section class="card">
      <h2>Система</h2>
      <div class="row"><span>Версия</span><span id="version" class="value">...</span></div>
      <div class="row"><span>Время работы</span><span id="uptime" class="value">...</span></div>
      <div class="row"><span>Память</span><span id="heap" class="value">...</span></div>
      <div class="row"><span>Точка настройки</span><span id="setupSsid" class="value">...</span></div>
    </section>
    <section class="card wide">
      <h2>Настройка сети</h2>
      <div class="fields">
        <div class="field"><label for="networks">Доступные сети</label><select id="networks"><option value="">Нажмите «Сканировать»</option></select></div>
        <div class="field"><label for="ssid">Название сети</label><input id="ssid" autocomplete="off"></div>
        <div class="field"><label for="password">Пароль</label><input id="password" type="password" autocomplete="new-password"></div>
      </div>
      <div class="row"><span>DHCP</span><input id="dhcp" type="checkbox" checked></div>
      <div id="staticFields" class="fields">
        <div class="field"><label for="staticIp">Статический IP</label><input id="staticIp" placeholder="192.168.1.50"></div>
        <div class="field"><label for="gateway">Шлюз</label><input id="gateway" placeholder="192.168.1.1"></div>
        <div class="field"><label for="subnet">Маска</label><input id="subnet" placeholder="255.255.255.0"></div>
        <div class="field"><label for="dns1">DNS 1</label><input id="dns1" placeholder="192.168.1.1"></div>
        <div class="field"><label for="dns2">DNS 2</label><input id="dns2" placeholder="1.1.1.1"></div>
      </div>
      <div class="buttons">
        <button id="scanButton">Сканировать</button>
        <button id="saveWifiButton">Сохранить Wi-Fi</button>
        <button id="resetWifiButton" class="warn">Сбросить сеть</button>
      </div>
      <div id="wifiMessage" class="muted"></div>
    </section>
    <section class="card wide">
      <h2>MQTT и Home Assistant</h2>
      <div class="fields">
        <div class="field"><label for="mqttServer">Сервер</label><input id="mqttServer" placeholder="192.168.1.100"></div>
        <div class="field"><label for="mqttPort">Порт</label><input id="mqttPort" type="number" min="1" max="65535" step="1" placeholder="1883"></div>
        <div class="field"><label for="mqttUser">Пользователь</label><input id="mqttUser" autocomplete="off"></div>
        <div class="field"><label for="mqttPassword">Пароль</label><input id="mqttPassword" type="password" autocomplete="new-password"></div>
      </div>
      <div class="row"><span>Очистить пароль MQTT</span><input id="mqttClearPassword" type="checkbox"></div>
      <div class="row"><span>Состояние</span><span id="mqttDetails" class="value">...</span></div>
      <div class="buttons">
        <button id="saveMqttButton">Сохранить MQTT</button>
        <button id="resetMqttButton" class="warn">MQTT из AppConfig.h</button>
      </div>
      <div id="mqttMessage" class="muted"></div>
    </section>
    <section class="card wide">
      <h2>Логика питания ПК</h2>
      <div class="fields">
        <div class="field"><label for="statusActiveHigh">PC ON = HIGH</label><select id="statusActiveHigh"><option value="1">HIGH</option><option value="0">LOW</option></select></div>
        <div class="field"><label for="buttonActiveHigh">Кнопка активна HIGH</label><select id="buttonActiveHigh"><option value="1">HIGH</option><option value="0">LOW</option></select></div>
        <div class="field"><label for="statusDebounceMs">Debounce статуса, ms</label><input id="statusDebounceMs" type="number" min="20" max="10000" step="10"></div>
        <div class="field"><label for="buttonPulseMs">Импульс кнопки, ms</label><input id="buttonPulseMs" type="number" min="50" max="5000" step="10"></div>
        <div class="field"><label for="startRetryMs">Retry включения, ms</label><input id="startRetryMs" type="number" min="1000" max="120000" step="500"></div>
        <div class="field"><label for="shutdownRetryMs">Retry выключения, ms</label><input id="shutdownRetryMs" type="number" min="1000" max="120000" step="500"></div>
        <div class="field"><label for="maxStartAttempts">Попытки включения</label><input id="maxStartAttempts" type="number" min="1" max="10" step="1"></div>
        <div class="field"><label for="maxShutdownAttempts">Попытки выключения</label><input id="maxShutdownAttempts" type="number" min="1" max="10" step="1"></div>
      </div>
      <div class="buttons">
        <button id="savePcSettingsButton">Сохранить логику ПК</button>
        <button id="resetPcSettingsButton" class="warn">Заводские настройки ПК</button>
      </div>
      <div id="pcSettingsMessage" class="muted"></div>
    </section>
  </div>
  <script>
    const el=id=>document.getElementById(id);
    const actionText={idle:'ожидание',turning_on:'включение',turning_off:'выключение',failed:'ошибка'};
    let wifiEditing=false,wifiLoaded=false,mqttEditing=false,mqttLoaded=false,lightEditing=false,lightLoaded=false,pcSettingsEditing=false,pcSettingsLoaded=false;
    ['ssid','password','dhcp','staticIp','gateway','subnet','dns1','dns2'].forEach(id=>el(id).addEventListener('input',()=>wifiEditing=true));
    ['mqttServer','mqttPort','mqttUser','mqttPassword','mqttClearPassword'].forEach(id=>el(id).addEventListener('input',()=>mqttEditing=true));
    ['lightEnabled','brightness','lightMode','colorTemp','rgbColor','red','green','blue'].forEach(id=>el(id).addEventListener('input',()=>lightEditing=true));
    ['statusActiveHigh','buttonActiveHigh','statusDebounceMs','buttonPulseMs','startRetryMs','shutdownRetryMs','maxStartAttempts','maxShutdownAttempts'].forEach(id=>el(id).addEventListener('input',()=>pcSettingsEditing=true));
    el('networks').addEventListener('change',()=>{if(el('networks').value){el('ssid').value=el('networks').value;wifiEditing=true}});
    el('dhcp').addEventListener('change',toggleStatic);
    el('scanButton').addEventListener('click',scanWifi);
    el('saveWifiButton').addEventListener('click',saveWifi);
    el('resetWifiButton').addEventListener('click',resetWifi);
    el('saveMqttButton').addEventListener('click',saveMqtt);
    el('resetMqttButton').addEventListener('click',resetMqtt);
    el('saveLightButton').addEventListener('click',saveLight);
    el('savePcSettingsButton').addEventListener('click',savePcSettings);
    el('resetPcSettingsButton').addEventListener('click',resetPcSettings);
    el('pcOnButton').addEventListener('click',()=>post('/api/pc/on'));
    el('pcOffButton').addEventListener('click',()=>post('/api/pc/off'));
    el('pcPulseButton').addEventListener('click',()=>post('/api/pc/pulse'));
    el('pcCancelButton').addEventListener('click',()=>post('/api/pc/cancel'));
    el('brightness').addEventListener('input',()=>el('brightnessValue').textContent=el('brightness').value);
    el('colorTemp').addEventListener('input',()=>el('colorTempValue').textContent=el('colorTemp').value);
    el('rgbColor').addEventListener('input',()=>{const c=hexToRgb(el('rgbColor').value);red.value=c.r;green.value=c.g;blue.value=c.b});
    ['red','green','blue'].forEach(id=>el(id).addEventListener('input',()=>{rgbColor.value=rgbToHex(red.value,green.value,blue.value)}));
    function toggleStatic(){const disabled=el('dhcp').checked;['staticIp','gateway','subnet','dns1','dns2'].forEach(id=>el(id).disabled=disabled)}
    function clamp(v,min,max){v=parseInt(v||'0',10);return Math.max(min,Math.min(max,v))}
    function rgbToHex(r,g,b){return '#'+[r,g,b].map(v=>clamp(v,0,255).toString(16).padStart(2,'0')).join('')}
    function hexToRgb(hex){const value=parseInt(hex.slice(1),16);return{r:(value>>16)&255,g:(value>>8)&255,b:value&255}}
    function formatDuration(seconds){const days=Math.floor(seconds/86400);seconds%=86400;const hours=Math.floor(seconds/3600);seconds%=3600;const minutes=Math.floor(seconds/60);return(days?days+' д ':'')+hours+' ч '+minutes+' мин'}
    function formatAge(ms){return ms<0?'--':Math.round(ms/1000)+' с'}
    async function post(path,body=''){await fetch(path,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});await refresh()}
    function loadWifi(n){if(wifiEditing||wifiLoaded)return;wifiLoaded=true;ssid.value=n.configuredSsid||'';dhcp.checked=!n.static;staticIp.value=n.config.ip==='0.0.0.0'?'':n.config.ip;gateway.value=n.config.gateway==='0.0.0.0'?'':n.config.gateway;subnet.value=n.config.subnet==='0.0.0.0'?'':n.config.subnet;dns1.value=n.config.dns1==='0.0.0.0'?'':n.config.dns1;dns2.value=n.config.dns2==='0.0.0.0'?'':n.config.dns2;toggleStatic()}
    function loadMqtt(m){if(mqttEditing||mqttLoaded)return;mqttLoaded=true;mqttServer.value=m.server||'';mqttPort.value=m.port||1883;mqttUser.value=m.user||'';mqttPassword.value='';mqttClearPassword.checked=false}
    function loadLight(l){if(lightEditing||lightLoaded)return;lightLoaded=true;lightEnabled.checked=l.on;brightness.value=l.brightness;brightnessValue.textContent=l.brightness;lightMode.value=l.mode;colorTemp.min=l.minColorTemp;colorTemp.max=l.maxColorTemp;colorTemp.value=l.colorTemp;colorTempValue.textContent=l.colorTemp;red.value=l.red;green.value=l.green;blue.value=l.blue;rgbColor.value=rgbToHex(l.red,l.green,l.blue)}
    function loadPcSettings(p){if(pcSettingsEditing||pcSettingsLoaded)return;pcSettingsLoaded=true;statusActiveHigh.value=p.statusActiveHigh?'1':'0';buttonActiveHigh.value=p.buttonActiveHigh?'1':'0';statusDebounceMs.value=p.statusDebounceMs;buttonPulseMs.value=p.buttonPulseMs;startRetryMs.value=p.startRetryMs;shutdownRetryMs.value=p.shutdownRetryMs;maxStartAttempts.value=p.maxStartAttempts;maxShutdownAttempts.value=p.maxShutdownAttempts}
    async function refresh(){
      try{
        const d=await(await fetch('/api/status')).json();
        pcState.textContent=d.pc.on?'включен':'выключен';
        pcAction.textContent=actionText[d.pc.action]||d.pc.action;
        pcAttempts.textContent=d.pc.attempts+' / '+(d.pc.desiredOn?d.pcSettings.maxStartAttempts:d.pcSettings.maxShutdownAttempts);
        pcButton.textContent=d.pc.buttonActive?'нажата':'отпущена';
        pcRaw.textContent=(d.pc.rawOn?'ON':'OFF')+(d.pc.statusChanging?' / debounce':'');
        temperature.textContent=d.climate.valid?d.climate.temperature.toFixed(1):'--';
        humidity.textContent=d.climate.valid?d.climate.humidity.toFixed(1)+' %':'--';
        sensorState.textContent=d.climate.valid?'работает':'нет данных';
        sensorAge.textContent=formatAge(d.climate.lastSuccessAgeMs);
        sensorError.textContent=d.climate.error;
        wifiState.textContent=d.network.connected?'подключен':'точка настройки';
        currentSsid.textContent=d.network.connected?d.network.ssid:d.network.setupSsid;
        networkIp.textContent=d.network.ip;
        rssi.textContent=d.network.connected?d.network.rssi+' dBm':'--';
        mqttState.textContent=d.mqtt.connected?'подключен':'нет связи';
        mqttDetails.textContent=d.mqtt.server+':'+d.mqtt.port+' / '+(d.mqtt.connected?'подключен':'нет связи')+(d.mqtt.passwordSet?' / пароль задан':' / без пароля');
        version.textContent=d.system.version;
        uptime.textContent=formatDuration(d.system.uptimeSec);
        heap.textContent=Math.round(d.system.freeHeap/1024)+' KB';
        setupSsid.textContent=d.network.setupSsid;
        networkBadge.textContent=d.pc.action==='failed'?'Ошибка ПК':(d.network.connected?'Wi-Fi подключен':'Точка настройки');
        networkBadge.className='pill '+(d.pc.action==='failed'?'warnpill':(d.network.connected?'on':'off'));
        loadWifi(d.network);loadMqtt(d.mqtt);loadLight(d.light);loadPcSettings(d.pcSettings)
      }catch(e){}
    }
    async function scanWifi(){
      scanButton.disabled=true;wifiMessage.textContent='Сканирование сетей...';
      try{
        for(let attempt=0;attempt<20;attempt++){
          const d=await(await fetch('/api/wifi/scan')).json();
          if(!d.scanning){
            networks.innerHTML='<option value="">Выберите сеть</option>';
            (d.networks||[]).forEach(n=>{const option=document.createElement('option');option.value=n.ssid;option.textContent=n.ssid+' ('+n.rssi+' dBm)'+(n.secure?' [пароль]':'');networks.appendChild(option)});
            wifiMessage.textContent='Найдено сетей: '+d.networks.length;return
          }
          await new Promise(resolve=>setTimeout(resolve,500))
        }
        wifiMessage.textContent='Сканирование занимает слишком много времени'
      }catch(e){wifiMessage.textContent='Ошибка сканирования'}finally{scanButton.disabled=false}
    }
    async function saveWifi(){
      if(!ssid.value){wifiMessage.textContent='Укажите название сети';return}
      if(!dhcp.checked&&(!staticIp.value||!gateway.value||!subnet.value)){wifiMessage.textContent='Заполните IP, шлюз и маску';return}
      const body=new URLSearchParams({ssid:ssid.value,password:password.value,mode:dhcp.checked?'dhcp':'static',ip:staticIp.value,gateway:gateway.value,subnet:subnet.value,dns1:dns1.value,dns2:dns2.value});
      wifiMessage.textContent='Сохранение Wi-Fi...';
      try{const r=await fetch('/api/wifi/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});const result=await r.json();wifiMessage.textContent=r.ok?'Wi-Fi сохранен':'Ошибка: '+(result.error||'неизвестная')}
      catch(e){wifiMessage.textContent='Соединение прервано. Проверьте новый IP или 192.168.4.1'}
    }
    async function resetWifi(){if(!confirm('Вернуть сетевые настройки из AppConfig.h?'))return;await fetch('/api/wifi/reset',{method:'POST'});wifiEditing=false;wifiLoaded=false;password.value='';wifiMessage.textContent='Сетевые настройки сброшены';setTimeout(refresh,1200)}
    async function saveMqtt(){
      if(!mqttServer.value){mqttMessage.textContent='Укажите MQTT сервер';return}
      const port=parseInt(mqttPort.value||'0',10);if(!port||port<1||port>65535){mqttMessage.textContent='Порт должен быть 1-65535';return}
      const body=new URLSearchParams({server:mqttServer.value,port:String(port),user:mqttUser.value,password:mqttPassword.value,clearPassword:mqttClearPassword.checked?'1':'0'});
      mqttMessage.textContent='Сохранение MQTT...';
      try{const r=await fetch('/api/mqtt/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});const result=await r.json();if(r.ok){mqttEditing=false;mqttLoaded=false;mqttPassword.value='';mqttClearPassword.checked=false;mqttMessage.textContent='MQTT сохранен';setTimeout(refresh,1200)}else{mqttMessage.textContent='Ошибка: '+(result.error||'неизвестная')}}
      catch(e){mqttMessage.textContent='Ошибка сохранения MQTT'}
    }
    async function resetMqtt(){if(!confirm('Вернуть MQTT настройки из AppConfig.h?'))return;await fetch('/api/mqtt/reset',{method:'POST'});mqttEditing=false;mqttLoaded=false;mqttPassword.value='';mqttClearPassword.checked=false;mqttMessage.textContent='MQTT настройки сброшены';setTimeout(refresh,1200)}
    async function saveLight(){
      const body=new URLSearchParams({on:lightEnabled.checked?'1':'0',brightness:brightness.value,mode:lightMode.value,colorTemp:colorTemp.value,red:red.value,green:green.value,blue:blue.value});
      lightMessage.textContent='Сохранение света...';
      try{const r=await fetch('/api/light/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});lightMessage.textContent=r.ok?'Свет сохранен':'Ошибка сохранения';lightEditing=false;lightLoaded=false;setTimeout(refresh,500)}
      catch(e){lightMessage.textContent='Ошибка сохранения света'}
    }
    async function savePcSettings(){
      const body=new URLSearchParams({statusActiveHigh:statusActiveHigh.value,buttonActiveHigh:buttonActiveHigh.value,statusDebounceMs:statusDebounceMs.value,buttonPulseMs:buttonPulseMs.value,startRetryMs:startRetryMs.value,shutdownRetryMs:shutdownRetryMs.value,maxStartAttempts:maxStartAttempts.value,maxShutdownAttempts:maxShutdownAttempts.value});
      pcSettingsMessage.textContent='Сохранение логики ПК...';
      try{const r=await fetch('/api/pc/settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});pcSettingsMessage.textContent=r.ok?'Логика ПК сохранена':'Ошибка сохранения';pcSettingsEditing=false;pcSettingsLoaded=false;setTimeout(refresh,500)}
      catch(e){pcSettingsMessage.textContent='Ошибка сохранения логики ПК'}
    }
    async function resetPcSettings(){if(!confirm('Вернуть заводские настройки логики ПК?'))return;await fetch('/api/pc/settings/reset',{method:'POST'});pcSettingsEditing=false;pcSettingsLoaded=false;pcSettingsMessage.textContent='Настройки ПК сброшены';setTimeout(refresh,500)}
    toggleStatic();refresh();setInterval(refresh,1000);
  </script>
</body>
</html>
)HTML";
}

WebInterface::WebInterface(NetworkManager &network,
                           ClimateSensor &climate,
                           RGBWWWController &lightController,
                           PcPowerController &pcPower,
                           PcPowerSettings &pcSettings,
                           HomeAssistantBridge &homeAssistant,
                           MqttSettings &mqttSettings)
    : _server(80),
      _network(network),
      _climate(climate),
      _lightController(lightController),
      _pcPower(pcPower),
      _pcSettings(pcSettings),
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
  _server.on("/", HTTP_GET, [this]()
             { handleRoot(); });
  _server.on("/wifi", HTTP_GET, [this]()
             { handleRoot(); });
  _server.on("/api/status", HTTP_GET, [this]()
             { handleStatus(); });
  _server.on("/api/wifi/scan", HTTP_GET, [this]()
             { handleWifiScan(); });
  _server.on("/api/wifi/save", HTTP_POST, [this]()
             { handleWifiSave(); });
  _server.on("/api/wifi/reset", HTTP_POST, [this]()
             { handleWifiReset(); });
  _server.on("/api/mqtt/save", HTTP_POST, [this]()
             { handleMqttSave(); });
  _server.on("/api/mqtt/reset", HTTP_POST, [this]()
             { handleMqttReset(); });
  _server.on("/api/light/save", HTTP_POST, [this]()
             { handleLightSave(); });
  _server.on("/api/pc/settings", HTTP_POST, [this]()
             { handlePcSettingsSave(); });
  _server.on("/api/pc/settings/reset", HTTP_POST, [this]()
             { handlePcSettingsReset(); });
  _server.on("/api/pc/on", HTTP_POST, [this]()
             {
               _pcPower.commandTurnOn();
               sendOk();
             });
  _server.on("/api/pc/off", HTTP_POST, [this]()
             {
               _pcPower.commandTurnOff();
               sendOk();
             });
  _server.on("/api/pc/toggle", HTTP_POST, [this]()
             {
               _pcPower.toggle();
               sendOk();
             });
  _server.on("/api/pc/pulse", HTTP_POST, [this]()
             {
               _pcPower.pulseButton();
               sendOk();
             });
  _server.on("/api/pc/cancel", HTTP_POST, [this]()
             {
               _pcPower.cancelCommand();
               sendOk();
             });
  _server.onNotFound([this]()
                     { _server.sendHeader("Location", "/", true);
                       _server.send(302, "text/plain", ""); });
}

void WebInterface::handleRoot()
{
  _server.send_P(200, "text/html; charset=utf-8", CONTROL_PAGE);
}

void WebInterface::handleStatus()
{
  const PcPowerRuntimeSettings &pc = _pcSettings.values();
  const int32_t lastSuccessAge = _climate.lastSuccessAgeMs() == UINT32_MAX
                                     ? -1
                                     : static_cast<int32_t>(_climate.lastSuccessAgeMs());

  String json;
  json.reserve(2800);
  json += "{\"network\":{\"connected\":";
  json += _network.connected() ? "true" : "false";
  json += ",\"provisioning\":";
  json += _network.provisioningMode() ? "true" : "false";
  json += ",\"static\":";
  json += _network.staticIpEnabled() ? "true" : "false";
  json += ",\"ip\":\"";
  json += _network.address().toString();
  json += "\",\"gateway\":\"";
  json += _network.gateway().toString();
  json += "\",\"subnet\":\"";
  json += _network.subnet().toString();
  json += "\",\"dns1\":\"";
  json += _network.dns1().toString();
  json += "\",\"dns2\":\"";
  json += _network.dns2().toString();
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
  json += "\"}},\"mqtt\":{\"connected\":";
  json += _homeAssistant.connected() ? "true" : "false";
  json += ",\"server\":\"";
  json += jsonEscape(_mqttSettings.server());
  json += "\",\"port\":";
  json += String(_mqttSettings.port());
  json += ",\"user\":\"";
  json += jsonEscape(_mqttSettings.user());
  json += "\",\"passwordSet\":";
  json += _mqttSettings.passwordSet() ? "true" : "false";
  json += "},\"pc\":{\"on\":";
  json += _pcPower.isOn() ? "true" : "false";
  json += ",\"rawOn\":";
  json += _pcPower.rawOn() ? "true" : "false";
  json += ",\"desiredOn\":";
  json += _pcPower.desiredOn() ? "true" : "false";
  json += ",\"commandActive\":";
  json += _pcPower.commandActive() ? "true" : "false";
  json += ",\"buttonActive\":";
  json += _pcPower.buttonActive() ? "true" : "false";
  json += ",\"statusChanging\":";
  json += _pcPower.statusChanging() ? "true" : "false";
  json += ",\"action\":\"";
  json += _pcPower.actionText();
  json += "\",\"attempts\":";
  json += String(_pcPower.attempts());
  json += ",\"statusAgeMs\":";
  json += String(_pcPower.statusAgeMs());
  json += ",\"lastMessage\":\"";
  json += jsonEscape(_pcPower.lastMessage());
  json += "\"},\"pcSettings\":{\"statusActiveHigh\":";
  json += pc.statusActiveHigh ? "true" : "false";
  json += ",\"buttonActiveHigh\":";
  json += pc.buttonActiveHigh ? "true" : "false";
  json += ",\"statusDebounceMs\":";
  json += String(pc.statusDebounceMs);
  json += ",\"buttonPulseMs\":";
  json += String(pc.buttonPulseMs);
  json += ",\"startRetryMs\":";
  json += String(pc.startRetryMs);
  json += ",\"shutdownRetryMs\":";
  json += String(pc.shutdownRetryMs);
  json += ",\"maxStartAttempts\":";
  json += String(pc.maxStartAttempts);
  json += ",\"maxShutdownAttempts\":";
  json += String(pc.maxShutdownAttempts);
  json += "},\"light\":{\"on\":";
  json += _lightController.isOn() ? "true" : "false";
  json += ",\"brightness\":";
  json += String(_lightController.getBrightness());
  json += ",\"mode\":\"";
  json += _lightController.isWhiteMode() ? "white" : "rgb";
  json += "\",\"colorTemp\":";
  json += String(_lightController.getColorTemperature());
  json += ",\"minColorTemp\":";
  json += String(Config::MIN_COLOR_TEMP_MIRED);
  json += ",\"maxColorTemp\":";
  json += String(Config::MAX_COLOR_TEMP_MIRED);
  json += ",\"red\":";
  json += String(_lightController.getRed());
  json += ",\"green\":";
  json += String(_lightController.getGreen());
  json += ",\"blue\":";
  json += String(_lightController.getBlue());
  json += ",\"white\":";
  json += String(_lightController.getWhite());
  json += ",\"warmWhite\":";
  json += String(_lightController.getWarmWhite());
  json += "},\"climate\":{\"valid\":";
  json += _climate.valid() ? "true" : "false";
  json += ",\"temperature\":";
  json += _climate.valid() ? String(_climate.temperature(), 2) : "0";
  json += ",\"humidity\":";
  json += _climate.valid() ? String(_climate.humidity(), 2) : "0";
  json += ",\"lastReadAgeMs\":";
  json += String(_climate.lastReadAgeMs());
  json += ",\"lastSuccessAgeMs\":";
  json += String(lastSuccessAge);
  json += ",\"error\":\"";
  json += jsonEscape(_climate.lastError());
  json += "\"},\"system\":{\"version\":\"";
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

void WebInterface::handleLightSave()
{
  const bool on = argEnabled(_server, "on", _lightController.isOn());
  const uint8_t brightness = static_cast<uint8_t>(argUInt(_server, "brightness", _lightController.getBrightness(), 0, 255));
  const uint16_t colorTemp = static_cast<uint16_t>(argUInt(_server,
                                                          "colorTemp",
                                                          _lightController.getColorTemperature(),
                                                          Config::MIN_COLOR_TEMP_MIRED,
                                                          Config::MAX_COLOR_TEMP_MIRED));
  const uint8_t red = static_cast<uint8_t>(argUInt(_server, "red", _lightController.getRed(), 0, 255));
  const uint8_t green = static_cast<uint8_t>(argUInt(_server, "green", _lightController.getGreen(), 0, 255));
  const uint8_t blue = static_cast<uint8_t>(argUInt(_server, "blue", _lightController.getBlue(), 0, 255));

  if (_server.arg("mode") == "rgb")
  {
    _lightController.setRGBmode();
    _lightController.setRGB(red, green, blue, brightness);
  }
  else
  {
    _lightController.setWWCWmode();
    _lightController.setBrightness(brightness);
    _lightController.setColorTempAndChangeMode(colorTemp);
  }

  if (on)
  {
    _lightController.turnOn();
  }
  else
  {
    _lightController.turnOff();
  }

  sendOk();
}

void WebInterface::handlePcSettingsSave()
{
  PcPowerRuntimeSettings settings = _pcSettings.values();
  settings.statusActiveHigh = argEnabled(_server, "statusActiveHigh", settings.statusActiveHigh);
  settings.buttonActiveHigh = argEnabled(_server, "buttonActiveHigh", settings.buttonActiveHigh);
  settings.statusDebounceMs = argUInt(_server, "statusDebounceMs", settings.statusDebounceMs, 20, 10000);
  settings.buttonPulseMs = argUInt(_server, "buttonPulseMs", settings.buttonPulseMs, 50, 5000);
  settings.startRetryMs = argUInt(_server, "startRetryMs", settings.startRetryMs, 1000, 120000);
  settings.shutdownRetryMs = argUInt(_server, "shutdownRetryMs", settings.shutdownRetryMs, 1000, 120000);
  settings.maxStartAttempts = static_cast<uint8_t>(argUInt(_server, "maxStartAttempts", settings.maxStartAttempts, 1, 10));
  settings.maxShutdownAttempts = static_cast<uint8_t>(argUInt(_server, "maxShutdownAttempts", settings.maxShutdownAttempts, 1, 10));

  if (!_pcSettings.save(settings))
  {
    _server.send(500, "application/json", "{\"ok\":false,\"error\":\"save_failed\"}");
    return;
  }

  _pcPower.applySettings();
  sendOk();
}

void WebInterface::handlePcSettingsReset()
{
  _pcSettings.restoreDefaults();
  _pcPower.applySettings();
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

bool WebInterface::argEnabled(WebServer &server, const char *name, bool fallback)
{
  if (!server.hasArg(name))
  {
    return fallback;
  }

  const String value = server.arg(name);
  return value == "1" || value == "true" || value == "on";
}

uint32_t WebInterface::argUInt(WebServer &server,
                               const char *name,
                               uint32_t fallback,
                               uint32_t minValue,
                               uint32_t maxValue)
{
  if (!server.hasArg(name))
  {
    return fallback;
  }

  uint32_t value = static_cast<uint32_t>(server.arg(name).toInt());
  if (value < minValue)
  {
    value = minValue;
  }
  if (value > maxValue)
  {
    value = maxValue;
  }
  return value;
}
