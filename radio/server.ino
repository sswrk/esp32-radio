#include "src/radio.h"

AsyncWebServer server(80);
AsyncWebSocket webSocket("/update");

void notifyClients(){
  String output;
  DynamicJsonDocument root(1024 * 100);
  RadioStationInfo currentStation = availableStations.at(radioStation);
  root["station"]["id"] = currentStation.id;
  root["station"]["name"] = currentStation.name;
  root["station"]["host"] = currentStation.host;
  root["station"]["path"] = currentStation.path;
  root["station"]["port"] = currentStation.port;
  root["station"]["isFavourite"] = currentStation.isFavourite;
  root["playing"] = !paused;
  serializeJson(root, output);
  webSocket.textAll(output);
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
 void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  webSocket.onEvent(onEvent);
  server.addHandler(&webSocket);
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "application/json", "{\"message\":\"Not found\"}");
}

void createServerHandlers() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/styles.scss", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/styles.scss", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });
  server.on("/img/next.svg", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/img/next.svg", "image/svg+xml");
  });
  server.on("/img/play.svg", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/img/play.svg", "image/svg+xml");
  });
  server.on("/img/previous.svg", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/img/previous.svg", "image/svg+xml");
  });
  server.on("/check-station", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument root(1024 * 100);
    RadioStationInfo currentStation = availableStations.at(radioStation);
    root["station"]["id"] = currentStation.id;
    root["station"]["name"] = currentStation.name;
    root["station"]["host"] = currentStation.host;
    root["station"]["path"] = currentStation.path;
    root["station"]["port"] = currentStation.port;
    root["station"]["isFavourite"] = currentStation.isFavourite;
    root["playing"] = !paused;
    serializeJson(root, *response);
    request->send(response);
  });
  server.on("/stations", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument root(1024 * 100);
    for (int i = 0; i < availableStations.size(); i++) {
      RadioStationInfo currentStation = availableStations.at(i);
      root["stations"][i]["id"] = currentStation.id;
      root["stations"][i]["name"] = currentStation.name;
      root["stations"][i]["host"] = currentStation.host;
      root["stations"][i]["path"] = currentStation.path;
      root["stations"][i]["port"] = currentStation.port;
      root["stations"][i]["isFavourite"] = currentStation.isFavourite;
    }
    serializeJson(root, *response);
    request->send(response);
  });
  server.on("/stations", HTTP_POST, [](AsyncWebServerRequest * request) {
  }, NULL, [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, (const char*)data);
    bool success = false;
    if (!error) {
      const char* host = doc["host"];
      const char* path = doc["path"];
      const char* name = doc["name"];
      int port = doc["port"];
      success = addRadioStation(host, path, name, port);
    }
    if (success) {
      request->send(200, "text/plain", "Radio station added");
    } else {
      request->send(400, "text/plain", "Wrong parameters");
    }
  });
  server.on("/stations", HTTP_DELETE, [](AsyncWebServerRequest * request) {
  }, NULL, [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, (const char*)data);
    bool success = false;
    if (!error) {
      int id = doc["id"];
      success = deleteRadioStation(id);
    }
    if (success) {
      request->send(200, "text/plain", "Radio station deleted");
    } else {
      request->send(400, "text/plain", "Wrong parameters");
    }
  });
  server.on("/favourites", HTTP_POST, [](AsyncWebServerRequest * request) {
  }, NULL, [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, (const char*)data);
    bool success = false;
    if (!error) {
      int id = doc["id"];
      success = addToFavourites(id);
    }
    if (success) {
      request->send(200, "text/plain", "Radio station added to favourited");
    } else {
      request->send(400, "text/plain", "Wrong parameters");
    }
  });
  server.on("/favourites", HTTP_DELETE, [](AsyncWebServerRequest * request) {
  }, NULL, [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, (const char*)data);
    bool success = false;
    if (!error) {
      int id = doc["id"];
      success = removeFromFavourites(id);
    }
    if (success) {
      request->send(200, "text/plain", "Radio station removed from favourites");
    } else {
      request->send(400, "text/plain", "Wrong parameters");
    }
  });
  server.on("/set-station", HTTP_POST, [](AsyncWebServerRequest * request) {
  }, NULL, [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, (const char*)data);
    bool success = false;
    if (!error) {
      int id = doc["id"];
      success = setStation(id);
    }
    if (success) {
      request->send(200, "text/plain", "Radio station set");
    } else {
      request->send(400, "text/plain", "Wrong parameters");
    }
  });
  server.on("/toggle-play", HTTP_GET, [](AsyncWebServerRequest * request) {
    toggleVolume();
    request->send(200, "text/plain", "Toggled play");
  });
  server.on("/louder", HTTP_GET, [](AsyncWebServerRequest * request) {
    volumeUp();
    request->send(200, "text/plain", "Volume +10");
  });
  server.on("/more-quiet", HTTP_GET, [](AsyncWebServerRequest * request) {
    volumeDown();
    request->send(200, "text/plain", "Volume -10");
  });
  server.onNotFound(notFound);
}

void initServer() {
  createServerHandlers();
  server.begin();
  initWebSocket();
  Serial.println("Server started!");
}
