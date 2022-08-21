#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <graphics.h>
#include <macros.h>
#include "lcd_model.h"
#include <lcd.h>
#include "ram_utils.h"
#include <stdbool.h>
#include "ascii_font.h"
// #include "usb_serial.h"

#include "helpers.h"
#include "objects.h"
#include "starfighter.h"
// ===================
// DEFINITIONS
// ===================
#define STARF_W 11
#define STARF_H 5
#define STARF_SPEED 0.3

double sx,sy,sdx;
int tx, ty;

char * starfighter = "    111    1111111111111111111111             1111111  ";
bool stationary;

//==================
// FUNCTIONS
//=================

//setup starfighter
void setup_starf(void) {
    sx = (LCD_X/2)-STARF_W/2;
    sy = LCD_Y - 5;
    sdx = 1;
    if(rand()%2==0) sdx*=-1; //50:50 chance of going either right or left
}

//moves starfighter left
void move_starf_left(){
  if(sdx==0) sdx = -1; // go in direction that toggle is pressed
  else if (sdx != -1) sdx = 0;
}

//moves starfighter right
void move_starf_right(){
  if(sdx==0) sdx = 1; // go in direction that toggle is pressed
  else if (sdx != 1) sdx = 0;
}

//update starfighter
void update_starf() {
    if(sx > 0 && sdx < 0) { // when all of the above
        sx += sdx; // can keep increasing
        tx += sdx;
    }
    if( (sx + STARF_W) < LCD_X && sdx > 0) { // when all of the above
        sx += sdx; // can keep increasing
        tx += sdx;
    }
}

//draws starfighter
void draw_starf(void) {
  draw_pixels(sx,sy,STARF_W,STARF_H,starfighter);
}

//draws turret
void draw_turret(){
    int x = sx+5;
    int y = sy;
    int adc_value = adc_read(0)/(1024/10);
    tx = sx+ adc_value ;//sx + (5* cos(adc_value * M_PI / 180));
    ty = 41;//sy  -(5 * sin(adc_value * M_PI / 180));
    draw_line(x,y,tx,ty, FG_COLOUR);
    // draw_string(x,y,)
  }

//=======================
int get_sx(){
  return sx;
}
int get_sy(){
  return sy;
}
int get_sw(){
  return STARF_W;
}
int get_sh(){
  return STARF_H;
}
char * char_starf(){
  return starfighter;
}
