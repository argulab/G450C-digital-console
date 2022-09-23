/*
* Analog console for rotor Yaesu G450C by Arsenio "Enio" Gutierrez EA2J
* Version 3.6, 14 september 2022
* This program is free software: you can redistribute it and/or modify it 
* under the terms of the GNU General Public License as published by
* the Free Software Foundation.
* This program is distribuited in the hope that is will useful, but
* without any warranty.
* The program is designed for the digital control of a rotor type G450C with
* a graphic display TFT with a ILI4391 driver.
* The graphic design, circuit boards and schematics can be downloaded from this same site.
*/

#include "Rotator.h"
#include "Display.h"

uint16_t lastAntennaPosition;
uint16_t lastPresetPosition;
presetStatus currentPresetState = WAITTING;

const uint16_t maxAntennaDegrees = 450;
const uint16_t maxPresetDegrees = 360;

Rotator rtr = Rotator ();
Display dsp_01 = Display ();

/* ----------------------------------------- Computing events*/
uint32_t compute_antenna_position () {
  uint32_t antenna = rtr.read_analogic_input (AZ_INPUT_PIN, maxAntennaDegrees);
  if (antenna != lastAntennaPosition) {
    lastAntennaPosition = antenna;
    rtr.do_antenna_position_change (antenna);
  }
  return antenna;
}

void setup() {
  uint32_t serialRate = 115200;
  Serial.begin (serialRate);
  Serial.flush();

  rtr.init_gpios ();
  dsp_01.init ();
  dsp_01.display_welcome ();
  dsp_01.display_initial_screen ();

  currentPresetState = WAITTING;
  lastPresetPosition = rtr.read_analogic_input (PRESET_INPUT_PIN, maxPresetDegrees);

  lastAntennaPosition = rtr.read_analogic_input(AZ_INPUT_PIN, maxAntennaDegrees);
  rtr.do_antenna_position_change(lastAntennaPosition);
}

/* ------------------ THE MAGIC STARTS HERE -- LOOP () 
The console manages three states:
WAITTING, 
is waiting for three events,
the CCW button has been pressed
the CW button has been pressed
the preset pot has varied more than 2 degrees CCW/CW
on CCW or CW button pressed, the rotor turn while button are pressed, the position indicators change with rotor position
If the preset postion are changed, the state change to SETTING
SETTING,
ndicates on screen the position of the preselected antenna
If the PRESET button are pressed init the state to TURNING
If the time elapsed for turnin (8s) is finish without the PRESET are pussed the mode is changed to WAITING 
TURNING
Turn the rotor to PRESET.
When the antenna position is the same of the preset, finishing the turn and change to WAITING mode.
If the PRESET button is pussem the turn is aborted and the mode change to WAITTING
*/

bool manualTurning = false;
uint32_t initSettingTime = millis ();
const uint32_t elapsedTimeForSetting = 8000;

void loop() { 
  uint16_t presetPosition = rtr.read_analogic_input (PRESET_INPUT_PIN, maxPresetDegrees);

  switch (currentPresetState) {
    case WAITTING:
      if (presetPosition > lastPresetPosition + 2 || presetPosition < lastPresetPosition - 2) {
        lastPresetPosition = presetPosition;
        initSettingTime = millis ();
        currentPresetState = SETTING;
      }

      while (digitalRead (CCW_BUTTON_PIN) == LOW) {
        manualTurning = true;
        dsp_01.draw_left_arrow (true);
        digitalWrite (CCW_RELAY_PIN, HIGH);
        compute_antenna_position ();
      }

      while (digitalRead (CW_BUTTON_PIN) == LOW) {
        manualTurning = true;
        dsp_01.draw_right_arrow (true);
        digitalWrite (CW_RELAY_PIN, HIGH);
        compute_antenna_position ();   
      }

      if (manualTurning) {
        manualTurning = false;
        dsp_01.draw_left_arrow (false);
        dsp_01.draw_right_arrow (false);
        digitalWrite (CCW_RELAY_PIN, LOW);
        digitalWrite (CW_RELAY_PIN, LOW);
      }

      break;

    case SETTING:
      if (presetPosition != lastPresetPosition) {
        lastPresetPosition = presetPosition;
        dsp_01.display_preset (dsp_01.format_degrees (presetPosition));
      }

      if (rtr.read_button_pressed (PRESET_BUTTON_PIN)) {
        dsp_01.display_preset ("    ");
        dsp_01.display_to (dsp_01.format_degrees (presetPosition));
        currentPresetState = TURNING;
      }

      if (millis () > initSettingTime + elapsedTimeForSetting) {
        dsp_01.display_preset ("    ");
        currentPresetState = WAITTING;
      }
     
      break;

    case TURNING:
      uint16_t antennaPosition = compute_antenna_position ();

      rotation goTo = rtr.compute_turning_direction (presetPosition, antennaPosition);
      switch (goTo) {
        case CCW:
          dsp_01.draw_left_arrow (true);
          digitalWrite (CCW_RELAY_PIN, HIGH);
          break;

        case CW:
          dsp_01.draw_right_arrow (true);
          digitalWrite (CW_RELAY_PIN, HIGH);
      }

      if (rtr.read_button_pressed (PRESET_BUTTON_PIN) || antennaPosition == presetPosition) {
        dsp_01.display_to ("    ");
        dsp_01.draw_left_arrow (false);
        dsp_01.draw_right_arrow (false);
        digitalWrite (CCW_RELAY_PIN, LOW);
        digitalWrite (CW_RELAY_PIN, LOW);
        currentPresetState = WAITTING;
      }
       
      break;
  }

  #ifdef DEBUG_TURNING_STATE
    rtr.debug_turning_state (currentPresetState);
  #endif//DEBUG_TURNING_STATE
}
