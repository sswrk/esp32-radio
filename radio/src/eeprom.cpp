#include "eeprom.h"

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
  Serial.println("Wrote station ID to EEPROM: " + String(id));
}

uint8_t getVolumeFromEEPROM() {
  uint8_t vol = EEPROM.read(2);
  Serial.println("Got volume from EEPROM: " + String(vol));
  return vol;
}

void writeVolumeToEEPROM(uint8_t vol) {
  EEPROM.write(2, vol);
  EEPROM.commit();
  Serial.println("Wrote volume to EEPROM: " + String(vol));
}