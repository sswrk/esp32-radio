#ifndef eeprom_h
#define eeprom_h

#define EEPROM_SIZE 3

#include <EEPROM.h>
#include <Arduino.h>

int getLastStationFromEEPROM();
void writeLastStationToEEPROM(int id);
uint8_t getVolumeFromEEPROM();
void writeVolumeToEEPROM(uint8_t vol);

#endif