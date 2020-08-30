// #ifdef DEBUG_SERIAL
// #define DEBUG_OUT Serial
// #endif
// #define PRINTSTREAM_FALLBACK
// #include "Debug.hpp"

#include <Arduino.h>

#include <Fsm.h>

//--------------------------------------------

#include <FastLED.h>

#define NUM_LEDS 25
#define LED_PIN 27

CRGB leds[NUM_LEDS];

void setLeds(CRGB colour)
{
  for (int num = 0; num < NUM_LEDS; num++)
  {
    leds[num] = colour;
  }
  FastLED.show();
}

//--------------------------------------------

#include <Fsm.h>

enum CommsStateEvent
{
  EV_COMMS_NO_EVENT,
  EV_COMMS_PKT_RXD,
  EV_COMMS_BOARD_TIMEDOUT,
  EV_COMMS_BD_FIRST_PACKET,
};

Fsm *commsFsm;

void addCommsStateTransitions()
{
  // EV_COMMS_PKT_RXD
  // commsFsm->add_transition(&stateCommsSearching, &stateCommsConnected, EV_COMMS_PKT_RXD, NULL);
  // commsFsm->add_transition(&stateCommsDisconnected, &stateCommsConnected, EV_COMMS_PKT_RXD, NULL);

  // // EV_COMMS_BOARD_TIMEDOUT
  // commsFsm->add_transition(&stateCommsConnected, &stateCommsDisconnected, EV_COMMS_BOARD_TIMEDOUT, NULL);

  // // EV_COMMS_BD_RESET
  // commsFsm->add_transition(&stateCommsConnected, &stateCommsConnected, EV_COMMS_BD_FIRST_PACKET, NULL);
  // commsFsm->add_transition(&stateCommsDisconnected, &stateCommsDisconnected, EV_COMMS_BD_FIRST_PACKET, NULL);
}

void setup()
{
  Serial.begin(115200);
  Serial.printf("Ready\n");

  // commsFsm = new Fsm(&stateCommsSearching);
}

void loop()
{
}