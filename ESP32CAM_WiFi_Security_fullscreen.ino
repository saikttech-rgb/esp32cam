/*
  ESP32-CAM WiFi Camera with Hi-Tech Web Interface
  Автор: sa
  Режим: Access Point + Web Server
  Auth: admin/admin
  
  Полная версия с:
  - MJPEG видеопотоком
  - Записью на SD (ручная + motion detection)
  - Управлением файлами
  - Автоудалением при заполнении
  - Очисткой всех файлов
  - Переворотом изображения (hmirror + vflip)
  - Полноэкранным режимом (двойной клик + кнопка)

Q: Как использовать полноэкранный режим?
A: Есть два способа:
Двойной клик по видео — вход/выход из fullscreen
Кнопка ⛶ — появляется при наведении на видео, нажмите для входа/выхода
Клавиша ESC — стандартный выход из fullscreen
Q: Fullscreen не работает в моём браузере?
A: Fullscreen API поддерживается во всех современных браузерах. Если не работает:
Обновите браузер до последней версии
Проверьте, что JavaScript включён
Попробуйте F11 для полноэкранного режима браузера
Некоторые расширения браузера могут блокировать fullscreen
*/

#define CAMERA_MODEL_AI_THINKER

#include "esp_camera.h"
#include "WiFi.h"
#include "WebServer.h"
#include "FS.h"
#include "SD_MMC.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_int_wdt.h"
#include "esp_task_wdt.h"

// ==================== НАСТРОЙКИ WiFi ====================
const char* AP_SSID = "ESP32-CAM-Security";
const char* AP_PASSWORD = "12345678";
const char* HTTP_USERNAME = "admin";
const char* HTTP_PASSWORD = "admin";

// ==================== ПИНЫ КАМЕРЫ (AI-Thinker) ====================
#if defined(CAMERA_MODEL_AI_THINKER)
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
#else
  #error "Camera model not selected"
#endif

// ==================== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ====================
WebServer server(80);
bool isRecording = false;
bool isMotionMode = false;
bool isAutoDeleteEnabled = true;
int autoDeleteThreshold = 95;
int currentFrameSize = FRAMESIZE_VGA;
int motionSensitivity = 15000;
unsigned long lastMotionTime = 0;
bool motionDetected = false;
uint32_t frameCounter = 0;

uint8_t* prevFrame = nullptr;
size_t prevFrameSize = 0;

// ==================== ПРОТОТИПЫ ФУНКЦИЙ ====================
void initCamera();
void initSDCard();
void initWiFi();
void setupWebServer();
void handleRoot();
void handleStream();
void handleCapture();
void handleStatus();
void handleControl();
void handleRec();
void handleFormat();
void handleDelete();
void handleDownload();
void handleNotFound();
bool checkAuth();
bool deleteOldestFile();
void autoDeleteWhenFull();
uint64_t getSDUsedBytes();
uint64_t getSDTotalBytes();
void recordFrame();
bool detectMotionAccurate(camera_fb_t* fb);
void processMotionDetection();
void processMotionRecording();
void processManualRecording();
void printSystemInfo();

// ==================== HTML/CSS/JS В ПРОСТРАНСТВЕ PROGMEM ====================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32-CAM</title>
<style>
* { margin:0; padding:0; box-sizing:border-box; }
body {
  background: #0a0a0f;
  color: #00ffff;
  font-family: 'Courier New', monospace;
  min-height: 100vh;
  display: flex;
  flex-direction: column;
  background-image: 
    linear-gradient(rgba(0,255,255,0.03) 1px, transparent 1px),
    linear-gradient(90deg, rgba(0,255,255,0.03) 1px, transparent 1px);
  background-size: 30px 30px;
}
header {
  padding: 15px 25px;
  border-bottom: 1px solid rgba(0,255,255,0.3);
  background: rgba(0,0,0,0.7);
  display: flex;
  justify-content: space-between;
  align-items: center;
  box-shadow: 0 0 20px rgba(0,255,255,0.2);
}
header h1 {
  font-size: 1.3em;
  letter-spacing: 3px;
  text-shadow: 0 0 10px #00ffff, 0 0 20px #00ffff;
}
.status-indicator {
  padding: 5px 15px;
  border: 1px solid #ff00ff;
  color: #ff00ff;
  font-size: 0.85em;
  text-shadow: 0 0 5px #ff00ff;
  box-shadow: 0 0 10px rgba(255,0,255,0.4);
}
.status-indicator.rec {
  background: #ff0033;
  color: #fff;
  border-color: #ff0033;
  animation: pulse 1s infinite;
}
@keyframes pulse {
  0%,100% { box-shadow: 0 0 10px #ff0033; }
  50% { box-shadow: 0 0 25px #ff0033, 0 0 40px #ff0033; }
}
.container {
  flex: 1;
  display: grid;
  grid-template-columns: 1fr 350px;
  gap: 20px;
  padding: 20px;
}
@media (max-width: 900px) {
  .container { grid-template-columns: 1fr; }
}
.panel {
  background: rgba(17,17,24,0.85);
  border: 1px solid rgba(0,255,255,0.25);
  padding: 18px;
  box-shadow: 0 0 15px rgba(0,255,255,0.1), inset 0 0 20px rgba(0,0,0,0.5);
}
.panel h2 {
  color: #ff00ff;
  font-size: 1em;
  letter-spacing: 2px;
  margin-bottom: 15px;
  padding-bottom: 8px;
  border-bottom: 1px solid rgba(255,0,255,0.3);
  text-shadow: 0 0 8px #ff00ff;
}
details {
  border: 1px solid rgba(0,255,255,0.3);
  margin-bottom: 15px;
}
details summary {
  padding: 10px 15px;
  cursor: pointer;
  background: rgba(0,255,255,0.05);
  letter-spacing: 2px;
  user-select: none;
  list-style: none;
}
details summary::-webkit-details-marker { display: none; }
details summary::before {
  content: '▶ ';
  color: #00ffff;
  margin-right: 8px;
}
details[open] summary::before { content: '▼ '; }
details[open] summary {
  border-bottom: 1px solid rgba(0,255,255,0.3);
}
.video-wrap {
  position: relative;
  padding: 10px;
  background: #000;
  text-align: center;
  min-height: 200px;
  display: flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
}
.video-wrap::after {
  content: '⛶ Двойной клик — полный экран';
  position: absolute;
  top: 10px;
  right: 10px;
  padding: 4px 10px;
  background: rgba(0,0,0,0.7);
  color: #00ffff;
  font-size: 0.7em;
  letter-spacing: 1px;
  border: 1px solid rgba(0,255,255,0.3);
  border-radius: 3px;
  opacity: 0;
  transition: opacity 0.3s;
  pointer-events: none;
  text-shadow: 0 0 5px #00ffff;
  z-index: 5;
}
.video-wrap:hover::after {
  opacity: 1;
}
.video-wrap:fullscreen {
  background: #000;
  display: flex;
  align-items: center;
  justify-content: center;
}
.video-wrap:fullscreen img {
  max-width: 100%;
  max-height: 100vh;
  border: none;
  box-shadow: 0 0 30px rgba(0,255,255,0.5);
}
.video-wrap:fullscreen::after {
  content: '⛶ ESC — выйти из полного экрана';
}
.video-wrap img {
  max-width: 100%;
  height: auto;
  border: 1px solid rgba(0,255,255,0.3);
  box-shadow: 0 0 15px rgba(0,255,255,0.3);
}
.video-wrap.off {
  color: #555;
  font-style: italic;
}
.fullscreen-btn {
  position: absolute;
  bottom: 15px;
  right: 15px;
  width: 40px;
  height: 40px;
  background: rgba(0,0,0,0.8);
  color: #00ffff;
  border: 1px solid #00ffff;
  font-size: 1.3em;
  cursor: pointer;
  opacity: 0;
  transition: all 0.3s;
  z-index: 10;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 3px;
}
.video-wrap:hover .fullscreen-btn {
  opacity: 1;
}
.fullscreen-btn:hover {
  background: rgba(0,255,255,0.2);
  box-shadow: 0 0 15px #00ffff;
  transform: scale(1.1);
}
.fullscreen-btn:active {
  transform: scale(0.95);
}
.controls {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 15px;
  margin-top: 15px;
}
.control-group label {
  display: block;
  font-size: 0.75em;
  color: #888;
  margin-bottom: 5px;
  letter-spacing: 1px;
}
select, input[type="range"], input[type="number"] {
  width: 100%;
  padding: 8px;
  background: #0a0a0f;
  color: #00ffff;
  border: 1px solid rgba(0,255,255,0.4);
  font-family: inherit;
  font-size: 0.9em;
  outline: none;
}
select:focus, input:focus {
  border-color: #00ffff;
  box-shadow: 0 0 10px rgba(0,255,255,0.5);
}
select option { background: #0a0a0f; }
.mode-toggle {
  display: flex;
  gap: 5px;
  margin-top: 5px;
}
.mode-toggle button {
  flex: 1;
  padding: 8px;
  background: transparent;
  color: #00ffff;
  border: 1px solid rgba(0,255,255,0.4);
  font-family: inherit;
  cursor: pointer;
  font-size: 0.8em;
  letter-spacing: 1px;
  transition: all 0.2s;
}
.mode-toggle button.active {
  background: rgba(0,255,255,0.2);
  box-shadow: 0 0 10px rgba(0,255,255,0.5);
  border-color: #00ffff;
}
.rec-btn {
  width: 100%;
  padding: 15px;
  margin-top: 15px;
  background: linear-gradient(135deg, #330000, #660000);
  color: #ff3333;
  border: 2px solid #ff0033;
  font-family: inherit;
  font-size: 1.2em;
  letter-spacing: 4px;
  cursor: pointer;
  font-weight: bold;
  transition: all 0.2s;
}
.rec-btn:hover:not(:disabled) {
  background: linear-gradient(135deg, #660000, #aa0000);
  box-shadow: 0 0 20px #ff0033;
}
.rec-btn.active {
  background: #ff0033;
  color: #fff;
  animation: pulse 1s infinite;
}
.rec-btn:disabled {
  opacity: 0.3;
  cursor: not-allowed;
}
.mem-bar {
  width: 100%;
  height: 20px;
  background: #0a0a0f;
  border: 1px solid rgba(0,255,255,0.3);
  margin: 10px 0;
  position: relative;
  overflow: hidden;
}
.mem-bar-fill {
  height: 100%;
  background: linear-gradient(90deg, #00ffff, #ff00ff);
  transition: width 0.5s;
  box-shadow: 0 0 10px #00ffff;
}
.mem-bar-text {
  position: absolute;
  top: 0; left: 0; right: 0; bottom: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 0.75em;
  color: #fff;
  text-shadow: 0 0 3px #000;
  letter-spacing: 1px;
}
.file-list {
  max-height: 250px;
  overflow-y: auto;
  border: 1px solid rgba(0,255,255,0.2);
  padding: 5px;
  background: rgba(0,0,0,0.4);
}
.file-list::-webkit-scrollbar { width: 6px; }
.file-list::-webkit-scrollbar-track { background: #0a0a0f; }
.file-list::-webkit-scrollbar-thumb { background: #00ffff; }
.file-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 6px 8px;
  border-bottom: 1px solid rgba(0,255,255,0.1);
  font-size: 0.8em;
}
.file-item:hover { background: rgba(0,255,255,0.05); }
.file-name {
  color: #00ffff;
  flex: 1;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}
.file-size {
  color: #888;
  margin: 0 10px;
  min-width: 60px;
  text-align: right;
}
.file-actions { display: flex; gap: 5px; }
.btn-sm {
  padding: 3px 8px;
  background: transparent;
  border: 1px solid;
  font-family: inherit;
  font-size: 0.85em;
  cursor: pointer;
  color: inherit;
}
.btn-del { color: #ff3333; border-color: #ff3333; }
.btn-del:hover { background: rgba(255,0,0,0.2); }
.btn-dl { color: #00ffff; border-color: #00ffff; }
.btn-dl:hover { background: rgba(0,255,255,0.2); }
.btn-danger {
  width: 100%;
  padding: 10px;
  background: transparent;
  color: #ff3333;
  border: 1px solid #ff3333;
  font-family: inherit;
  letter-spacing: 2px;
  cursor: pointer;
  margin-top: 10px;
}
.btn-danger:hover {
  background: rgba(255,0,0,0.2);
  box-shadow: 0 0 15px rgba(255,0,0,0.5);
}
.auto-delete-row {
  display: flex;
  align-items: center;
  gap: 10px;
  margin: 10px 0;
  font-size: 0.85em;
}
.auto-delete-row input[type="checkbox"] {
  width: 18px;
  height: 18px;
  accent-color: #00ffff;
}
.threshold-row {
  display: flex;
  align-items: center;
  gap: 10px;
  margin-top: 8px;
}
.threshold-row input[type="range"] { flex: 1; }
.threshold-val {
  color: #ff00ff;
  min-width: 40px;
  text-align: right;
  text-shadow: 0 0 5px #ff00ff;
}
footer {
  padding: 12px;
  text-align: center;
  border-top: 1px solid rgba(0,255,255,0.3);
  background: rgba(0,0,0,0.7);
  color: #ff00ff;
  font-size: 0.8em;
  letter-spacing: 3px;
  text-shadow: 0 0 8px #ff00ff;
}
.empty-msg {
  text-align: center;
  padding: 20px;
  color: #555;
  font-style: italic;
  font-size: 0.85em;
}
</style>
</head>
<body>

<header>
  <h1>◢ ESP32-CAM Home Security ◣</h1>
  <div id="statusIndicator" class="status-indicator">IDLE</div>
</header>

<div class="container">
  <div class="main-area">
    <div class="panel">
      <h2>[VIDEO FEED]</h2>
      <details id="videoDetails" open>
        <summary>ТРАНСЛЯЦИЯ</summary>
        <div class="video-wrap" id="videoWrap">
          <img id="videoStream" src="/stream" alt="stream">
          <button class="fullscreen-btn" onclick="toggleFullscreen()" title="Полный экран">⛶</button>
        </div>
      </details>

      <div class="controls">
        <div class="control-group">
          <label>КАЧЕСТВО ЗАПИСИ</label>
          <select id="qualitySelect" onchange="setQuality(this.value)">
            <option value="11">QVGA 320x240</option>
            <option value="10">CIF 400x296</option>
            <option value="9">HVGA 480x320</option>
            <option value="7" selected>VGA 640x480</option>
            <option value="6">SVGA 800x600</option>
            <option value="5">XGA 1024x768</option>
            <option value="4">HD 1280x720</option>
            <option value="3">SXGA 1280x1024</option>
            <option value="2">UXGA 1600x1200</option>
          </select>
        </div>
        <div class="control-group">
          <label>ЧУВСТВИТЕЛЬНОСТЬ ДВИЖЕНИЯ</label>
          <input type="range" id="sensSlider" min="5000" max="50000" step="1000" value="15000" onchange="setSensitivity(this.value)">
          <div style="text-align:right;font-size:0.75em;color:#888;margin-top:3px;" id="sensVal">15000</div>
        </div>
      </div>

      <div class="control-group" style="margin-top:15px;">
        <label>РЕЖИМ ЗАПИСИ НА SD</label>
        <div class="mode-toggle">
          <button id="modeManual" class="active" onclick="setMode('manual')">ВРУЧНУЮ</button>
          <button id="modeMotion" onclick="setMode('motion')">ПРИ ДВИЖЕНИИ</button>
        </div>
      </div>

      <button id="recBtn" class="rec-btn" onclick="toggleRec()">▶ REC</button>
    </div>
  </div>

  <div class="sidebar">
    <div class="panel">
      <h2>[SD UTILITIES]</h2>
      
      <label style="font-size:0.75em;color:#888;letter-spacing:1px;">ПАМЯТЬ</label>
      <div class="mem-bar">
        <div class="mem-bar-fill" id="memFill" style="width:0%"></div>
        <div class="mem-bar-text" id="memText">-- / --</div>
      </div>

      <div class="auto-delete-row">
        <input type="checkbox" id="autoDeleteChk" onchange="setAutoDelete(this.checked)">
        <label for="autoDeleteChk" style="color:#00ffff;">АВТОУДАЛЕНИЕ ПРИ ЗАПОЛНЕНИИ</label>
      </div>
      <div class="threshold-row">
        <label style="font-size:0.75em;color:#888;">ПОРОГ:</label>
        <input type="range" id="threshSlider" min="70" max="99" value="95" onchange="setThreshold(this.value)">
        <span class="threshold-val" id="threshVal">95%</span>
      </div>

      <h2 style="margin-top:20px;">[ФАЙЛЫ]</h2>
      <div class="file-list" id="fileList">
        <div class="empty-msg">Загрузка...</div>
      </div>

      <button class="btn-danger" onclick="formatSD()">⚠ ОЧИСТИТЬ ВСЕ ФАЙЛЫ</button>
    </div>
  </div>
</div>

<footer>© sa</footer>

<script>
let currentMode = 'manual';
let isRecording = false;
const videoWrap = document.getElementById('videoWrap');

// Сворачивание видеопотока
document.getElementById('videoDetails').addEventListener('toggle', function(e) {
  if (document.fullscreenElement) {
    return;
  }
  
  const img = document.getElementById('videoStream');
  const wrap = document.getElementById('videoWrap');
  if (this.open) {
    img.src = '/stream?' + Date.now();
    wrap.classList.remove('off');
  } else {
    img.src = '';
    wrap.classList.add('off');
    wrap.innerHTML = '<span style="color:#555;font-style:italic;">[ ПОТОК ОСТАНОВЛЕН ]</span>';
  }
  if (this.open && !wrap.querySelector('img')) {
    wrap.innerHTML = '<img id="videoStream" src="/stream" alt="stream"><button class="fullscreen-btn" onclick="toggleFullscreen()" title="Полный экран">⛶</button>';
  }
});

// ==================== FULLSCREEN ПО ДВОЙНОМУ КЛИКУ ====================
videoWrap.addEventListener('dblclick', function(e) {
  e.preventDefault();
  e.stopPropagation();
  
  if (document.fullscreenElement) {
    document.exitFullscreen().catch(err => {
      console.error('Fullscreen exit error:', err);
    });
    return;
  }
  
  if (videoWrap.requestFullscreen) {
    videoWrap.requestFullscreen().catch(err => {
      console.error('Fullscreen enter error:', err);
    });
  } else if (videoWrap.webkitRequestFullscreen) {
    videoWrap.webkitRequestFullscreen();
  } else if (videoWrap.msRequestFullscreen) {
    videoWrap.msRequestFullscreen();
  }
});

// ==================== FULLSCREEN ПО КНОПКЕ ====================
function toggleFullscreen() {
  if (document.fullscreenElement) {
    document.exitFullscreen().catch(err => {
      console.error('Fullscreen exit error:', err);
    });
  } else {
    if (videoWrap.requestFullscreen) {
      videoWrap.requestFullscreen().catch(err => {
        console.error('Fullscreen enter error:', err);
      });
    } else if (videoWrap.webkitRequestFullscreen) {
      videoWrap.webkitRequestFullscreen();
    } else if (videoWrap.msRequestFullscreen) {
      videoWrap.msRequestFullscreen();
    }
  }
}

// Отслеживание изменений fullscreen
document.addEventListener('fullscreenchange', function() {
  const isFullscreen = !!document.fullscreenElement;
  console.log('Fullscreen:', isFullscreen ? 'ON' : 'OFF');
  videoWrap.classList.toggle('is-fullscreen', isFullscreen);
});

// Обновление статуса
function updateStatus() {
  fetch('/status')
    .then(r => r.json())
    .then(data => {
      const pct = data.sd_total > 0 ? (data.sd_used / data.sd_total * 100) : 0;
      document.getElementById('memFill').style.width = pct + '%';
      document.getElementById('memText').textContent = 
        formatMB(data.sd_used) + ' / ' + formatMB(data.sd_total);
      
      currentMode = data.mode;
      document.getElementById('modeManual').classList.toggle('active', data.mode === 'manual');
      document.getElementById('modeMotion').classList.toggle('active', data.mode === 'motion');
      
      const ind = document.getElementById('statusIndicator');
      const recBtn = document.getElementById('recBtn');
      if (data.recording) {
        isRecording = true;
        ind.textContent = data.mode === 'motion' ? 'MOTION REC' : 'REC';
        ind.classList.add('rec');
        recBtn.classList.add('active');
        recBtn.textContent = '■ STOP';
      } else {
        isRecording = false;
        ind.textContent = 'IDLE';
        ind.classList.remove('rec');
        recBtn.classList.remove('active');
        recBtn.textContent = '▶ REC';
      }
      
      document.getElementById('autoDeleteChk').checked = data.auto_delete;
      document.getElementById('threshSlider').value = data.auto_threshold;
      document.getElementById('threshVal').textContent = data.auto_threshold + '%';
      
      document.getElementById('qualitySelect').value = data.framesize;
      
      document.getElementById('sensSlider').value = data.sensitivity;
      document.getElementById('sensVal').textContent = data.sensitivity;
      
      renderFiles(data.files || []);
    })
    .catch(e => console.error('Status error:', e));
}

function formatMB(bytes) {
  if (bytes < 1024*1024) return (bytes/1024).toFixed(1) + ' KB';
  if (bytes < 1024*1024*1024) return (bytes/1024/1024).toFixed(1) + ' MB';
  return (bytes/1024/1024/1024).toFixed(2) + ' GB';
}

function renderFiles(files) {
  const list = document.getElementById('fileList');
  if (!files.length) {
    list.innerHTML = '<div class="empty-msg">[ НЕТ ФАЙЛОВ ]</div>';
    return;
  }
  list.innerHTML = files.map(f => `
    <div class="file-item">
      <span class="file-name" title="${f.name}">${f.name}</span>
      <span class="file-size">${formatMB(f.size)}</span>
      <div class="file-actions">
        <button class="btn-sm btn-dl" onclick="downloadFile('${f.name}')">↓</button>
        <button class="btn-sm btn-del" onclick="deleteFile('${f.name}')">×</button>
      </div>
    </div>
  `).join('');
}

function apiPost(url, data) {
  fetch(url, {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify(data)
  }).then(() => setTimeout(updateStatus, 300));
}

function setQuality(v) { apiPost('/control', {framesize: parseInt(v)}); }
function setMode(m) { apiPost('/control', {mode: m}); }
function setSensitivity(v) { 
  document.getElementById('sensVal').textContent = v;
  apiPost('/control', {sensitivity: parseInt(v)}); 
}
function setAutoDelete(v) { apiPost('/control', {auto_delete: v}); }
function setThreshold(v) {
  document.getElementById('threshVal').textContent = v + '%';
  apiPost('/control', {auto_threshold: parseInt(v)});
}
function toggleRec() { apiPost('/rec', {}); }

function deleteFile(name) {
  if (!confirm('Удалить файл: ' + name + '?')) return;
  fetch('/delete?file=' + encodeURIComponent(name), {method:'POST'})
    .then(() => setTimeout(updateStatus, 300));
}

function downloadFile(name) {
  window.location.href = '/download?file=' + encodeURIComponent(name);
}

function formatSD() {
  if (!confirm('⚠ ВНИМАНИЕ!\nВсе файлы в папке /img будут УДАЛЕНЫ.\nФайловая система SD-карты НЕ будет затронута.\n\nПродолжить?')) return;
  
  const btn = event.target;
  const originalText = btn.textContent;
  btn.textContent = '⏳ ОЧИСТКА...';
  btn.disabled = true;
  
  fetch('/format', {method:'POST'})
    .then(r => r.json())
    .then(data => {
      alert(`✅ Очистка завершена!\n\nУдалено файлов: ${data.deleted}\nОшибок: ${data.failed}\nОсвобождено: ${formatMB(data.freed)}`);
      setTimeout(updateStatus, 500);
    })
    .catch(err => {
      alert('❌ Ошибка очистки: ' + err.message);
    })
    .finally(() => {
      btn.textContent = originalText;
      btn.disabled = false;
    });
}

updateStatus();
setInterval(updateStatus, 3000);
</script>

</body>
</html>
)rawliteral";

// ==================== SETUP ====================
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("╔══════════════════════════════════════╗");
  Serial.println("║   ESP32-CAM HI-TECH CAMERA SYSTEM   ║");
  Serial.println("║         Author: sa (2026)           ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();

  initCamera();
  initSDCard();
  initWiFi();
  setupWebServer();
  printSystemInfo();
  
  Serial.println("╔══════════════════════════════════════╗");
  Serial.println("║         SYSTEM READY                 ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("WiFi SSID: ESP32-CAM");
  Serial.println("WiFi Password: 12345678");
  Serial.println("Web Login: admin / admin");
  Serial.println("Open: http://192.168.4.1");
  Serial.println();
}

// ==================== LOOP ====================
void loop() {
  server.handleClient();
  
  if (isMotionMode) {
    processMotionDetection();
    processMotionRecording();
  }
  
  if (!isMotionMode && isRecording) {
    processManualRecording();
  }
  
  if (isAutoDeleteEnabled) {
    autoDeleteWhenFull();
  }
  
  delay(1);
}

// ==================== ИНИЦИАЛИЗАЦИЯ КАМЕРЫ ====================
void initCamera() {
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
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  sensor_t* s = esp_camera_sensor_get();
  s->set_brightness(s, 0);
  s->set_contrast(s, 0);
  s->set_saturation(s, 0);
  s->set_special_effect(s, 0);
  s->set_whitebal(s, 1);
  s->set_awb_gain(s, 1);
  s->set_wb_mode(s, 0);
  s->set_exposure_ctrl(s, 1);
  s->set_aec2(s, 0);
  s->set_ae_level(s, 0);
  s->set_aec_value(s, 300);
  s->set_gain_ctrl(s, 1);
  s->set_agc_gain(s, 0);
  s->set_gainceiling(s, (gainceiling_t)0);
  s->set_bpc(s, 0);
  s->set_wpc(s, 1);
  s->set_raw_gma(s, 1);
  s->set_lenc(s, 1);
  s->set_hmirror(s, 1);  // ПЕРЕВЕРНУТО
  s->set_vflip(s, 1);    // ПЕРЕВЕРНУТО
  s->set_dcw(s, 1);
  s->set_bpc(s, 0);
  
  Serial.println("Camera initialized");
}

// ==================== ИНИЦИАЛИЗАЦИЯ SD КАРТЫ ====================
void initSDCard() {
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD Card mount failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD Card attached");
    return;
  }
  
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) Serial.println("MMC");
  else if (cardType == CARD_SD) Serial.println("SDSC");
  else if (cardType == CARD_SDHC) Serial.println("SDHC");
  else Serial.println("UNKNOWN");
  
  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  
  if (!SD_MMC.exists("/img")) {
    SD_MMC.mkdir("/img");
    Serial.println("Created /img directory");
  }
}

// ==================== ИНИЦИАЛИЗАЦИЯ WiFi ====================
void initWiFi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

// ==================== НАСТРОЙКА ВЕБ-СЕРВЕРА ====================
void setupWebServer() {
  server.on("/", HTTP_GET, []() {
    if (!checkAuth()) {
      server.requestAuthentication(BASIC_AUTH, "ESP32-CAM", "Authentication required");
      return;
    }
    handleRoot();
  });
  
  server.on("/stream", HTTP_GET, handleStream);
  
  server.on("/capture", HTTP_GET, []() {
    if (!checkAuth()) {
      server.requestAuthentication(BASIC_AUTH, "ESP32-CAM", "Authentication required");
      return;
    }
    handleCapture();
  });
  
  server.on("/status", HTTP_GET, []() {
    if (!checkAuth()) {
      server.requestAuthentication(BASIC_AUTH, "ESP32-CAM", "Authentication required");
      return;
    }
    handleStatus();
  });
  
  server.on("/control", HTTP_POST, []() {
    if (!checkAuth()) {
      server.requestAuthentication(BASIC_AUTH, "ESP32-CAM", "Authentication required");
      return;
    }
    handleControl();
  });
  
  server.on("/rec", HTTP_POST, []() {
    if (!checkAuth()) {
      server.requestAuthentication(BASIC_AUTH, "ESP32-CAM", "Authentication required");
      return;
    }
    handleRec();
  });
  
  server.on("/format", HTTP_POST, []() {
    if (!checkAuth()) {
      server.requestAuthentication(BASIC_AUTH, "ESP32-CAM", "Authentication required");
      return;
    }
    handleFormat();
  });
  
  server.on("/delete", HTTP_POST, []() {
    if (!checkAuth()) {
      server.requestAuthentication(BASIC_AUTH, "ESP32-CAM", "Authentication required");
      return;
    }
    handleDelete();
  });
  
  server.on("/download", HTTP_GET, []() {
    if (!checkAuth()) {
      server.requestAuthentication(BASIC_AUTH, "ESP32-CAM", "Authentication required");
      return;
    }
    handleDownload();
  });
  
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

// ==================== ПРОВЕРКА АВТОРИЗАЦИИ ====================
bool checkAuth() {
  if (server.hasHeader("Authorization")) {
    String authHeader = server.header("Authorization");
    if (authHeader.startsWith("Basic ")) {
      String authPlaintext = authHeader.substring(6);
      if (authPlaintext == "YWRtaW46YWRtaW4=") {
        return true;
      }
    }
  }
  return false;
}

// ==================== ОБРАБОТЧИК ГЛАВНОЙ СТРАНИЦЫ ====================
void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

// ==================== ОБРАБОТЧИК НЕ НАЙДЕНО ====================
void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

// ==================== MJPEG ВИДЕОПОТОК ====================
void handleStream() {
  if (!checkAuth()) {
    server.requestAuthentication(BASIC_AUTH, "ESP32-CAM", "Authentication required");
    return;
  }
  
  WiFiClient client = server.client();
  client.setTimeout(0);
  
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n";
  response += "Connection: close\r\n";
  response += "Cache-Control: no-cache\r\n";
  response += "\r\n";
  
  client.print(response);
  
  while (client.connected()) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      delay(100);
      continue;
    }
    
    String frameHeader = "--frame\r\n";
    frameHeader += "Content-Type: image/jpeg\r\n";
    frameHeader += "Content-Length: " + String(fb->len) + "\r\n";
    frameHeader += "\r\n";
    
    client.print(frameHeader);
    
    size_t bytesSent = client.write(fb->buf, fb->len);
    if (bytesSent != fb->len) {
      Serial.printf("Failed to send frame (sent %d of %d)\n", bytesSent, fb->len);
      esp_camera_fb_return(fb);
      break;
    }
    
    client.print("\r\n");
    
    esp_camera_fb_return(fb);
    
    delay(30);
  }
  
  Serial.println("Stream client disconnected");
}

// ==================== ОДИНОЧНЫЙ СНИМОК ====================
void handleCapture() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }
  
  server.sendHeader("Content-Disposition", "attachment; filename=capture.jpg");
  server.send_P(200, "image/jpeg", (const char*)fb->buf, fb->len);
  
  esp_camera_fb_return(fb);
}

// ==================== СТАТУС СИСТЕМЫ (JSON) ====================
void handleStatus() {
  uint64_t sdTotal = SD_MMC.totalBytes();
  uint64_t sdUsed = SD_MMC.usedBytes();
  
  char json[4096];
  int offset = 0;
  
  offset += snprintf(json + offset, sizeof(json) - offset, "{");
  offset += snprintf(json + offset, sizeof(json) - offset, 
    "\"recording\":%s,", isRecording ? "true" : "false");
  offset += snprintf(json + offset, sizeof(json) - offset, 
    "\"mode\":\"%s\",", isMotionMode ? "motion" : "manual");
  offset += snprintf(json + offset, sizeof(json) - offset, 
    "\"framesize\":%d,", currentFrameSize);
  offset += snprintf(json + offset, sizeof(json) - offset, 
    "\"sensitivity\":%d,", motionSensitivity);
  offset += snprintf(json + offset, sizeof(json) - offset, 
    "\"sd_total\":%llu,", sdTotal);
  offset += snprintf(json + offset, sizeof(json) - offset, 
    "\"sd_used\":%llu,", sdUsed);
  offset += snprintf(json + offset, sizeof(json) - offset, 
    "\"auto_delete\":%s,", isAutoDeleteEnabled ? "true" : "false");
  offset += snprintf(json + offset, sizeof(json) - offset, 
    "\"auto_threshold\":%d,", autoDeleteThreshold);
  
  offset += snprintf(json + offset, sizeof(json) - offset, "\"files\":[");
  
  File root = SD_MMC.open("/img");
  if (root && root.isDirectory()) {
    bool first = true;
    File file = root.openNextFile();
    while (file && offset < sizeof(json) - 200) {
      if (!file.isDirectory()) {
        if (!first) {
          offset += snprintf(json + offset, sizeof(json) - offset, ",");
        }
        offset += snprintf(json + offset, sizeof(json) - offset, 
          "{\"name\":\"%s\",\"size\":%u}", 
          file.name(), file.size());
        first = false;
      }
      file.close();
      file = root.openNextFile();
    }
  }
  root.close();
  
  offset += snprintf(json + offset, sizeof(json) - offset, "]}");
  
  server.send(200, "application/json", json);
}

// ==================== УПРАВЛЕНИЕ НАСТРОЙКАМИ ====================
void handleControl() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "No JSON body");
    return;
  }
  
  String body = server.arg("plain");
  
  int fsIdx = body.indexOf("\"framesize\":");
  if (fsIdx != -1) {
    int start = fsIdx + 12;
    int end = body.indexOf(",", start);
    if (end == -1) end = body.indexOf("}", start);
    String val = body.substring(start, end);
    currentFrameSize = val.toInt();
    
    sensor_t* s = esp_camera_sensor_get();
    if (s) {
      s->set_framesize(s, (framesize_t)currentFrameSize);
      Serial.printf("Frame size set to %d\n", currentFrameSize);
    }
  }
  
  int modeIdx = body.indexOf("\"mode\":\"");
  if (modeIdx != -1) {
    int start = modeIdx + 8;
    int end = body.indexOf("\"", start);
    String mode = body.substring(start, end);
    isMotionMode = (mode == "motion");
    Serial.printf("Mode set to %s\n", isMotionMode ? "motion" : "manual");
  }
  
  int sensIdx = body.indexOf("\"sensitivity\":");
  if (sensIdx != -1) {
    int start = sensIdx + 14;
    int end = body.indexOf(",", start);
    if (end == -1) end = body.indexOf("}", start);
    String val = body.substring(start, end);
    motionSensitivity = val.toInt();
    Serial.printf("Motion sensitivity set to %d\n", motionSensitivity);
  }
  
  int adIdx = body.indexOf("\"auto_delete\":");
  if (adIdx != -1) {
    int start = adIdx + 14;
    int end = body.indexOf(",", start);
    if (end == -1) end = body.indexOf("}", start);
    String val = body.substring(start, end);
    isAutoDeleteEnabled = (val == "true");
    Serial.printf("Auto-delete %s\n", isAutoDeleteEnabled ? "enabled" : "disabled");
  }
  
  int atIdx = body.indexOf("\"auto_threshold\":");
  if (atIdx != -1) {
    int start = atIdx + 17;
    int end = body.indexOf(",", start);
    if (end == -1) end = body.indexOf("}", start);
    String val = body.substring(start, end);
    autoDeleteThreshold = val.toInt();
    Serial.printf("Auto-delete threshold set to %d%%\n", autoDeleteThreshold);
  }
  
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

// ==================== СТАРТ/СТОП ЗАПИСИ ====================
void handleRec() {
  if (isMotionMode) {
    server.send(400, "text/plain", "Cannot manually record in motion mode");
    return;
  }
  
  isRecording = !isRecording;
  Serial.printf("Recording %s\n", isRecording ? "started" : "stopped");
  
  server.send(200, "application/json", 
    String("{\"recording\":") + (isRecording ? "true" : "false") + "}");
}

// ==================== ОЧИСТКА ВСЕХ ФАЙЛОВ НА SD ====================
void handleFormat() {
  Serial.println("[FORMAT] Clearing all files from /img...");
  
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    server.send(500, "text/plain", "SD card not detected");
    return;
  }
  
  File root = SD_MMC.open("/img");
  if (!root) {
    server.send(500, "text/plain", "Cannot open /img directory");
    return;
  }
  
  if (!root.isDirectory()) {
    root.close();
    server.send(500, "text/plain", "/img is not a directory");
    return;
  }
  
  int deletedCount = 0;
  int failedCount = 0;
  uint64_t freedBytes = 0;
  
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String filename = String(file.name());
      size_t fileSize = file.size();
      file.close();
      
      String filepath = "/img/" + filename;
      if (SD_MMC.remove(filepath.c_str())) {
        deletedCount++;
        freedBytes += fileSize;
        Serial.printf("[FORMAT] Deleted: %s (%u bytes)\n", filepath.c_str(), fileSize);
      } else {
        failedCount++;
        Serial.printf("[FORMAT] Failed to delete: %s\n", filepath.c_str());
      }
    } else {
      file.close();
    }
    file = root.openNextFile();
  }
  root.close();
  
  if (isRecording) {
    isRecording = false;
    Serial.println("[FORMAT] Recording stopped due to cleanup");
  }
  
  Serial.printf("[FORMAT] Done. Deleted: %d, Failed: %d, Freed: %llu bytes\n", 
                deletedCount, failedCount, freedBytes);
  
  char response[256];
  snprintf(response, sizeof(response), 
    "{\"status\":\"cleared\",\"deleted\":%d,\"failed\":%d,\"freed\":%llu}", 
    deletedCount, failedCount, freedBytes);
  
  server.send(200, "application/json", response);
}

// ==================== УДАЛЕНИЕ ФАЙЛА ====================
void handleDelete() {
  if (!server.hasArg("file")) {
    server.send(400, "text/plain", "Missing file parameter");
    return;
  }
  
  String filename = server.arg("file");
  String filepath = "/img/" + filename;
  
  Serial.printf("Deleting file: %s\n", filepath.c_str());
  
  if (SD_MMC.remove(filepath.c_str())) {
    Serial.println("File deleted successfully");
    server.send(200, "application/json", "{\"status\":\"deleted\"}");
  } else {
    Serial.println("Failed to delete file");
    server.send(500, "text/plain", "Delete failed");
  }
}

// ==================== СКАЧИВАНИЕ ФАЙЛА ====================
void handleDownload() {
  if (!server.hasArg("file")) {
    server.send(400, "text/plain", "Missing file parameter");
    return;
  }
  
  String filename = server.arg("file");
  String filepath = "/img/" + filename;
  
  File file = SD_MMC.open(filepath.c_str());
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  
  if (file.isDirectory()) {
    file.close();
    server.send(400, "text/plain", "Cannot download directory");
    return;
  }
  
  server.sendHeader("Content-Disposition", "attachment; filename=" + filename);
  server.streamFile(file, "image/jpeg");
  file.close();
  
  Serial.printf("File downloaded: %s\n", filename.c_str());
}

// ==================== ПОЛУЧЕНИЕ РАЗМЕРА SD ====================
uint64_t getSDTotalBytes() {
  return SD_MMC.totalBytes();
}

uint64_t getSDUsedBytes() {
  return SD_MMC.usedBytes();
}

// ==================== ГЕНЕРАЦИЯ ИМЕНИ ФАЙЛА ====================
String generateFilename() {
  frameCounter++;
  char filename[32];
  snprintf(filename, sizeof(filename), "/img/img_%05lu.jpg", frameCounter);
  return String(filename);
}

// ==================== ЗАПИСЬ КАДРА НА SD ====================
void recordFrame() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed during recording");
    return;
  }
  
  String filename = generateFilename();
  
  File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
  if (!file) {
    Serial.printf("Failed to open file for writing: %s\n", filename.c_str());
    esp_camera_fb_return(fb);
    return;
  }
  
  size_t bytesWritten = file.write(fb->buf, fb->len);
  file.close();
  
  if (bytesWritten > 0) {
    Serial.printf("Recorded: %s (%u bytes)\n", filename.c_str(), bytesWritten);
  } else {
    Serial.printf("Failed to write: %s\n", filename.c_str());
  }
  
  esp_camera_fb_return(fb);
}

// ==================== АВТОУДАЛЕНИЕ ПРИ ЗАПОЛНЕНИИ ====================
void autoDeleteWhenFull() {
  uint64_t totalBytes = getSDTotalBytes();
  uint64_t usedBytes = getSDUsedBytes();
  
  if (totalBytes == 0) return;
  
  float usedPercent = (float)usedBytes / totalBytes * 100.0;
  
  if (usedPercent >= autoDeleteThreshold) {
    Serial.printf("[AUTO-DELETE] SD card %0.1f%% full (threshold: %d%%)\n", 
                  usedPercent, autoDeleteThreshold);
    
    if (deleteOldestFile()) {
      Serial.println("[AUTO-DELETE] Oldest file deleted");
    } else {
      Serial.println("[AUTO-DELETE] No files to delete");
    }
  }
}

// ==================== УДАЛЕНИЕ САМОГО СТАРОГО ФАЙЛА ====================
bool deleteOldestFile() {
  File root = SD_MMC.open("/img");
  if (!root || !root.isDirectory()) {
    return false;
  }
  
  String oldestFile = "";
  uint32_t oldestNumber = UINT32_MAX;
  
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String name = String(file.name());
      
      int underscorePos = name.indexOf('_');
      int dotPos = name.indexOf('.');
      
      if (underscorePos != -1 && dotPos != -1 && dotPos > underscorePos) {
        String numStr = name.substring(underscorePos + 1, dotPos);
        uint32_t num = numStr.toInt();
        
        if (num < oldestNumber) {
          oldestNumber = num;
          oldestFile = name;
        }
      }
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();
  
  if (oldestFile.length() > 0) {
    String filepath = "/img/" + oldestFile;
    Serial.printf("Deleting oldest file: %s\n", filepath.c_str());
    return SD_MMC.remove(filepath.c_str());
  }
  
  return false;
}

// ==================== ДЕТЕКЦИЯ ДВИЖЕНИЯ ====================
bool detectMotionAccurate(camera_fb_t* fb) {
  static uint8_t* prevGray = nullptr;
  static size_t prevGraySize = 0;
  
  sensor_t* s = esp_camera_sensor_get();
  if (!s) return false;
  
  framesize_t originalFramesize = (framesize_t)currentFrameSize;
  
  s->set_framesize(s, FRAMESIZE_QVGA);
  s->set_pixformat(s, PIXFORMAT_GRAYSCALE);
  
  camera_fb_t* grayFb = esp_camera_fb_get();
  if (!grayFb) {
    s->set_pixformat(s, PIXFORMAT_JPEG);
    s->set_framesize(s, originalFramesize);
    return false;
  }
  
  bool motionDetected = false;
  
  if (prevGray != nullptr && prevGraySize == grayFb->len) {
    uint32_t diffSum = 0;
    size_t pixelCount = grayFb->len;
    
    for (size_t i = 0; i < pixelCount; i++) {
      int diff = abs((int)grayFb->buf[i] - (int)prevGray[i]);
      diffSum += diff;
    }
    
    float avgDiff = (float)diffSum / pixelCount;
    float threshold = (float)motionSensitivity / 1000.0;
    
    if (avgDiff > threshold) {
      motionDetected = true;
      Serial.printf("[MOTION] Detected! Avg diff: %.2f (threshold: %.2f)\n", 
                    avgDiff, threshold);
    }
  }
  
  if (prevGray != nullptr) {
    free(prevGray);
  }
  prevGraySize = grayFb->len;
  prevGray = (uint8_t*)malloc(prevGraySize);
  if (prevGray != nullptr) {
    memcpy(prevGray, grayFb->buf, prevGraySize);
  }
  
  esp_camera_fb_return(grayFb);
  
  s->set_pixformat(s, PIXFORMAT_JPEG);
  s->set_framesize(s, originalFramesize);
  
  return motionDetected;
}

// ==================== ОБРАБОТКА MOTION DETECTION ====================
void processMotionDetection() {
  if (!isMotionMode) return;
  
  static unsigned long lastMotionCheck = 0;
  if (millis() - lastMotionCheck < 500) return;
  lastMotionCheck = millis();
  
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) return;
  
  bool motion = detectMotionAccurate(fb);
  
  esp_camera_fb_return(fb);
  
  if (motion) {
    motionDetected = true;
    lastMotionTime = millis();
    
    if (!isRecording) {
      isRecording = true;
      Serial.println("[MOTION] Recording started");
    }
  }
}

// ==================== ЗАПИСЬ В РЕЖИМЕ MOTION ====================
void processMotionRecording() {
  if (!isMotionMode || !isRecording) return;
  
  if (millis() - lastMotionTime < 3000) {
    recordFrame();
    delay(100);
  } else {
    isRecording = false;
    motionDetected = false;
    Serial.println("[MOTION] Recording stopped (timeout)");
  }
}

// ==================== ЗАПИСЬ В РУЧНОМ РЕЖИМЕ ====================
void processManualRecording() {
  if (isMotionMode || !isRecording) return;
  
  recordFrame();
  delay(100);
}

// ==================== ДИАГНОСТИКА ====================
void printSystemInfo() {
  Serial.println("\n=== SYSTEM INFORMATION ===");
  Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("PSRAM size: %u bytes\n", ESP.getPsramSize());
  Serial.printf("PSRAM free: %u bytes\n", ESP.getFreePsram());
  Serial.printf("CPU freq: %u MHz\n", ESP.getCpuFreqMHz());
  
  uint64_t sdTotal = getSDTotalBytes();
  uint64_t sdUsed = getSDUsedBytes();
  Serial.printf("SD Total: %llu MB\n", sdTotal / (1024 * 1024));
  Serial.printf("SD Used: %llu MB\n", sdUsed / (1024 * 1024));
  Serial.printf("SD Free: %llu MB\n", (sdTotal - sdUsed) / (1024 * 1024));
  Serial.println("==========================\n");
}

/*
═══════════════════════════════════════════════════════════════
                    © sa - 2026
═══════════════════════════════════════════════════════════════
*/
