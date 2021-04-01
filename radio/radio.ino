#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>

#define EEPROM_SIZE 2

long interval = 250;
long previousButtonMillis = 0;

int radioStation = 0;
int previousRadioStation = -1;
const int previousButton = 13;
const int nextButton = 15;

char ssid[] = "<SSID>";
char pass[] = "<PASSWORD>";

char *host[4] = {"centova.radios.pt", "wamu-1.streamguys.com", "realfm.live24.gr", "live.slovakradio.sk"};
char *path[4] = {"/stream?type=http&nocache=3342", "/", "/realfm", "/Devin_256.mp3"};
int   port[4] = {9496, 80, 80, 8000};

int status = WL_IDLE_STATUS;
WiFiClient  client;
uint8_t mp3buff[32];

void IRAM_ATTR previousButtonInterrupt() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousButtonMillis > interval) {

    Serial.println("PREVIOUS");

    if (radioStation > 0)
      radioStation--;
    else
      radioStation = 3;

    previousButtonMillis = currentMillis;
  }
}

void IRAM_ATTR nextButtonInterrupt() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousButtonMillis > interval) {

    Serial.println("NEXT");

    if (radioStation < 3)
      radioStation++;
    else
      radioStation = 0;

    previousButtonMillis = currentMillis;
  }
}

void setup () {

  Serial.begin(9600);
  delay(500);

  EEPROM.begin(EEPROM_SIZE);

  pinMode(previousButton, INPUT_PULLUP);
  pinMode(nextButton, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(previousButton), previousButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(nextButton), nextButtonInterrupt, RISING);

  connectToWIFI();

  radioStation = getLastStationFromEEPROM();
  if (radioStation < 0 || radioStation >= 4) {
    radioStation = 0;
  }
  previousRadioStation = radioStation;
}

void loop() {

  if (radioStation != previousRadioStation) {
    connectToStation(radioStation);
    previousRadioStation = radioStation;
    writeLastStationToEEPROM(radioStation);
  }

  if (client.available() > 0) {
    uint8_t bytesread = client.read(mp3buff, 32);
//    for(int i=0; i<bytesread; i++){
//      Serial.println(mp3buff[i]);
//    }
//    Serial.println("");
//    TODO: send data to audio module input
  }
}

void connectToStation(int stationId ) {
  Serial.println("Connecting to radio station...");
  if (client.connect(host[stationId], port[stationId]) ) {
    Serial.println("Connected to radio station!");
  }
  client.print(String("GET ") + path[stationId] + " HTTP/1.1\r\n" +
               "Host: " + host[stationId] + "\r\n" +
               "Connection: close\r\n\r\n");
  drawRadioStationName(stationId);
}

void connectToWIFI() {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
}

void drawRadioStationName(int id) {
  Serial.println(host[id]);
}

int getLastStationFromEEPROM() {
  byte byte1 = EEPROM.read(0);
  byte byte2 = EEPROM.read(1);
  int id = (byte1 << 8) + byte2;
  Serial.println("Got station ID from EEPROM: " + String(id));
  return id;
}

void writeLastStationToEEPROM(int id) {
  byte byte1 = id >> 8;
  byte byte2 = id & 0xFF;
  EEPROM.write(0, byte1);
  EEPROM.write(1, byte2);
  EEPROM.commit();
  Serial.println("Wrote station ID to EEPROM: " + String(radioStation));
}
