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

void setLed(CRGB colour, int i, bool show = true)
{
  leds[i] = colour;
  if (show)
    FastLED.show();
}

void setLeds(CRGB colour, bool show = true)
{
  for (int num = 0; num < NUM_LEDS; num++)
  {
    leds[num] = colour;
  }
  if (show)
    FastLED.show();
}

bool isIn(int *data, int len, int idx)
{
  int buff[len];
  memcpy(buff, data, len);
  for (int i = 0; i < len; i++)
  {
    if (buff[i] == idx)
    {
      Serial.printf("X");
      return true;
    }
  }
  Serial.printf("-");
  return false;
}

enum Pattern
{
  NONE,
  MIDDLE_DOT,
  INNER_RING,
  INNER_RING_SOLID,
  OUTER_RING,
  OUTER_RING_THICK
};

int middle_dot[] = {12};
int inner_ring[] = {6, 7, 8,
                    11, 13,
                    16, 17, 18};
int outer_ring[] = {0, 1, 2, 3, 4,
                    5, 9,
                    10, 14,
                    15, 19,
                    20, 21, 22, 23, 24};
int outer_ring_thick[] = {0, 1, 2, 3, 4,
                          5, 6, 7, 8, 9,
                          10, 11, 13, 14,
                          15, 16, 17, 18, 19,
                          20, 21, 22, 23, 24};

void setPattern(CRGB colour, Pattern pattern)
{
  switch ((int)pattern)
  {
  case (int)MIDDLE_DOT:
  {
    setLeds(CRGB::Black, false);
    leds[12] = colour;
    FastLED.show();
    break;
  }

  case INNER_RING:
  {
    setLeds(CRGB::Black, false);
    for (int i = 0; i < sizeof(inner_ring) / sizeof(inner_ring[0]); i++)
      leds[inner_ring[i]] = colour;
    FastLED.show();
    break;
  }

  case INNER_RING_SOLID:
  {
    setLeds(CRGB::Black, false);
    for (int i = 0; i < sizeof(inner_ring) / sizeof(inner_ring[0]); i++)
      leds[inner_ring[i]] = colour;
    leds[12] = colour;
    FastLED.show();
    break;
  }

  case OUTER_RING:
  {
    setLeds(CRGB::Black, false);
    for (int i = 0; i < sizeof(outer_ring) / sizeof(outer_ring[0]); i++)
      leds[outer_ring[i]] = colour;
    FastLED.show();
    break;
  }

  case OUTER_RING_THICK:
  {
    setLeds(CRGB::Black, false);
    for (int i = 0; i < sizeof(outer_ring_thick) / sizeof(outer_ring_thick[0]); i++)
      leds[outer_ring_thick[i]] = colour;
    FastLED.show();
    break;
  }
  }
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

#define BLOOM_TIME_MS 30 * 1000
#define BLOOM_TIME_ENDING_MS 5 * 1000
#define FILL_AND_STIR_MS 90 * 1000
#define FILL_AND_STIR_ENDING_MS 10 * 1000
#define END_TIME_MS 10 * 1000

// #define BLOOM_TIME_SECONDS 3
// #define FILL_AND_STIR_SECONDS 3
// #define END_TIME 3

elapsedMillis sinceNextState = 0;

State stateStart(
    [] {
      Serial.printf("State: stateStart\n");

      FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
      FastLED.setBrightness(30);

      setPattern(CRGB::Green, MIDDLE_DOT);
      sinceNextState = 0;
    });

State stateBloom(
    [] {
      Serial.printf("State: stateBloom\n");
      setPattern(CRGB::Blue, INNER_RING);
      sinceNextState = 0;
    },
    [] {
      if (sinceNextState > BLOOM_TIME_MS - BLOOM_TIME_ENDING_MS)
      {
        lightsFsm->trigger(EV_NEXT);
        // sinceNextState = 0;
      }
    },
    [] {
    });

State stateBloomEnding(
    [] {
      Serial.printf("State: stateBloomEnding\n");
      setPattern(CRGB::Blue, INNER_RING_SOLID);
    },
    [] {
      if (sinceNextState > BLOOM_TIME_MS)
      {
        lightsFsm->trigger(EV_NEXT);
        sinceNextState = 0;
      }
    },
    [] {
    });

State stateFillAndStir(
    [] {
      Serial.printf("State: stateFillAndStir\n");
      setPattern(CRGB::Green, OUTER_RING);
    },
    [] {
      if (sinceNextState > FILL_AND_STIR_MS - FILL_AND_STIR_ENDING_MS)
      {
        lightsFsm->trigger(EV_NEXT);
        // dont zero timer
      }
    },
    [] {
    });

State stateFillAndStirEnding(
    [] {
      Serial.printf("State: stateFillAndStirEnding\n");
      setPattern(CRGB::Green, OUTER_RING_THICK);
    },
    [] {
      if (sinceNextState > FILL_AND_STIR_MS)
      {
        lightsFsm->trigger(EV_NEXT);
        sinceNextState = 0;
      }
    },
    [] {
    });

State stateEnd(
    [] {
      Serial.printf("State: stateEnd\n");
      setLeds(CRGB::Red);
    },
    [] {
      if (sinceNextState > END_TIME_MS)
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
  lightsFsm->add_transition(&stateBloom, &stateBloomEnding, EV_NEXT, NULL);
  lightsFsm->add_transition(&stateBloomEnding, &stateFillAndStir, EV_NEXT, NULL);
  lightsFsm->add_transition(&stateFillAndStir, &stateFillAndStirEnding, EV_NEXT, NULL);
  lightsFsm->add_transition(&stateFillAndStirEnding, &stateEnd, EV_NEXT, NULL);
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