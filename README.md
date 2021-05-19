# What is it?

This is a web radio player made for ESP32 board with VS1003 audio module. It can be controlled with buttons and user panel hosted on ESP32 web server.

# Hardware

1. ESP32-DevKitC
2. VS1003
3. (Optionally) 6 monostable buttons
4. Connection wires

It can be powered by USB connected to ESP32 or from other 5V source.

# Software

1. Arduino IDE with correct board set
2. Arduino ESP32 Filesystem Uploader

# Libraries

1. ESPAsyncWebServer 
2. ArduinoJson 
3. ESPVS1003 
4. HTTPClient 
5. ArduinoNvs 
6. Vector 

# PIN Connections

| VS1003 PIN | ESP32 PIN |
|---|---|
| 5V  | (if no external 5V) VIN  |
| GND  | (if no external GND) GND   |
| MISO  | D19  |
| MOSI  | D23  |
| SCK  | D18  |
| DREQ  | D35  |
| RSET  | EN  |
| XS  | D32  |
| DCS  | D33  |

| Button | ESP32 PIN |
|---|---|
| Next station | D15 |
| Previous station | D13 |
| Volume Up | D4 |
| Volume Down | D2 |
| Add or remove favourite | D34 |
| Switch favourites mode | D22 |
