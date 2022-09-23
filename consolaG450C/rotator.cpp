#include "HardwareSerial.h"
#include <stdint.h>
#include "Arduino.h"
#include "Rotator.h"
#include "Display.h"

String LastFormattedCardinalPosition = "";
uint16_t LastBeamPosition = 999;
float maxRawReference = 948.0;
bool LastSwButtonState = LOW;

Display dsp_02 = Display ();

Rotator::Rotator () {}

void Rotator::init_gpios () {
  pinMode (CCW_BUTTON_PIN, INPUT_PULLUP);
  pinMode (CW_BUTTON_PIN, INPUT_PULLUP);
  pinMode (PRESET_BUTTON_PIN, INPUT_PULLUP);
  pinMode (CCW_RELAY_PIN, OUTPUT);
  pinMode (CW_RELAY_PIN, OUTPUT);
  digitalWrite (CCW_RELAY_PIN, LOW);
  digitalWrite (CW_RELAY_PIN, LOW);  
}

/* --------------------------------------------------------- DEBUGING */

uint16_t counter = 0;
uint16_t maxRead = 0;
uint16_t minRead = 9999;

void Rotator::debug_analogic_input (uint16_t read) {
  while (counter < 10000) {
    if (read > maxRead) maxRead = read;
    if (read < minRead) minRead = read;
    counter++;
  }

  if (counter == 10000) {
    Serial.print("Max Read = ");
    Serial.println(maxRead);
    Serial.print ("Min Read ");
    Serial.println (minRead);

    counter = 0;
    maxRead = 0;
    minRead = 9999;
  }
}

void Rotator::debug_turning_state (presetStatus status) {
  Serial.print ("Estado = ");
  switch (status) {
    case WAITTING:
      Serial.println ("WAITTING");
      break;
    case SETTING:
      Serial.println ("SETTING");
      break;
    case TURNING:
          Serial.println ("TURNING");
      break;
  }
}

void Rotator::debug_analogic_reading (uint16_t value) {
  Serial.print ("volts = ");
  Serial.println((value * 5.0) / 1023, 2);
}

/*  -------------------------------------------------------- ANTENNA POSITION
 * The max raw read for the ADC (10bit) in the analogic input powered with a external power is 925 RAW
 * The most important feature is to estabilize the power with a hard RC filter (4K7/33uF) and use the 
 * software filter with the average of 100 reads.
 * Besides, the float comma to integer conversion of variables most be very careful.
 */

uint16_t Rotator::read_analogic_input (uint8_t analogPin, uint32_t maxDegrees) {
  uint32_t readPosition = 0;
  uint32_t readSumma = 0;
  uint32_t maxRaw = 1023;

  for (uint8_t i = 0; i < 128; i++) {
    readPosition = analogRead (analogPin); 
    readSumma += readPosition; 
  }

  #ifdef DEBUG_ANALOGIC_READ
    debug_analogic_reading(int (readSumma / 128.0));
    delay (2000);
  #endif//DEBUG_ANALOGIC_READ

  return ((readSumma / 128) * maxDegrees) / maxRaw;
}

/* ---------------------------------------------  Pointer of compass mouvement  ---*/

void Rotator::do_antenna_position_change (uint32_t position) {
  bool ovl = position > 360;
  uint16_t theta = position % 360;
  String formattedOvlap = ovl ? "OVL" : "   ";
  String formattedCardinalPosition = dsp_02.format_cardinal_position (theta);
  
  dsp_02.display_beam_position (dsp_02.format_degrees (theta));
  dsp_02.display_overlap_position (formattedOvlap);
  
  if (formattedCardinalPosition != LastFormattedCardinalPosition) {
    LastFormattedCardinalPosition = formattedCardinalPosition;
    dsp_02.display_cardinal_position (formattedCardinalPosition);
  }
  dsp_02.draw_triangle ((theta - 90.0) * (PI / 180.0));
}

/* ---------------------------------------------------------- PRESET */

bool Rotator::read_button_pressed (uint8_t pin) {
  bool pressed = digitalRead (pin) == LOW;
  
  if (pressed) {
    delay (400);
    if (digitalRead (pin) == LOW)
      return true;
  }
  return false;
}

rotation Rotator::compute_turning_direction (uint16_t pst, uint16_t ant) {
  uint8_t overLap = ant > 360;
  uint16_t antDir = ant % 360;

  if (overLap) {
    if (pst > 90) return CCW;
    return pst > antDir ? CW : CCW;
  }

  if (pst > 90) return pst > antDir ? CCW : CW;
  return (pst + 180) > antDir ? CW : CCW;
}