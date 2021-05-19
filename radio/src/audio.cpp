#include "audio.h"

/*----------------------------------
           PIN MAPPINGS
----------------------------------*/
//VS1003
extern uint8_t csVS = 32; //D32
extern uint8_t dcsVS = 33; //D33
extern uint8_t dreqVS = 35; //D35
extern uint8_t rsetVS = 3; //EN
//extern uint8_t sckVS = 18; //D18
//extern uint8_t misoVS = 19; //D19
//extern uint8_t mosiVS = 23; //D23

/*----------------------------------
          VS1003 SETTINGS
----------------------------------*/
uint8_t buff[32];
extern VS1003 player(csVS, dcsVS, dreqVS, rsetVS);