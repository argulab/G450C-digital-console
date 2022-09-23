/*
* Display class with the methods that manage the indicators of the screen
*/

#include <SPI.h>
#include "Display.h"

#include <Adafruit_ILI9341.h>
Adafruit_ILI9341 tft = Adafruit_ILI9341 (TFT_CS, TFT_DC);

Display::Display () {};

/*----------------------------------------- Initial screen */

void Display::init() {
  tft.begin ();
  tft.setRotation (3);
  tft.fillScreen (GREY);
}

void Display::display_welcome () {
  tft.setTextSize (2);
  tft.setTextColor (YELLOW);
  tft.setCursor (20, 96);
  tft.print ("Yaesu G450 console, v2.0");
  tft.setCursor (44, 120);
  tft.print ("by EA2J, march 2022");
  
  delay (2000);
  tft.fillScreen (GREY);
}

/* ----------------------------------------  Format functions */

String Display::format_degrees (uint16_t dg) {
  String stringDegrees = String (dg); 
  uint8_t spaces = stringDegrees.length ();

  if (spaces == 1) {
    return "  " + stringDegrees + char (0xF7);
  } else if (spaces == 2) {
    return " " + stringDegrees + char (0xF7);
  } else {
    return "    ";
  }
}

String Display::format_cardinal_position (uint16_t beam) {
  if (beam >= 338 || beam <  23) return "N";
  if (beam >=  23 && beam <  68) return "NE";
  if (beam >=  68 && beam < 113) return "E";
  if (beam >= 113 && beam < 158) return "SE";
  if (beam >= 158 && beam < 203) return "S";
  if (beam >= 203 && beam < 248) return "SW";
  if (beam >= 248 && beam < 293) return "W";
  if (beam >= 293 && beam < 344) return "NW";
  return "";
}

/* ----------------------------------------  Drawing the screen */

const uint16_t rdOuter = 118;
const uint16_t rdInner = 110;
const uint16_t rdCenter = 10;
const float skipPhase = 90;
const float radConversion = PI / 180;

void Display::draw_main_coodinate_lines () {
  float sx;
  float sy;
  float deltaXinit;
  float deltaYinit;
  float deltaXend;
  float deltaYend;

  for (uint16_t dgs = 0; dgs < 360; dgs += 45) {
    sx = cos ((dgs - skipPhase) * radConversion);
    sy = sin ((dgs - skipPhase) * radConversion);
    
    deltaXinit = sx * 114 + CX;
    deltaYinit = sy * 114 + CY;
    deltaXend = sx * 100 + CX;
    deltaYend = sy * 100 + CY;
    tft.drawLine (deltaXinit, deltaYinit, deltaXend, deltaYend, CYAN );
  }
}

void Display::draw_cardinal_points () {
  float sx;
  float sy;
  float deltaXinit;
  float deltaYinit;
  
  for (uint16_t dgs = 0; dgs < 360; dgs += 5) {
    sx = cos ((dgs - skipPhase) * radConversion);
    sy = sin ((dgs - skipPhase) * radConversion);
        
    deltaXinit = sx * 102 + CX;
    deltaYinit = sy * 102 + CY;
    tft.drawPixel (deltaXinit, deltaYinit, CYAN);
    tft.setTextColor (WHITE, BLACK);

    switch (dgs) {
      case 0:
        tft.fillCircle (deltaXinit, deltaYinit, 2, WHITE);
        tft.setCursor (deltaXinit - 5, deltaYinit + 8);
        tft.print ("N");
        break;
      case 90:
        tft.fillCircle (deltaXinit, deltaYinit, 2, WHITE);
        tft.setCursor (deltaXinit - 16, deltaYinit - 7);
        tft.print ("E");
        break;
      case 180:
        tft.fillCircle (deltaXinit, deltaYinit, 2, WHITE);
        tft.setCursor (deltaXinit - 5, deltaYinit - 22);
        tft.print ("S");
        break;
      case 270:
        tft.fillCircle (deltaXinit, deltaYinit, 2, WHITE);
        tft.setCursor (deltaXinit + 10, deltaYinit - 7);
        tft.print ("W");
        break;
    }
  }  
}

void Display::display_initial_screen () {
  tft.fillCircle (CX, CY, rdOuter, CYAN);
  tft.fillCircle (CX, CY, rdInner, BLACK);
  tft.fillCircle (CX, CX, rdCenter, WHITE); 

  draw_main_coodinate_lines ();
  draw_cardinal_points ();

  tft.fillRect (241, 0, 80, 240, BLACK);

  tft.setTextSize (3);
  tft.setTextColor (WHITE, BLACK);
  
  tft.setCursor (255, 4);
  tft.print ("AZT");

  tft.fillRect (241, 59, 79, 27, BLUE);

  tft.setCursor (246, 94);
  tft.print ("PRST");

  tft.fillRect (241, 147, 79, 3, WHITE);

  tft.setCursor (264, 156);
  tft.print ("TO");

  tft.fillRect (241, 208, 79, 27, BLUE);
}

/*
 * -------------------------------------------------------  drawing pointier of the compass -------------------------
 * The needle of the compass is a isosceles triangle. 
 * The apex coordenates are computed by
 * theta is the direction angle of the antenna beam in radians phased -90 dgrees
 * coordinate x = sin (theta) * radius + center x coordinate 
 * coordinate y = cos (theta) * radius + center y coordinate
 * The 2,8'' screen have 320 px (x) x 240 px (y), the center of sphere this 120 x 120 (xy)
 * radius = (module) 70px
 */

float last_apex_x = 0;
float last_apex_y = 0;
float last_right_corner_x = 0;
float last_right_corner_y = 0;
float last_left_corner_x = 0;
float last_left_corner_y = 0;

void Display::draw_triangle (float theta) {
  uint16_t module = 70;
  uint16_t center_x = 120;
  uint16_t center_y = 120;
  float attack_angle = (40 * (PI / 180)) / 2;
  uint16_t longitude = 58;

  tft.fillTriangle (last_apex_x, last_apex_y, last_right_corner_x, last_right_corner_y, last_left_corner_x, last_left_corner_y, BLACK);
  
  last_apex_x = polar_to_x (theta, module) + center_x;
  last_apex_y = polar_to_y (theta, module) + center_y;

  last_right_corner_x = polar_to_x (theta - attack_angle, module - longitude) + center_x;
  last_right_corner_y = polar_to_y (theta - attack_angle, module - longitude) + center_y;

  last_left_corner_x = polar_to_x (theta + attack_angle, module - longitude) + center_x;
  last_left_corner_y = polar_to_y (theta + attack_angle, module - longitude) + center_y;

  tft.fillTriangle (last_apex_x, last_apex_y, last_right_corner_x, last_right_corner_y, last_left_corner_x, last_left_corner_y, WHITE);
}

float Display::polar_to_x (float angle, uint16_t module) {
  return cos (angle) * module;
}

float Display::polar_to_y (float angle, uint16_t module) {
  return sin (angle) * module;
}

/* ------------------------------------------------------- drawing the box data */

 void Display::display_beam_position (String text) {
  tft.setTextSize (3);
  tft.setTextColor (YELLOW, BLACK);
  tft.setCursor (246, 32);
  tft.print (text);
}

void Display::display_overlap_position (String text) {
  tft.setTextSize (3);
  tft.setTextColor (WHITE, BLUE);
  tft.setCursor (255, 62);
  tft.print (text);
}

void Display::display_cardinal_position (String text) {
  uint8_t digits = text.length ();
  uint16_t xPos = digits == 2 ? 264 : 273;
  
  tft.setTextSize (3);
  tft.setTextColor (WHITE, BLUE);
  tft.setCursor (255, 211);
  tft.print ("   ");
  tft.setCursor (xPos,211);
  tft.print (text);  
}

/* ------------------------------------------------------------ Rotor turning */

 void Display::draw_left_arrow (bool draw) {
  if (draw) {
    tft.fillTriangle (4, 224, 16, 216, 16, 230, YELLOW);
    tft.fillRect (16, 220, 17, 6, YELLOW);    
  } else {
    tft.fillRect (4, 214, 30, 18, GREY);    
  }
 }

void Display::draw_right_arrow (bool draw) {
  if (draw) {
    tft.fillTriangle (236, 224, 224, 216, 224, 230, YELLOW);
    tft.fillRect (210, 220, 17, 6, YELLOW);    
  } else {
    tft.fillRect (210, 216, 30, 18, GREY);    
  }  
}

void Display::display_preset (String text) {
  tft.setTextSize (3);
  tft.setTextColor (YELLOW, BLACK);
  tft.setCursor (246, 122);
  tft.print (text);
}

void Display::display_to (String text) {
  tft.setTextSize (3);
  tft.setTextColor (YELLOW, BLACK);
  tft.setCursor (246, 184);
  tft.print (text);
}