#ifndef audio_h
#define audio_h

#include <Arduino.h>
#include <ESPVS1003.h>
#include <SPI.h>

/*----------------------------------
           PIN MAPPINGS
----------------------------------*/
//VS1003
extern uint8_t csVS;
extern uint8_t dcsVS;
extern uint8_t dreqVS;
extern uint8_t rsetVS;
//extern uint8_t sckVS;
//extern uint8_t misoVS;
//extern uint8_t mosiVS;

/*----------------------------------
          VS1003 SETTINGS
----------------------------------*/
extern uint8_t buff[320];
extern VS1003 player;

#endif