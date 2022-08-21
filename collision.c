#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <graphics.h>
#include <macros.h>
#include <lcd.h>
#include "ram_utils.h"
#include <stdbool.h>

#include "starfighter.h"
#include "plasma.h"
#include "objects.h"
#include "helpers.h"

// ===================
// DEFINITIONS
// ===================
int ast_count=0,bould_count=0,frag_count=0;
char * asteroid ="   1     111   11111 1111111 11111   111     1   ";
char * boulder ="  1   111 11111 111   1  ";
char * fragment =" 1 111 1 ";

char * plasma = "1111";

// ===================
// FUNCTIONS
// ===================
bool collision(int x0, int y0, int w0, int h0, char pixels0[], int x1, int y1, int w1, int h1, char pixels1[]){
int x = 0;
int y = 0;
for (x = x0; x < x0 + w0; x++) {
	for ( y = y0; y < y0 + h0; y++) {
		if (( x >= x1) && (x < x1 + w1)) {
			if (( y >= y1) && (y < y1+h1)) {
				if ((pixels1[(x-x1)+(y-y1)*w1] != ' ')&&(pixels0[(x-x0)+(y-y0)*w0] != ' ')){
					return true;
				}}}}}
	return false;
}

//checks collision between plasma and asteroid
void check_collision_asteroid(){
	for( int i = 0; i<get_max_plasma(); i++){
		for(int j = 0; j<get_max_asteroid(); j++){
			if (get_ax(j)<LCD_X && get_ay(j)<LCD_Y){
			if (collision(get_ax(j),get_ay(j),7,7,asteroid,get_px(i),get_py(i),2,2,plasma)){
				draw_int(10,10,1,FG_COLOUR);
				do_collided_object(1,j);
				plasma_collide(i);
				ast_count++;
			}
			}
		}
	}
}

//checks collision between plasma and boulder
void check_collision_boulder(){
	for( int i = 0; i<get_max_plasma(); i++){
		for( int k = 0; k<get_max_boulder(); k++){
			if (get_bx(k)<LCD_X&& get_by(k)<LCD_Y){
				if (collision(get_bx(k),get_by(k),5,5,boulder,get_px(i),get_py(i),2,2,plasma)) {
					do_collided_object(2,k);
					plasma_collide(i);
					bould_count++;
				}
			}
		}
	}
}

//checks collision between plasma and fragment
void check_collision_fragment(){
	for( int i = 0; i<get_max_plasma(); i++){
		for( int l = 0; l<get_max_fragment(); l++){
			if (get_fx(l)<LCD_X&& get_fy(l)<LCD_Y){
				if (collision(get_fx(l),get_fy(l),3,3,fragment,get_px(i),get_py(i),2,2,plasma)) {
					do_collided_object(3,l);
					plasma_collide(i);
					frag_count++;
				}
			}
		}
	}
}

/// to use in other .c files.
char * char_asteroid(){
	return asteroid;
}
char * char_boulder(){
	return boulder;
}
char * char_fragment(){
	return fragment;
}
char * char_plasma(){
	return plasma;
}
