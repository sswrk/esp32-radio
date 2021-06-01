# What is it?

This is a web radio player made for ESP32 board with VS1003 audio module. It can be controlled with buttons and user panel hosted on ESP32 web server.

# Features

1. Adding radio stations
2. Removing radio stations
3. Adding to favourites
4. Removing from favourites
5. Changing the stations (all or favourites)
6. Changing the volume
7. Listening to the radio (connecting headphones or speakers to VS1003)
8. Pausing/playing the current station
9. Switching between all and favourites mode

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

![Diagram](https://user-images.githubusercontent.com/58403334/120080244-39baf000-c0b8-11eb-92bc-a52461ce3d02.png)

# How to run

## Tools

Arduino IDE with ESP32 board set, [Arduino ESP32 filesystem uploader](https://github.com/me-no-dev/arduino-esp32fs-plugin), libraries which can be found in Library Manager or on GitHub:

1. ESPAsyncWebServer 
2. ArduinoJson 
3. ESPVS1003 
4. HTTPClient 
5. ArduinoNvs 
6. Vector 

## Setting up

Open connections.h file and enter your WiFi credentials.

## Launching

Load the code to your ESP32 using Arduino IDE and upload the server files to board's flash memory using the plugin. The server's IP number is printed to the serial port on startup.
