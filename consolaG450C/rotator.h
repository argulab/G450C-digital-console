#ifndef Rotator_h
#define Rotator_h
#include "Arduino.h"

#define AZ_INPUT_PIN A0
#define PRESET_INPUT_PIN A2
#define CW_BUTTON_PIN 17
#define CCW_BUTTON_PIN 19
#define PRESET_BUTTON_PIN 21
#define CCW_RELAY_PIN 47
#define CW_RELAY_PIN 46

//#define DEBUG_READ_ANALOGIC_INPUT
//#define DEBUG_TURNING_STATE
//#define DEBUG_ANALOGIC_READ
//#define DEBUG_BUTTONS_STATE

typedef enum {
  CCW, CW
  } rotation;

typedef enum {
    AZIMUTH, PRESET
  } angleReading;

typedef enum {
  WAITTING, SETTING, TURNING
} presetStatus;

class Rotator {
  public:
    Rotator ();
    void init_gpios (); 
    uint16_t read_analogic_input (uint8_t analogPin, uint32_t maxDegrees);
    void do_antenna_position_change (uint32_t position);
    bool read_button_pressed (uint8_t pin); 
    rotation compute_turning_direction (uint16_t pst, uint16_t ant);
    void debug_analogic_input (uint16_t read);
    void debug_turning_state (presetStatus status);
    void debug_analogic_reading (uint16_t value);
  private:   
};
#endif
