const char* ap_ssid = "ESP32-CAM-SECURITY";      // Имя вашей Wi-Fi сети
const char* ap_password = "12345678";    // Пароль (минимум 8 символов)

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"       
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h"  
#include "FS.h"             
#include "SD_MMC.h"    

String Feedback = ""; 

String Command = "";
String cmd = "";
String P1 = "";
String P2 = "";
String P3 = "";
String P4 = "";
String P5 = "";
String P6 = "";
String P7 = "";
String P8 = "";
String P9 = "";

byte ReceiveState = 0;
byte cmdState = 1;
byte strState = 1;
byte questionstate = 0;
byte equalstate = 0;
byte semicolonstate = 0;

String currentStyle = "classic";   // classic или hitech

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WiFiServer server(80);
WiFiClient client;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Ошибка камеры 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  sensor_t* s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_SVGA);

  ledcAttachPin(4, 4);
  ledcSetup(4, 5000, 8);

  // Настройка ESP32 как точки доступа
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);

  Serial.println("");
  Serial.println("Точка доступа создана");
  Serial.print("SSID: ");
  Serial.println(ap_ssid);
  Serial.print("Пароль: ");
  Serial.println(ap_password);
  Serial.print("IP-адрес ESP32: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("");

  // Сигнал успешного запуска (вспышки LED)
  for (int i = 0; i < 5; i++) {
    ledcWrite(4, 10);
    delay(200);
    ledcWrite(4, 0);
    delay(200);
  }

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  server.begin();
}

void loop() {
  listenConnection();
}

// -------------------------------------------------------------
// Все остальные функции (ExecuteCommand, listenConnection, 
// handleStream, HTML страницы, status, mainpage, getStill,
// saveCapturedImage, ListImages, deleteimage, showimage, getCommand)
// остаются без изменений. Они приведены ниже для полноты.
// -------------------------------------------------------------

void ExecuteCommand() {
  if (cmd != "getstill") {
    Serial.println("cmd= " + cmd + " ,P1= " + P1 + " ,P2= " + P2 + " ,P3= " + P3 + " ,P4= " + P4 + " ,P5= " + P5 + " ,P6= " + P6 + " ,P7= " + P7 + " ,P8= " + P8 + " ,P9= " + P9);
    Serial.println("");
  }

  if (cmd == "your cmd") {
  } else if (cmd == "restart") {
    ESP.restart();
  } else if (cmd == "flash") {
    ledcAttachPin(4, 4);
    ledcSetup(4, 5000, 8);
    int val = P1.toInt();
    ledcWrite(4, val);
  } else if (cmd == "resetwifi") {
    // Эта команда больше не актуальна для режима AP
    Feedback = "Команда недоступна в режиме точки доступа";
  } else if (cmd == "framesize") {
    int val = P1.toInt();
    sensor_t* s = esp_camera_sensor_get();
    s->set_framesize(s, (framesize_t)val);
  } else if (cmd == "quality") {
    sensor_t* s = esp_camera_sensor_get();
    s->set_quality(s, P1.toInt());
  } else if (cmd == "contrast") {
    sensor_t* s = esp_camera_sensor_get();
    s->set_contrast(s, P1.toInt());
  } else if (cmd == "brightness") {
    sensor_t* s = esp_camera_sensor_get();
    s->set_brightness(s, P1.toInt());
  } else if (cmd == "saturation") {
    sensor_t* s = esp_camera_sensor_get();
    s->set_saturation(s, P1.toInt());
  } else if (cmd == "saveimage") {
    saveCapturedImage(P1);
    Feedback = ListImages();
  } else if (cmd == "listimages") {
    Feedback = ListImages();
  } else if (cmd == "deleteimage") {
    Feedback = deleteimage(P1) + "<br>" + ListImages();
  } else if (cmd == "style") {
    if (P1 == "classic" || P1 == "hitech") {
      currentStyle = P1;
    }
  } else {
    Feedback = "Команда не определена.";
  }
  if (Feedback == "") Feedback = Command;
}

void listenConnection() {
  Feedback = "";
  Command = "";
  cmd = "";
  P1 = P2 = P3 = P4 = P5 = P6 = P7 = P8 = P9 = "";
  ReceiveState = 0, cmdState = 1, strState = 1, questionstate = 0, equalstate = 0, semicolonstate = 0;

  client = server.available();

  if (client) {
    String currentLine = "";
    String requestPath = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        getCommand(c);

        if (c == '\n') {
          if (currentLine.startsWith("GET ")) {
            int firstSpace = currentLine.indexOf(' ');
            int secondSpace = currentLine.indexOf(' ', firstSpace + 1);
            if (firstSpace != -1 && secondSpace != -1) {
              requestPath = currentLine.substring(firstSpace + 1, secondSpace);
            }
          }

          if (currentLine.length() == 0) {
            if (requestPath == "/stream") {
              handleStream();
              break;
            } else if (cmd == "getstill") {
              getStill();
            } else if (cmd == "showimage") {
              showimage();
            } else if (cmd == "status") {
              status();
            } else if (cmd == "style") {
              client.println("HTTP/1.1 302 Found");
              client.println("Location: /");
              client.println();
            } else {
              mainpage();
            }
            Feedback = "";
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        if ((currentLine.indexOf("/?") != -1) && (currentLine.indexOf(" HTTP") != -1)) {
          if (Command.indexOf("stop") != -1) {
            client.println();
            client.println();
            client.stop();
          }
          currentLine = "";
          Feedback = "";
          ExecuteCommand();
        }
      }
    }
    delay(1);
    client.stop();
  }
}

void handleStream() {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=--myboundary");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();

  while (client.connected()) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      break;
    }

    client.print("--myboundary\r\n");
    client.print("Content-Type: image/jpeg\r\n");
    client.print("Content-Length: ");
    client.print(fb->len);
    client.print("\r\n\r\n");
    client.write(fb->buf, fb->len);
    client.print("\r\n");

    esp_camera_fb_return(fb);
    delay(50);
  }
}

// -------------------------------------------------------------
// CLASSIC HTML
// -------------------------------------------------------------
static const char PROGMEM INDEX_HTML_CLASSIC[] = R"rawliteral(
<!doctype html>
<html>
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>ESP32 OV2460 Стоп кадр</title>
    <style>
        body {
            font-family: Arial,Helvetica,sans-serif;
            background: #181818;
            color: #EFEFEF;
            font-size: 16px;
            margin: 20px;
        }
        .container {
            display: flex;
            flex-wrap: wrap;
            gap: 20px;
        }
        .video-col {
            flex: 2;
            min-width: 300px;
        }
        .settings-col {
            flex: 1.2;
            min-width: 280px;
            background: #363636;
            padding: 12px;
            border-radius: 8px;
        }
        .image-container {
            position: relative;
            margin-bottom: 10px;
        }
        .image-container img {
            width: 100%;
            border-radius: 4px;
        }
        .close {
            position: absolute;
            right: 8px;
            top: 8px;
            background: #ff3034;
            width: 24px;
            height: 24px;
            text-align: center;
            line-height: 22px;
            border-radius: 50%;
            cursor: pointer;
            font-weight: bold;
        }
        .hidden {
            display: none;
        }
        button {
            background: #ff3034;
            border: none;
            color: white;
            padding: 6px 12px;
            margin: 4px;
            border-radius: 4px;
            cursor: pointer;
        }
        button:hover {
            background: #ff494d;
        }
        .input-group {
            margin: 12px 0;
            display: flex;
            flex-wrap: wrap;
            align-items: center;
            gap: 8px;
        }
        .input-group label {
            min-width: 110px;
        }
        select, input[type=range] {
            flex: 1;
        }
        iframe {
            width: 100%;
            height: 240px;
            border: 1px solid #555;
            background: #222;
            margin-top: 16px;
        }
        .message {
            color: #ff6666;
            margin-top: 8px;
        }
        hr {
            border-color: #444;
        }
    </style>
</head>
<body>
<div class="container">
    <div class="video-col">
        <div id="stream-container" class="image-container hidden">
            <div class="close" id="close-stream">×</div>
            <img id="stream" src="">
        </div>
        <div id="stream-placeholder" style="background:#222; text-align:center; padding:60px;">[ Видео не активно ]</div>
    </div>
    <div class="settings-col">
        <h3>Настройки камеры</h3>
        <div>
            <button id="restart">Перезапуск</button>
            <button id="startStream">▶ Видео</button>
            <button id="stopStream">⏹ Стоп</button>
            <button id="saveStill">Сохранить на SD</button>
            <button id="imageList">Список файлов</button>
            <hr>
            <button id="styleClassic">Classic Style</button>
            <button id="styleHitech">Hi‑Tech Style</button>
        </div>
        <div class="input-group">
            <label>Подсветка (LED)</label>
            <input type="range" id="flash" min="0" max="255" value="0" class="default-action">
        </div>
        <div class="input-group">
            <label>Разрешение</label>
            <select id="framesize" class="default-action">
                <option value="10">UXGA(1600x1200)</option>
                <option value="9">SXGA(1280x1024)</option>
                <option value="8">XGA(1024x768)</option>
                <option value="7">SVGA(800x600)</option>
                <option value="6">VGA(640x480)</option>
                <option value="5" selected>CIF(400x296)</option>
                <option value="4">QVGA(320x240)</option>
                <option value="3">HQVGA(240x176)</option>
                <option value="0">QQVGA(160x120)</option>
            </select>
        </div>
        <div class="input-group">
            <label>Качество</label>
            <input type="range" id="quality" min="10" max="63" value="10" class="default-action">
        </div>
        <div class="input-group">
            <label>Яркость</label>
            <input type="range" id="brightness" min="-2" max="2" value="1" class="default-action">
        </div>
        <div class="input-group">
            <label>Контраст</label>
            <input type="range" id="contrast" min="-2" max="2" value="1" class="default-action">
        </div>
        <div class="input-group">
            <label>Насыщенность</label>
            <input type="range" id="saturation" min="-2" max="2" value="1" class="default-action">
        </div>
        <div id="message" class="message"></div>
        <hr>
        <div id="filelist-container">
            <iframe id="ifr" style="display:none;"></iframe>
        </div>
    </div>
</div>

<script>
document.addEventListener('DOMContentLoaded', function() {
  const baseHost = location.origin;
  const hide = el => el.classList.add('hidden');
  const show = el => el.classList.remove('hidden');

  function updateConfig(el) {
    let value;
    if (el.type === 'range' || el.type === 'select-one') value = el.value;
    else return;
    fetch(`${baseHost}/?${el.id}=${value}`).catch(e=>console.warn);
  }

  fetch(`${baseHost}/?status`)
    .then(r=>r.json())
    .then(state=>{
      document.querySelectorAll('.default-action').forEach(el=>{
        if(el.id=="flash"){
          el.value=0;
          fetch(baseHost+"/?flash=0");
        } else if(state[el.id]!==undefined){
          el.value = state[el.id];
        }
      });
    });

  const view = document.getElementById('stream');
  const viewContainer = document.getElementById('stream-container');
  const placeholder = document.getElementById('stream-placeholder');
  const streamBtn = document.getElementById('startStream');
  const stopBtn = document.getElementById('stopStream');
  const closeBtn = document.getElementById('close-stream');
  const messageDiv = document.getElementById('message');
  const restartBtn = document.getElementById('restart');
  const saveStill = document.getElementById('saveStill');
  const imageListBtn = document.getElementById('imageList');
  const iframe = document.getElementById('ifr');
  const styleClassic = document.getElementById('styleClassic');
  const styleHitech = document.getElementById('styleHitech');

  let myTimer, restartCount=0;
  let streamState = false;

  function error_handle(){
    restartCount++;
    clearInterval(myTimer);
    if(restartCount<=2){
      messageDiv.innerHTML = `Ошибка потока. Повтор через ${restartCount} мин...`;
      myTimer = setInterval(()=>streamBtn.click(),10000);
    } else {
      messageDiv.innerHTML = "Критическая ошибка. Перезагрузите ESP32-CAM.";
    }
  }

  streamBtn.onclick = ()=>{
    clearInterval(myTimer);
    streamState = true;
    myTimer = setInterval(error_handle,5000);
    placeholder.style.display='none';
    view.src = baseHost+'/?getstill='+Math.random();
    show(viewContainer);
    iframe.style.display='none';
  };
  view.onload = ()=>{
    clearInterval(myTimer);
    restartCount=0;
    if(streamState) streamBtn.click();
  };
  stopBtn.onclick = ()=>{
    clearInterval(myTimer);
    streamState = false;
    placeholder.style.display='block';
    window.stop();
    hide(viewContainer);
    iframe.style.display='none';
  };
  closeBtn.onclick = ()=>{
    hide(viewContainer);
    placeholder.style.display='block';
  };
  imageListBtn.onclick = ()=>{
    show(viewContainer);
    iframe.style.display='block';
    iframe.src = baseHost+'?listimages';
  };
  saveStill.onclick = ()=>{
    show(viewContainer);
    iframe.style.display='block';
    let d = new Date();
    let ts = d.getFullYear()*10000000000+(d.getMonth()+1)*100000000+d.getDate()*1000000+d.getHours()*10000+d.getMinutes()*100+d.getSeconds();
    iframe.src = baseHost+'?saveimage='+ts;
  };
  restartBtn.onclick = ()=>fetch(baseHost+"/?restart");
  styleClassic.onclick = ()=>fetch(baseHost+"/?style=classic").then(()=>location.reload());
  styleHitech.onclick = ()=>fetch(baseHost+"/?style=hitech").then(()=>location.reload());

  document.querySelectorAll('.default-action').forEach(el=>{
    el.onchange = ()=>updateConfig(el);
  });
});
</script>
</body>
</html>
)rawliteral";

// -------------------------------------------------------------
// HI-TECH HTML
// -------------------------------------------------------------
static const char PROGMEM INDEX_HTML_HITECH[] = R"rawliteral(
<!doctype html>
<html>
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>ESP32-CAM // HI‑TECH INTERFACE</title>
    <style>
        * {
            box-sizing: border-box;
        }
        body {
            background: radial-gradient(circle at 20% 30%, #0a0f1e, #03050b);
            font-family: 'Segoe UI', 'Orbitron', 'Arial', sans-serif;
            color: #ccf;
            margin: 0;
            padding: 20px;
            min-height: 100vh;
        }
        body::before {
            content: "";
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 2px;
            background: linear-gradient(90deg, #00f2ff, #a200ff, #00f2ff);
            z-index: 100;
        }
        .container {
            max-width: 1400px;
            margin: 0 auto;
            backdrop-filter: blur(2px);
        }
        .header {
            font-family: 'Orbitron', monospace;
            font-size: 1.6rem;
            font-weight: 500;
            letter-spacing: 4px;
            text-transform: uppercase;
            background: linear-gradient(135deg, #8af, #2af);
            -webkit-background-clip: text;
            background-clip: text;
            color: transparent;
            text-shadow: 0 0 8px rgba(0,170,255,0.3);
            border-bottom: 1px solid rgba(0,200,255,0.4);
            padding-bottom: 8px;
            margin-bottom: 25px;
            display: flex;
            justify-content: space-between;
            align-items: flex-end;
            flex-wrap: wrap;
        }
        .badge {
            font-size: 0.8rem;
            background: rgba(0,200,255,0.15);
            padding: 4px 12px;
            border-radius: 20px;
            letter-spacing: 1px;
            border: 1px solid #0cf;
        }
        .grid {
            display: flex;
            flex-wrap: wrap;
            gap: 24px;
        }
        .video-panel {
            flex: 2;
            min-width: 300px;
            background: rgba(10,20,35,0.6);
            backdrop-filter: blur(4px);
            border: 1px solid rgba(0,200,255,0.3);
            border-radius: 16px;
            padding: 12px;
            box-shadow: 0 0 20px rgba(0,200,255,0.1);
        }
        .video-container {
            position: relative;
            background: #000;
            border-radius: 12px;
            overflow: hidden;
        }
        .video-container img {
            width: 100%;
            display: block;
        }
        .controls-panel {
            flex: 1.2;
            min-width: 280px;
            background: rgba(15,25,45,0.7);
            backdrop-filter: blur(8px);
            border: 1px solid rgba(0,200,255,0.3);
            border-radius: 20px;
            padding: 18px;
            box-shadow: 0 0 20px rgba(0,0,0,0.5);
        }
        .btn-group {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin-bottom: 20px;
        }
        .hitech-btn {
            background: rgba(0,0,0,0.6);
            border: 1px solid #0cf;
            color: #0cf;
            font-family: 'Orbitron', monospace;
            font-size: 0.75rem;
            padding: 8px 14px;
            border-radius: 30px;
            cursor: pointer;
            transition: 0.2s;
            letter-spacing: 1px;
            font-weight: bold;
        }
        .hitech-btn:hover {
            background: #0cf;
            color: #010101;
            box-shadow: 0 0 12px #0cf;
            border-color: #fff;
        }
        .ctrl-card {
            background: rgba(0,0,0,0.4);
            border-radius: 16px;
            padding: 12px;
            margin: 16px 0;
            border-left: 3px solid #0cf;
        }
        .ctrl-card label {
            display: block;
            font-size: 0.7rem;
            text-transform: uppercase;
            letter-spacing: 2px;
            margin-bottom: 8px;
            color: #9cf;
        }
        input[type=range] {
            width: 100%;
            background: #1a2a3a;
            height: 3px;
            border-radius: 3px;
            -webkit-appearance: none;
        }
        input[type=range]:focus {
            outline: none;
        }
        input[type=range]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 14px;
            height: 14px;
            border-radius: 0;
            background: #0cf;
            cursor: pointer;
            box-shadow: 0 0 4px #0cf;
            transform: rotate(45deg);
        }
        select {
            background: #0a1420;
            border: 1px solid #0cf;
            color: #0cf;
            padding: 8px;
            border-radius: 8px;
            width: 100%;
            font-family: monospace;
        }
        .iframe-container {
            margin-top: 20px;
            border-top: 1px dashed rgba(0,200,255,0.3);
            padding-top: 16px;
        }
        iframe {
            width: 100%;
            height: 240px;
            border: 1px solid #0cf;
            background: #03070f;
            border-radius: 12px;
        }
        .message {
            background: rgba(255,50,50,0.2);
            border-left: 4px solid #f33;
            padding: 6px 12px;
            font-size: 0.8rem;
            margin-top: 12px;
        }
        .hidden {
            display: none !important;
        }
        .close {
            position: absolute;
            top: 10px;
            right: 12px;
            background: rgba(0,0,0,0.7);
            color: #0cf;
            width: 26px;
            height: 26px;
            text-align: center;
            line-height: 24px;
            border-radius: 30px;
            cursor: pointer;
            border: 1px solid #0cf;
            font-weight: bold;
        }
        @media (max-width: 760px) {
            .header { font-size: 1.2rem; }
            .hitech-btn { padding: 5px 10px; font-size: 0.7rem; }
        }
    </style>
</head>
<body>
<div class="container">
    <div class="header">
        <span>⚡ ESP32‑CAM <span style="font-weight:300;">//</span> HI‑TECH STREAM</span>
        <span class="badge">v.2.0 | SECURE LINK</span>
    </div>
    <div class="grid">
        <div class="video-panel">
            <div class="video-container">
                <div id="stream-container" class="hidden">
                    <div class="close" id="close-stream">✕</div>
                    <img id="stream" src="">
                </div>
                <div id="stream-placeholder" style="text-align:center; padding:60px; color:#4af;">[ SYSTEM READY | START VIDEO ]</div>
            </div>
        </div>
        <div class="controls-panel">
            <div class="btn-group">
                <button id="restart" class="hitech-btn">⟳ REBOOT</button>
                <button id="startStream" class="hitech-btn">▶ VIDEO</button>
                <button id="stopStream" class="hitech-btn">⏹ STOP</button>
                <button id="saveStill" class="hitech-btn">💾 SAVE SD</button>
                <button id="imageList" class="hitech-btn">📁 FILES</button>
                <button id="styleClassic" class="hitech-btn">◀ CLASSIC</button>
                <button id="styleHitech" class="hitech-btn">HI‑TECH ▶</button>
            </div>
            <div class="ctrl-card">
                <label>🔆 LED FLASH</label>
                <input type="range" id="flash" min="0" max="255" value="0" class="default-action">
            </div>
            <div class="ctrl-card">
                <label>📏 RESOLUTION</label>
                <select id="framesize" class="default-action">
                    <option value="10">UXGA 1600x1200</option><option value="9">SXGA 1280x1024</option>
                    <option value="8">XGA 1024x768</option><option value="7">SVGA 800x600</option>
                    <option value="6">VGA 640x480</option><option value="5" selected>CIF 400x296</option>
                    <option value="4">QVGA 320x240</option><option value="3">HQVGA 240x176</option>
                    <option value="0">QQVGA 160x120</option>
                </select>
            </div>
            <div class="ctrl-card">
                <label>🎞 JPEG QUALITY</label>
                <input type="range" id="quality" min="10" max="63" value="10" class="default-action">
            </div>
            <div class="ctrl-card">
                <label>☀ BRIGHTNESS</label>
                <input type="range" id="brightness" min="-2" max="2" value="1" class="default-action">
            </div>
            <div class="ctrl-card">
                <label>◐ CONTRAST</label>
                <input type="range" id="contrast" min="-2" max="2" value="1" class="default-action">
            </div>
            <div class="ctrl-card">
                <label>🎨 SATURATION</label>
                <input type="range" id="saturation" min="-2" max="2" value="1" class="default-action">
            </div>
            <div id="message" class="message"></div>
            <div class="iframe-container">
                <iframe id="ifr" style="display:none;"></iframe>
            </div>
        </div>
    </div>
</div>
<script>
document.addEventListener('DOMContentLoaded', function() {
  const baseHost = location.origin;
  const hide = el => el.classList.add('hidden');
  const show = el => el.classList.remove('hidden');

  function updateConfig(el) {
    let value = (el.type==='range'||el.type==='select-one') ? el.value : null;
    if(value!==null) fetch(`${baseHost}/?${el.id}=${value}`).catch(e=>console.warn);
  }
  fetch(`${baseHost}/?status`).then(r=>r.json()).then(state=>{
    document.querySelectorAll('.default-action').forEach(el=>{
      if(el.id=="flash"){ el.value=0; fetch(baseHost+"/?flash=0"); }
      else if(state[el.id]!==undefined) el.value = state[el.id];
    });
  });
  const view = document.getElementById('stream');
  const viewContainer = document.getElementById('stream-container');
  const placeholder = document.getElementById('stream-placeholder');
  const streamBtn = document.getElementById('startStream');
  const stopBtn = document.getElementById('stopStream');
  const closeBtn = document.getElementById('close-stream');
  const messageDiv = document.getElementById('message');
  const restartBtn = document.getElementById('restart');
  const saveStill = document.getElementById('saveStill');
  const imageListBtn = document.getElementById('imageList');
  const iframe = document.getElementById('ifr');
  const styleClassic = document.getElementById('styleClassic');
  const styleHitech = document.getElementById('styleHitech');
  let myTimer, restartCount=0;
  let streamState = false;
  function error_handle(){
    restartCount++;
    clearInterval(myTimer);
    if(restartCount<=2) messageDiv.innerHTML = `⚠️ STREAM ERROR. Retry ${restartCount} min...`;
    else messageDiv.innerHTML = "❌ FATAL ERROR. Reboot ESP32-CAM.";
    if(restartCount<=2) myTimer = setInterval(()=>streamBtn.click(),10000);
  }
  streamBtn.onclick = ()=>{
    clearInterval(myTimer);
    streamState = true;
    myTimer = setInterval(error_handle,5000);
    placeholder.style.display='none';
    view.src = baseHost+'/?getstill='+Math.random();
    show(viewContainer);
    iframe.style.display='none';
  };
  view.onload = ()=>{ clearInterval(myTimer); restartCount=0; if(streamState) streamBtn.click(); };
  stopBtn.onclick = ()=>{
    clearInterval(myTimer); streamState = false; placeholder.style.display='block';
    window.stop(); hide(viewContainer); iframe.style.display='none';
  };
  closeBtn.onclick = ()=>{ hide(viewContainer); placeholder.style.display='block'; };
  imageListBtn.onclick = ()=>{ show(viewContainer); iframe.style.display='block'; iframe.src = baseHost+'?listimages'; };
  saveStill.onclick = ()=>{
    show(viewContainer); iframe.style.display='block';
    let d = new Date();
    let ts = d.getFullYear()*10000000000+(d.getMonth()+1)*100000000+d.getDate()*1000000+d.getHours()*10000+d.getMinutes()*100+d.getSeconds();
    iframe.src = baseHost+'?saveimage='+ts;
  };
  restartBtn.onclick = ()=>fetch(baseHost+"/?restart");
  styleClassic.onclick = ()=>fetch(baseHost+"/?style=classic").then(()=>location.reload());
  styleHitech.onclick = ()=>fetch(baseHost+"/?style=hitech").then(()=>location.reload());
  document.querySelectorAll('.default-action').forEach(el=>{ el.onchange = ()=>updateConfig(el); });
});
</script>
</body>
</html>
)rawliteral";

// -------------------------------------------------------------
// Служебные функции
// -------------------------------------------------------------
void status() {
  sensor_t* s = esp_camera_sensor_get();
  String json = "{";
  json += "\"framesize\":" + String(s->status.framesize) + ",";
  json += "\"quality\":" + String(s->status.quality) + ",";
  json += "\"brightness\":" + String(s->status.brightness) + ",";
  json += "\"contrast\":" + String(s->status.contrast) + ",";
  json += "\"saturation\":" + String(s->status.saturation) + ",";
  json += "\"special_effect\":" + String(s->status.special_effect) + ",";
  json += "\"vflip\":" + String(s->status.vflip) + ",";
  json += "\"hmirror\":" + String(s->status.hmirror);
  json += "}";

  client.println("HTTP/1.1 200 OK");
  client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
  client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
  client.println("Content-Type: application/json; charset=utf-8");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();

  for (int Index = 0; Index < json.length(); Index = Index + 1024) {
    client.print(json.substring(Index, Index + 1024));
  }
}

void mainpage() {
  client.println("HTTP/1.1 200 OK");
  client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
  client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
  client.println("Content-Type: text/html; charset=utf-8");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();

  String Data = "";
  if (cmd != "")
    Data = Feedback;
  else {
    if (currentStyle == "hitech")
      Data = String((const char*)INDEX_HTML_HITECH);
    else
      Data = String((const char*)INDEX_HTML_CLASSIC);
  }

  for (int Index = 0; Index < Data.length(); Index = Index + 1024) {
    client.print(Data.substring(Index, Index + 1024));
  }
}

void getStill() {
  camera_fb_t* fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Сбой захвата камеры");
    delay(1000);
    ESP.restart();
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
  client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
  client.println("Content-Type: image/jpeg");
  client.println("Content-Disposition: form-data; name=\"imageFile\"; filename=\"picture.jpg\"");
  client.println("Content-Length: " + String(fb->len));
  client.println("Connection: close");
  client.println();

  uint8_t* fbBuf = fb->buf;
  size_t fbLen = fb->len;
  for (size_t n = 0; n < fbLen; n = n + 1024) {
    if (n + 1024 < fbLen) {
      client.write(fbBuf, 1024);
      fbBuf += 1024;
    } else if (fbLen % 1024 > 0) {
      size_t remainder = fbLen % 1024;
      client.write(fbBuf, remainder);
    }
  }
  esp_camera_fb_return(fb);
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
}

String saveCapturedImage(String filename) {
  String response = "";
  String path_jpg = "/" + filename + ".jpg";

  camera_fb_t* fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Сбой захвата камеры");
    return "<font color=Red>Сбой захвата камеры</font>";
  }

  if (!SD_MMC.begin()) {
    response = "Не установлена Карта памяти";
    return "<font color=yellow>Не установлена Карта памяти</font>";
  }

  fs::FS& fs = SD_MMC;
  Serial.printf("Имя файла-картинки: %s\n", path_jpg.c_str());

  File file = fs.open(path_jpg.c_str(), FILE_WRITE);
  if (!file) {
    esp_camera_fb_return(fb);
    SD_MMC.end();
    return "<font color=Red>Не удалось открыть файл в режиме записи</font>";
  } else {
    file.write(fb->buf, fb->len);
    Serial.printf("Файл записан по адресу: %s\n", path_jpg.c_str());
  }
  file.close();
  SD_MMC.end();

  esp_camera_fb_return(fb);
  Serial.println("");

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  return response;
}

String ListImages() {
  if (!SD_MMC.begin()) {
    Serial.println("Не установлена Карта памяти");
    return "<font color=Red>Не установлена Карта памяти</font>";
  }

  fs::FS& fs = SD_MMC;
  File root = fs.open("/");
  if (!root) {
    Serial.println("Не удалось открыть каталог");
    return "<font color=Red>Не удалось открыть каталог</font>";
  }

  String list = "";
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String filename = String(file.name());
      if (filename.startsWith("/")) filename = filename.substring(1);
      String row = "<td>"
                   "<td><font color=Green>" + filename + "</font></td>"
                   "<td align='right'><font color=Green>" + String(file.size()) + "</font></td>"
                   "<td><button onclick='location.href = location.origin + \"?deleteimage=" + filename + "\";'>Удалить</button></td>"
                   "</tr>";
      list = row + list;
    }
    file = root.openNextFile();
  }
  list = "<table border=1><tr><th>Имя файла</th><th>Размер</th><th>Удалить</th></tr>" + list + "</table>";

  file.close();
  SD_MMC.end();

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  return list;
}

String deleteimage(String filename) {
  if (!SD_MMC.begin()) {
    Serial.println("Не установлена Карта памяти");
    return "Не установлена Карта памяти";
  }

  fs::FS& fs = SD_MMC;
  String fullPath = filename;
  if (!fullPath.startsWith("/")) fullPath = "/" + fullPath;
  String message = "";
  if (fs.remove(fullPath)) {
    message = "<font color=Red>" + filename + " удалён</font>";
  } else {
    message = "<font color=Red>Ошибка удаления " + filename + "</font>";
  }
  SD_MMC.end();

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  return message;
}

void showimage() {
  if (!SD_MMC.begin()) {
    Serial.println("Не установлена Карта памяти");
    return;
  }
  Serial.println(P1);
  fs::FS& fs = SD_MMC;
  String filepath = P1;
  if (!filepath.startsWith("/")) filepath = "/" + filepath;

  File file = fs.open(filepath);
  if (!file) {
    Serial.println("Не удалось открыть файл для чтения");
    SD_MMC.end();
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("File not found");
    return;
  } else {
    Serial.println("Считать файл: " + filepath);
    Serial.println("Размер файла: " + String(file.size()));

    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
    client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
    client.println("Content-Type: image/jpeg");
    String dispName = P1;
    if (dispName.startsWith("/")) dispName = dispName.substring(1);
    client.println("Content-Disposition: form-data; name=\"imageFile\"; filename=\"" + dispName + "\"");
    client.println("Content-Length: " + String(file.size()));
    client.println("Connection: close");
    client.println();

    byte buf[1024];
    int i = -1;
    while (file.available()) {
      i++;
      buf[i] = file.read();
      if (i == (sizeof(buf) - 1)) {
        client.write((const uint8_t*)buf, sizeof(buf));
        i = -1;
      } else if (!file.available()) {
        client.write((const uint8_t*)buf, i + 1);
      }
    }
    client.println();
    file.close();
    SD_MMC.end();
  }
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
}

void getCommand(char c) {
  if (c == '?') ReceiveState = 1;
  if ((c == ' ') || (c == '\r') || (c == '\n')) ReceiveState = 0;

  if (ReceiveState == 1) {
    Command = Command + String(c);

    if (c == '=') cmdState = 0;
    if (c == ';') strState++;

    if ((cmdState == 1) && ((c != '?') || (questionstate == 1))) cmd = cmd + String(c);
    if ((cmdState == 0) && (strState == 1) && ((c != '=') || (equalstate == 1))) P1 = P1 + String(c);
    if ((cmdState == 0) && (strState == 2) && (c != ';')) P2 = P2 + String(c);
    if ((cmdState == 0) && (strState == 3) && (c != ';')) P3 = P3 + String(c);
    if ((cmdState == 0) && (strState == 4) && (c != ';')) P4 = P4 + String(c);
    if ((cmdState == 0) && (strState == 5) && (c != ';')) P5 = P5 + String(c);
    if ((cmdState == 0) && (strState == 6) && (c != ';')) P6 = P6 + String(c);
    if ((cmdState == 0) && (strState == 7) && (c != ';')) P7 = P7 + String(c);
    if ((cmdState == 0) && (strState == 8) && (c != ';')) P8 = P8 + String(c);
    if ((cmdState == 0) && (strState >= 9) && ((c != ';') || (semicolonstate == 1))) P9 = P9 + String(c);

    if (c == '?') questionstate = 1;
    if (c == '=') equalstate = 1;
    if ((strState >= 9) && (c == ';')) semicolonstate = 1;
  }
}
