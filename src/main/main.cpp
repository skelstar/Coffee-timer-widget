// #ifdef DEBUG_SERIAL
// #define DEBUG_OUT Serial
// #endif
// #define PRINTSTREAM_FALLBACK
// #include "Debug.hpp"

#include <Arduino.h>
#include <elapsedMillis.h>

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
  EV_BUTTON_PRESSED,
  EV_NEXT,
};

Fsm *lightsFsm;

//------------------------------------------

elapsedMillis sinceNextState = 0;

State stateStart(
    [] {
      Serial.printf("State: stateStart\n");

      FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
      FastLED.setBrightness(30);

      setLeds(CRGB::Black);
      sinceNextState = 0;
    });

State stateBloom(
    [] {
      Serial.printf("State: stateBloom\n");
      sinceNextState = 0;
      setLeds(CRGB::Blue);
    },
    [] {
      if (sinceNextState > 2000)
      {
        lightsFsm->trigger(EV_NEXT);
      }
    },
    [] {
    });

State stateFillAndStir(
    [] {
      Serial.printf("State: stateFillAndStir\n");
      sinceNextState = 0;
      setLeds(CRGB::Green);
    },
    [] {
      if (sinceNextState > 5000)
      {
        lightsFsm->trigger(EV_NEXT);
      }
    },
    [] {
    });

State stateEnd(
    [] {
      Serial.printf("State: stateEnd\n");
      sinceNextState = 0;
      setLeds(CRGB::Red);
    },
    [] {
      if (sinceNextState > 2000)
      {
        lightsFsm->trigger(EV_NEXT);
      }
    },
    [] {});

State stateOff([] {
  Serial.printf("State: stateOff\n");
});

void addLightsFsmStateTransitions()
{
  lightsFsm->add_transition(&stateStart, &stateBloom, EV_BUTTON_PRESSED, NULL);
  lightsFsm->add_transition(&stateBloom, &stateFillAndStir, EV_NEXT, NULL);
  lightsFsm->add_transition(&stateFillAndStir, &stateEnd, EV_NEXT, NULL);
  lightsFsm->add_transition(&stateEnd, &stateOff, EV_NEXT, NULL);
}

//--------------------------------------------

#include <Button2.h>

Button2 button(39);

void buttonClicked(Button2 &btn)
{
  Serial.printf("button clicked!\n");

  lightsFsm->trigger(EV_BUTTON_PRESSED);
}

//------------------------------------------

void setup()
{
  Serial.begin(115200);
  Serial.printf("Ready\n");

  button.setPressedHandler(buttonClicked);

  lightsFsm = new Fsm(&stateStart);
  addLightsFsmStateTransitions();
}

void loop()
{
  lightsFsm->run_machine();

  button.loop();

  vTaskDelay(10);
}