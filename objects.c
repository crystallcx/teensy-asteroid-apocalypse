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
#include "starfighter.h"
#include "plasma.h"
#include "objects.h"
#include "collision.h"

// ===================
// DEFINITIONS
// ===================

#define ASTEROID_W 7
#define ASTEROID_H 7
#define FRAG_W 3
#define FRAG_H 3
#define BOULDER_W 5
#define BOULDER_H 5
#define MAX_B 6
#define MAX_A 3
#define MAX_F 12

double by[MAX_B], bx[MAX_B], bdx[MAX_B], bdy[MAX_B];
double fx[MAX_F], fy[MAX_F], fdx[MAX_F], fdy[MAX_F];
double ay[MAX_A], ax[MAX_A];
int score=0;
int lives = 5;
int count_asteroid,count_boulder,count_fragment;

//==================
// FUNCTIONS
//=================


//initialise objects
void init_objects(){
	for (int k = 0; k<MAX_A; k++){
		ax[k] = rand() % LCD_X;
		ay[k] = -1*rand() %LCD_Y;
		while (ax[k] < 7 || ax[k]> (LCD_X-ASTEROID_W-5)) ax[k] = rand() % LCD_X;
		while (ay[k] > -7 || ay[k]<-15) ay[k] = -1*rand() % LCD_Y;
		if (ax[k]<LCD_X/2) {
			SET_BIT(PINB,2);
			_delay_ms(20);
			CLEAR_BIT(PINB,2);
			_delay_ms(20);
			SET_BIT(PINB,2);
			_delay_ms(20);
			CLEAR_BIT(PINB,2);
		}
		else if (ax[k]>LCD_X/2){
			SET_BIT(PINB,3);
			_delay_ms(20);
			CLEAR_BIT(PINB,3);
			_delay_ms(20);
			SET_BIT(PINB,3);
			_delay_ms(20);
			CLEAR_BIT(PINB,3);
		}
	}
	for (int i = 0; i<MAX_F; i++){
		fx[i] = 100;
	}
	for (int j = 0; j<MAX_B; j++){
		bx[j] = 100;
	}
}

//draw 1 object
void draw_asteroid(int i){
		draw_pixels(ax[i], ay[i], ASTEROID_W, ASTEROID_H, char_asteroid());
}
void draw_boulder(int i) {
		 draw_pixels(bx[i], by[i], BOULDER_W, BOULDER_H, char_boulder());
}
void draw_fragment(int i){
	 draw_pixels(fx[i], fy[i], FRAG_W, FRAG_H, char_fragment());
}

//draw all objects
void draw_objects() {
    for (int i =0; i < MAX_A; i++){
            draw_asteroid(i);
    }
		for (int j =0; j < MAX_B; j++){
						draw_boulder(j);
		}
		for (int k =0; k < MAX_F; k++){
            draw_fragment(k);
    }
}

//update 1 object
void update_object(int type, int i) {
	double adc_value = (double) adc_read(1);
	double delayy = adc_value/2048;
	switch(type){
		case 0: //asteroids
		ay[i]+= delayy;
		if (ay[i] >= 39-6 && ax[i]<84) {
			  ay[i] = 100;
				lives --;
				count_asteroid --;
		}
		break;
		case 1: //boulders
		by[i]+= delayy;
		if (by[i] >= 39-4 && bx[i]<84) {
			  bx[i] = 100;
				lives --;
				count_boulder --;
		}
		break;
		case 2: //fragments
		fy[i]+= delayy;
		if (fy[i] >= 39-2 && fx[i]<84) {
			  fx[i] = 100;
				lives --;
				count_fragment --;
		}
		break;
	}
}

//new wave
void new_wave(){
	if (count_fragment==0 && count_asteroid == 0 && count_boulder == 0) {
		for (int k = 0; k<MAX_A; k++){
			ax[k] = rand() % LCD_X;
			ay[k] = -1*rand() %LCD_Y;
			while (ax[k] < 7 || ax[k]> (LCD_X-ASTEROID_W-5)) ax[k] = rand() % LCD_X;
			while (ay[k] > -7 || ay[k]<-15) ay[k] = -1*rand() % LCD_Y;
		}
		count_asteroid = 3;
		reset_objects_left();
	}
}

//update all objects
void update_objects() {
	new_wave();
	for ( int j = 0; j < MAX_A; j++ ) {
		if (ax[j] <84) update_object(0 , j);
	}
  for ( int i = 0; i < MAX_B; i++ ) {
		if (bx[i] <84) update_object(1 , i);
	}
	for ( int k = 0; k < MAX_F; k++ ) {
		if (fx[k] <84) update_object( 2, k);
	}
}

//called when an object collides with a plasma.
void do_collided_object(int object_type, int i){
  switch(object_type){
    case 1: //case 2 : if asteroid[i] collides with missile
			bx[2*i] = ax[i]-5;
			by[2*i] = ay[i];
			draw_boulder(2*i); update_object(1 , 2*i);
			bx[2*i+1] = ax[i]+5;
			by[2*i+1] = ay[i];
			draw_boulder(2*i+1); update_object(1 , 2*i+1);
			ax[i] = 100;
			count_asteroid --;
			count_boulder +=2;
			score++;
			break;
    case 2: //
			fx[2*i] = bx[i]-3;
			fy[2*i] = by[i];
			draw_fragment(2*i); update_object(2 , 2*i);
			fx[2*i+1] = bx[i]+3;
			fy[2*i+1] = by[i];
			draw_fragment(2*i+1); update_object(2, 2*i+1);
			bx[i] = 100;
			by[i] = -7;
			count_fragment +=2; count_boulder --;
			score += 2;
      break;
		case 3:
			fx[i] = 100;
			count_fragment --;
			score +=4;
			break;
  }
}

//set score
void set_score(){
	score = usb_serial_getchar();
 if (score < 0) {
	 score = 0;
 }
}

//set lives
void set_lives(){
	 lives = usb_serial_getchar();
	 if (lives < 0 || lives > 5) {
	 lives = 5;
	 }
}

//get remaining objects
int objects_left(int type){
		switch(type){
			case 0:
			return count_asteroid;
			break;
			case 1:
			return count_boulder;
			break;
			case 2:
			return count_fragment;
			break;
		}
		return 0;
}

void reset_score(){
	score=0;
}

void reset_lives(){
	lives = 5;
}

int get_ax(int i){
  return ax[i];
}
int get_ay(int i){
  return ay[i];
}
int get_bx(int i){
  return bx[i];
}
int get_by(int i){
  return by[i];
}
int get_fx(int i){
  return fx[i];
}
int get_fy(int i){
  return fy[i];
}

int get_max_asteroid(){
  return MAX_A;
}
int get_max_boulder(){
  return MAX_B;
}
int get_max_fragment(){
  return MAX_F;
}

int get_score(){
	return score;
}
int get_lives(){
	if (lives<0) {
		lives = 0;
	}
	return lives;
}

void reset_objects_left(){
	count_asteroid=3;
	count_boulder=0;
	count_fragment=0;
}
