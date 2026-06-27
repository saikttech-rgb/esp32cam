# 📹 ESP32-CAM Hi-Tech WiFi Camera

<div align="center">

![ESP32-CAM](https://img.shields.io/badge/ESP32-CAM-orange?style=for-the-badge&logo=espressif)
![Arduino IDE](https://img.shields.io/badge/Arduino-IDE-00878F?style=for-the-badge&logo=arduino)
![WiFi](https://img.shields.io/badge/WiFi-AP%20Mode-blue?style=for-the-badge&logo=wifi)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)
![Version](https://img.shields.io/badge/Version-1.0.0-purple?style=for-the-badge)

**Профессиональная система видеонаблюдения на базе ESP32-CAM с Hi-Tech веб-интерфейсом**

[Возможности](#-возможности) • [Установка](#-установка) • [Использование](#-использование) • [API](#-api-endpoints) • [FAQ](#-faq) 

</div>

---

## 📖 Описание

**ESP32-CAM Hi-Tech Camera** — это полнофункциональная система видеонаблюдения на микроконтроллере ESP32-CAM (AI-Thinker) с современным киберпанк-интерфейсом. Устройство работает как автономная WiFi точка доступа с веб-интерфейсом для управления камерой, записи видео на SD-карту и детекции движения.

### 🎯 Ключевые особенности

- **Автономная работа** — создаёт собственную WiFi сеть, не требует роутера
- **Hi-Tech дизайн** — неоновый киберпанк интерфейс с cyan/magenta акцентами
- **MJPEG видеопоток** — реальное время с настраиваемым качеством (QVGA → UXGA)
- **Запись на SD-карту** — ручной режим или автоматическая запись при движении
- **Motion Detection** — программная детекция движения через сравнение кадров
- **Умное управление памятью** — автоматическое удаление старых файлов при заполнении (FIFO)
- **Безопасность** — HTTP Basic Auth защита (admin/admin)
- **Адаптивный дизайн** — работает на ПК, планшетах и смартфонах

---

## ✨ Возможности

### 🎥 Видеонаблюдение
- ✅ MJPEG видеопоток в реальном времени (~15-30 FPS)
- ✅ Сворачиваемый видеоплеер (экономия ресурсов при сворачивании)
- ✅ 9 уровней качества: от QVGA (320×240) до UXGA (1600×1200)
- ✅ Одиночный снимок с возможностью скачивания

### 💾 Запись на SD-карту
- ✅ **Ручной режим** — запись по нажатию кнопки REC
- ✅ **Режим детекции движения** — автоматическая запись при обнаружении движения
- ✅ Настраиваемая чувствительность motion detection
- ✅ Инкрементальные имена файлов (img_00001.jpg, img_00002.jpg, ...)

### 🗂️ Управление файлами
- ✅ Просмотр списка файлов с размерами
- ✅ Скачивание отдельных файлов
- ✅ Удаление файлов по одному
- ✅ Массовая очистка всех файлов
- ✅ **Автоматическое удаление** при заполнении SD-карты (FIFO)
- ✅ Настраиваемый порог автоудаления (70-99%)

### 🎨 Интерфейс
- ✅ Hi-Tech / Cyberpunk стиль
- ✅ Неоновые акценты (cyan #00ffff, magenta #ff00ff)
- ✅ Glow-эффекты и анимации
- ✅ Адаптивная вёрстка (CSS Grid/Flexbox)
- ✅ AJAX обновление статуса каждые 3 секунды
- ✅ Визуальный progress-bar заполнения SD-карты

### 🔒 Безопасность
- ✅ HTTP Basic Authentication использует base64 кодирование (НЕ шифрование)
- ✅ Логин/пароль: `admin` / `admin`. WiFi пароль передаётся в открытом виде при первом подключении
- ✅ Защита всех endpoints

### 🔒 Для продакшена рекомендуется:
- ✅ Использовать HTTPS  
- ✅ Добавить proper authentication (OAuth2, JWT)  
- ✅ Ограничить доступ по MAC-адресу  
- ✅ Изменить дефолтные пароли  

---

## 🔧 Требования

### Аппаратные требования

| Компонент | Минимум | Рекомендуется |
|-----------|---------|---------------|
| **Плата** | ESP32-CAM (AI-Thinker) | ESP32-CAM с антенной |
| **SD-карта** | MicroSD 4GB (FAT32) | MicroSD 16-32GB Class 10 |
| **Питание** | 5V 1A | 5V 2A блок питания |
| **USB-кабель** | Micro-USB (с передачей данных) | Качественный кабель |
| **Компьютер** | USB порт | USB 3.0 порт |

### Программные требования

- **Arduino IDE** 2.x или новее
- **ESP32 Board Package** 2.x или 3.x
- **Драйвер USB-UART** (CH340 / CP2102 / FTDI)
- **Веб-браузер** с поддержкой JavaScript (Chrome, Firefox, Edge, Safari)

---

## 🚀 Установка

### Шаг 1: Установка Arduino IDE

1. Скачайте Arduino IDE с [официального сайта](https://www.arduino.cc/en/software)
2. Установите и запустите

### Шаг 2: Добавление поддержки ESP32

1. Откройте **File → Preferences**
2. В поле **"Additional Board Manager URLs"** добавьте: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
3. Нажмите **OK**
4. Откройте **Tools → Board → Boards Manager**
5. Найдите **"esp32"** и установите последнюю версию
6. Перезапустите Arduino IDE

### Шаг 3: Установка драйверов

Определите чип USB-UART на вашей плате:

| Чип | Драйвер | Ссылка |
|-----|---------|--------|
| **CH340** | CH341SER | [Скачать](https://www.wch-ic.com/downloads/CH341SER_EXE.html) |
| **CP2102** | CP210x | [Скачать](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) |
| **FTDI** | CDM21228 | [Скачать](https://ftdichip.com/drivers/d2xx-drivers/) |

### Шаг 4: Загрузка прошивки

1. Скачайте или клонируйте репозиторий:
```bash
git clone https://github.com/yourusername/esp32-cam-hitech-camera.git
```

---

## 📡 Использование

### Подключение к камере

1. **Подключитесь к WiFi сети:**
   - SSID: `ESP32-CAM`
   - Пароль: `12345678`

2. **Откройте веб-браузер:**

```html
http://192.168.4.1
``` 

3. **Авторизуйтесь:**
- Логин: `admin`
- Пароль: `admin`

---

### Основной интерфейс

#### Видеопоток
- Разверните блок **"ТРАНСЛЯЦИЯ"** для просмотра видео
- При сворачивании поток останавливается (экономия ресурсов)

#### Настройки камеры
- **Качество записи:** выберите разрешение из выпадающего списка
- **Чувствительность движения:** ползунок 5000-50000
- **Режим записи:**
  - **ВРУЧНУЮ** — запись по кнопке REC
  - **ПРИ ДВИЖЕНИИ** — автоматическая запись при детекции

#### Кнопка REC
- В ручном режиме: нажмите для старта/остановки записи
- В режиме движения: индикатор статуса (IDLE / MOTION REC)

---

### Панель утилит (справа)

#### Память SD-карты
- Progress-bar показывает заполнение
- Текст: "Занято X MB / Всего Y MB"

#### Автоудаление
- ✅ Включите чекбокс **"АВТОУДАЛЕНИЕ ПРИ ЗАПОЛНЕНИИ"**
- Настройте порог (70-99%) — при достижении порога старые файлы удаляются автоматически

#### Файлы
- Список всех файлов с размерами
- Кнопка **↓** — скачать файл
- Кнопка **×** — удалить файл

#### Очистка
- Кнопка **"ОЧИСТИТЬ ВСЕ ФАЙЛЫ"** — удаляет все записи из папки `/img`

---


## 🔌 API Endpoints

Все endpoints защищены HTTP Basic Auth (логин: `admin`, пароль: `admin`).

### Список endpoints

| Метод | URL | Описание | Параметры |
|-------|-----|----------|-----------|
| `GET` | `/` | Главная страница (HTML) | — |
| `GET` | `/stream` | MJPEG видеопоток | — |
| `GET` | `/capture` | Одиночный снимок (JPEG) | — |
| `GET` | `/status` | JSON статус системы | — |
| `POST` | `/control` | Изменение настроек | JSON body |
| `POST` | `/rec` | Старт/стоп записи (toggle) | — |
| `POST` | `/format` | Очистка всех файлов | — |
| `POST` | `/delete` | Удаление файла | `?file=filename.jpg` |
| `GET` | `/download` | Скачивание файла | `?file=filename.jpg` |

---

### Примеры запросов

#### 📊 Получить статус системы

```bash
curl -u admin:admin http://192.168.4.1/status
``` 
Ответ:
{
  "recording": false,
  "mode": "manual",
  "framesize": 7,
  "sensitivity": 15000,
  "sd_total": 31914983424,
  "sd_used": 15728640,
  "auto_delete": true,
  "auto_threshold": 95,
  "files": [
    {"name": "img_00001.jpg", "size": 524288},
    {"name": "img_00002.jpg", "size": 512000}
  ]
}

#### 🎨 Изменить качество видео
```bash
curl -u admin:admin -X POST http://192.168.4.1/control \
  -H "Content-Type: application/json" \
  -d '{"framesize": 9}'
```
Значения framesize:  

11 — QVGA (320×240)  
10 — CIF (400×296)  
9 — HVGA (480×320)  
7 — VGA (640×480)  
6 — SVGA (800×600)  
5 — XGA (1024×768)  
4 — HD (1280×720)  
3 — SXGA (1280×1024)  
2 — UXGA (1600×1200)  


#### 🏃 Включить режим детекции движения
```bash
curl -u admin:admin -X POST http://192.168.4.1/control \
  -H "Content-Type: application/json" \
  -d '{"mode": "motion"}'
```

Доступные режимы:
"manual" — ручная запись по кнопке REC  
"motion" — автоматическая запись при движении  

#### 🗑️ Удалить файл
```bash
curl -u admin:admin -X POST "http://192.168.4.1/delete?file=img_00001.jpg"
```

#### 💾 Скачать файл
```bash
curl -u admin:admin -O "http://192.168.4.1/download?file=img_00001.jpg"
```

#### 🎬 Старт/стоп записи (toggle)
```bash
curl -u admin:admin -X POST http://192.168.4.1/rec
```  
Ответ:
```bash
{"recording": true}
```

#### 🧹 Очистить все файлы
```bash
curl -u admin:admin -X POST http://192.168.4.1/format
```
Ответ:
```bash
{
  "status": "cleared",
  "deleted": 47,
  "failed": 0,
  "freed": 15728640
}
```

#### ⚙️ Изменить несколько настроек одновременно
```bash
curl -u admin:admin -X POST http://192.168.4.1/control \
  -H "Content-Type: application/json" \
  -d '{
    "framesize": 7,
    "mode": "motion",
    "sensitivity": 12000,
    "auto_delete": true,
    "auto_threshold": 90
  }'
```  

#### 💡 Советы по использованию API  
**Авторизация** — все запросы требуют заголовок Authorization: Basic YWRtaW46YWRtaW4= (base64 от admin:admin)  
**Content-Type** — для POST-запросов с JSON используйте application/json  
**Кодирование** — имена файлов в параметрах ?file= должны быть URL-кодированы  
**Видеопоток** — /stream возвращает multipart/x-mixed-replace с boundary frame  
**Статус** — /status возвращает актуальное состояние системы в реальном времени  

```cpp
1
``` 


---



## ❓ FAQ  

**Q: Можно ли подключить камеру к домашнему WiFi роутеру?**  
**A:** Текущая версия работает только в режиме Access Point. Для подключения к роутеру нужно изменить код:  

```cpp
// Вместо WiFi.softAP() используйте:
WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
}
Serial.println(WiFi.localIP());
```  

---

**Q: Как изменить пароль WiFi?**  
**A:** Измените константу AP_PASSWORD в начале файла .ino и загрузите прошивку заново.

---

**Q: Как добавить NTP для точного времени?**  
**A:** Добавьте в setup():

```cpp
#include <time.h>
configTime(0, 0, "pool.ntp.org", "time.nist.gov");
```
Но это требует подключения к интернету (STA режим).

---

**Q: Можно ли использовать несколько камер одновременно?**  
**A:** Нет, каждая камера создаёт свою WiFi сеть. Для масштабирования используйте роутер и подключите все камеры к одной сети (требуется модификация кода).

---

**Q: Какой максимальный размер SD-карты?**  
**A:** Теоретически до 2TB (SDXC), но рекомендуется 16-32GB для стабильной работы.

---

**Q: Как увеличить дальность WiFi?**  
**A:**
Используйте плату с внешней антенной
Добавьте усилитель сигнала
Уменьшите расстояние между камерой и устройством

---

**Q: Как перевернуть изображение камеры?**  
**A:** В функции initCamera() измените:

```cpp
s->set_hmirror(s, 1);  // Горизонтальное отражение
s->set_vflip(s, 1);    // Вертикальное отражение
``` 
Комбинации:  

**hmirror=0, vflip=0** — нормальное изображение  
**hmirror=1, vflip=1** — поворот на 180°  
**hmirror=1, vflip=0** — зеркальное отражение по горизонтали  
**hmirror=0, vflip=1** — зеркальное отражение по вертикали  

```cpp
1
```

--- 

## 📄 Лицензия
Этот проект распространяется под лицензией MIT License - см. файл LICENSE для подробностей.

MIT License

Copyright (c) 2026 sa

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


---

## 👤 Автор
dsaru   
GitHub: @saikttech-rgb  
Email: sa.ikttech@example.com   

---

## 🙏 Благодарности  
Espressif Systems — за ESP32  
Arduino Community — за Arduino IDE  
ESP32 Camera Driver — за драйвер камеры  
Всем контрибьюторам open-source проектов  

---

## ⚠️ Дисклеймер
Этот проект предоставляется "КАК ЕСТЬ" без каких-либо гарантий. Автор не несёт ответственности за любой ущерб, возникший в результате использования данного программного обеспечения.
Используйте responsibly и соблюдайте местные законы о видеонаблюдении и приватности.

<div align="center">

© dsaru 2026 | ESP32-CAM Hi-Tech Camera  
Сделано с ❤️ для сообщества IoT  
⭐ Если проект был полезен, поставьте звезду на GitHub! ⭐  
</div>





