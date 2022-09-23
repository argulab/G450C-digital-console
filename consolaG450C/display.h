#ifndef Display_h
#define Display_h

#include "Arduino.h"

#define BLACK   0x0000
#define RED     0xF800
#define WHITE   0xFFFF
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define GREY    0x5AEB
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F

#define TFT_CS 39
#define TFT_DC 41
#define CX 120      //Coodinate X of center of the circumference (pixels)
#define CY 120      //Coodinate Y of center of the circumference (pixels)

class Display {
  public:
    Display ();
    void init ();
    void display_welcome ();
    void display_initial_screen ();
    void display_beam_position (String text);
    void display_overlap_position (String text);
    void display_cardinal_position (String text);
    void display_preset (String text);
    void draw_left_arrow (bool draw);
    void draw_right_arrow (bool draw);
    String format_degrees (uint16_t dg);
    String format_cardinal_position (uint16_t beam);
    void draw_triangle (float theta);
    void display_to (String text);
  private: 
    void draw_main_coodinate_lines ();
    void draw_cardinal_points (); 
    float polar_to_x(float theta, uint16_t module);
    float polar_to_y(float theta, uint16_t module);
};
#endif