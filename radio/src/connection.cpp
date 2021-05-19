#include "connection.h"

/*----------------------------------
           WIFI SETTINGS
----------------------------------*/
char ssid[] = "<SSID>";
char pass[] = "<PASS>";

extern int status = WL_IDLE_STATUS;
WiFiClient  client;

/*----------------------------------
    RADIO CONNECTION FUNCTIONS
----------------------------------*/
void connectToStation(int stationId ) {
  Serial.println("Connecting to radio station...");
  char buf[100];
  availableStations.at(stationId).host.toCharArray(buf, 100);
  if (client.connect(buf, availableStations.at(stationId).port)) {
    Serial.println("Connected to radio station!");
  }
  client.print(String("GET ") + availableStations.at(stationId).path + " HTTP/1.1\r\n" +
               "Host: " + availableStations.at(stationId).host + "\r\n" +
               "Connection: close\r\n\r\n");
  drawRadioStationName(stationId);
  connectedToStation = true;
  player.startSong();
}

void connectToWIFI() {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}