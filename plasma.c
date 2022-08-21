#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <string.h>
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
#include "objects.h"
#include "starfighter.h"
#include "plasma.h"
#include "helpers.h"
#include "collision.h"
#include <math.h>

// ===================
// DEFINITIONS
// ===================
#define MAX_PLASMA 50
#define PLASMA_SPEED 0.8
#define PLASMA_W 2
#define PLASMA_H 2
int count;
int plasma_count=0;
double shot_time=0;

struct SHOT{
	    bool flag;
	    double x;
	    double y;
      double angle;
};

//array for struct
struct SHOT shot[MAX_PLASMA];

//counter for ISR
volatile uint8_t game_counter = 0;

//==================
// FUNCTIONS
//=================

//reset plasma
void reset_plasma(){
	for(int i = 0; i < MAX_PLASMA; i++){
		shot[i].flag=false;
		shot[i].x = 100;
		plasma_count=0;

	}
}

//interrupt service routine for timer 3
ISR(TIMER3_OVF_vect) {
  game_counter ++;
}

//game time (is always incrementing)
double game_time(){
	return (game_counter * 65536.0 + TCNT3) * 256 / 8000000;
}

//shoot plasma
void shoot_plasma(){
	double adc_value = adc_read(0)/70;
	double adc_angle = 30+adc_read(0)/8.525;
	for(int i = 0; i < MAX_PLASMA; i++){
		if(shot[i].flag==false && (game_time()>shot_time+(double)0.2)){
			  shot_time=game_time();
				shot[i].flag =true;
				shot[i].x = get_sx() + adc_value-2;
				shot[i].y = get_sy() - 5;
				shot[i].angle = -M_PI+(adc_angle*(M_PI/180));
				plasma_count++;
				break;
				}
	}
}

//update all plasmas
void update_plasmas(){
	for(int i=0;i<MAX_PLASMA;++i){
		if(shot[i].flag ){
				shot[i].x+=cos(shot[i].angle)*PLASMA_SPEED;
				shot[i].y+=sin(shot[i].angle)*PLASMA_SPEED;
				if(shot[i].y<-2 || shot[i].x<-2 || shot[i].x>85){//|| shot[i].x<-2 || shot[i].x>85
						shot[i].flag=false;
						plasma_count--;
						shot[i].x = 80;
				}
		}
	}
}

// called when plasma collides with any object
void plasma_collide(int i){
		shot[i].y = 120;
		plasma_count--;
		shot[i].flag = false;
}

//draws all plasmas
void draw_plasmas(){
  for(int i=0;i<MAX_PLASMA;++i){
    if(shot[i].flag){
			draw_pixels(shot[i].x,shot[i].y,2,2,char_plasma());
    }
  }
}

///=========
double get_px(int i){
  return shot[i].x;
}

double get_py(int i){
  return shot[i].y;
}

int get_max_plasma(){
  return MAX_PLASMA;
}

int get_plasma_count(){
	return plasma_count;
}
