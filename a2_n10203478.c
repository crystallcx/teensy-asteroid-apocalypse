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
#include "helpers.h"
#include "starfighter.h"
#include "plasma.h"
#include "objects.h"
#include "collision.h"

// ===================
// DEFINITIONS
// ===================
char buffer[200];

#define BIT(x) (1 << (x))
#define OVERFLOW_TOP (1023)
#define ADC_MAX (1023)

#define FREQ (8000000.0)
#define TIMER_SCALE (256.0)
volatile uint8_t counter = 0;
double start_time=0,reset_time=0,current_time=0;
bool game_over = false;
bool start_game = false;
bool paused;
volatile uint8_t state_count = 0;
volatile uint8_t switch_closed;

//PWM FUNCTION
void set_duty_cycle(int duty_cycle) {
	TC4H = duty_cycle >> 8;
	OCR4A = duty_cycle & 0xff;
}

//SETUP TEENSY
void setup_teensy(void){
  DDRF &= ~((1 << PF6) | (1 << PF5)); //left & right buttons.
  DDRB &= ~((1 << PB0) | (1 << PB1) | (1 << PB7) ); // centre, left, down - joystick
  DDRD &= ~((1 << PD0) | (1 << PD1)); // right, up.
  DDRF &= ~((1 << PF0) | (1 << PF1)); //left & right potentiometer

  DDRB |= (1 << 2); //enable output to LED0
  DDRB |= (1 << 3); //enable output to LED1
  PORTD = (1 << 6); //turn on teensy led
  //SETUP TIMER 1 - 16bit
  TCCR1A = 0; //timer 1 - counter control register A
  TCCR1B = 4; //timer 1 - counter control register B /256
  TIMSK1 = 1; //timer 1 - timer interrupt mask register

  //SETUP TIMER 3 - 16 bit
  TCCR3A = 0;
  TCCR3B = 4;
  TIMSK3 = 1;

  sei();	// Enable timer overflow, and turn on interrupts.
}

//INTERRUPT SERVICE ROUTINE
ISR(TIMER1_OVF_vect) {
  int time = game_time()/1;
  if (!paused) {
    counter ++;
    send_formatted(buffer,sizeof(buffer), "\r Current postion: (%d,%d)\n \r"
    "Elapsed time is %d%d:%d%d\n \n \r"
    "Score: %d\n \r"
    "Lives: %d\n\r"
    "Objects left:%d,%d,%d\n\r"
    "Turret Aim: %d\n\r"
    "Plasma count: %d\n\r",
    get_sx(),get_sy(),(time/60)/10 % 6,time/60 % 10,time/10 % 6 ,time % 10,
    get_score(),get_lives(),objects_left(0),objects_left(1),objects_left(2),
    ((int)((30+adc_read(0)/8.525))),get_plasma_count());
  }
}

//GAME TIMER (PAUSE DOES NOT INCREMENT TIMER)
double elapsed_time() {
  if (paused) return current_time;
  else return (counter * 65536.0 + TCNT1) * TIMER_SCALE / FREQ;
}

//DRAW DEFLECTER SHIELD
void draw_barrier() {
    for (int h = 0; h < LCD_X; h++){
        draw_pixel(h,39, FG_COLOUR); // use foregroung colour at (39,:)
        h++;
    }
}

//INTRO SCREEN
void intro_screen(){
  clear_screen();
  draw_string(20, 10, "Asteroid", FG_COLOUR); //Game Title
  draw_string(20, 20, "Apocalypse", FG_COLOUR); //Game Title
	draw_string(20, 30, "n10203478", FG_COLOUR); //student number
  set_duty_cycle(50);
  if (BIT_IS_SET(PINF,6)) { //intro animation countdown
    clear_screen();
    clear_screen(); draw_string(LCD_X/2,LCD_Y/2,"3",FG_COLOUR); show_screen();
    _delay_ms(500);
    clear_screen(); draw_string(LCD_X/2,LCD_Y/2,"2",FG_COLOUR); show_screen();
    _delay_ms(500);
    clear_screen(); draw_string(LCD_X/2,LCD_Y/2,"1",FG_COLOUR); show_screen();
    _delay_ms(500);
    clear_screen();
    SET_BIT(PORTB,2);
    start_game = true;
  }
  show_screen();
}

//SHOW STATUS SCREEN
void show_status(){
  int time = elapsed_time()-reset_time;
  sprintf(buffer,"S:%d", get_score() );
  draw_string(0,0, buffer, FG_COLOUR);
  sprintf(buffer,"L:%d",get_lives());
  draw_string(28, 0, buffer, FG_COLOUR);
  sprintf(buffer,"T:%d%d:%d%d",(time/60)/10 % 6,time/60 % 10,time/10 % 6 ,time % 10);
  draw_string(48,0,buffer, FG_COLOUR);
  // draw_int(10,10,objects_left(0),FG_COLOUR);// fragment count
  // draw_int(20,10,objects_left(1),FG_COLOUR);// fragment count
  // draw_int(30,10,objects_left(2),FG_COLOUR);// fragment count
}

//DRAW ALL FUNCTION
void draw_all(){
  clear_screen();
  draw_barrier();
  draw_starf();
  draw_turret();
  draw_plasmas();
  draw_objects();
  // show_screen();
}

// INITIALISE ALL
void init_all(){
  setup_starf();
  init_objects();
}

//PAUSE FUNC
void do_pause() {
  paused = !paused;
}

//SETUP
void setup(void) {
  setup_teensy();
  set_clock_speed(CPU_8MHz);
  adc_init();

  TC4H = OVERFLOW_TOP >> 8;
  OCR4C = OVERFLOW_TOP & 0xff;
  TCCR4A = BIT(COM4A1) | BIT(PWM4A);
  SET_BIT(DDRC,7);
  setup_pwm();
  lcd_init(LCD_DEFAULT_CONTRAST);
  start_time = elapsed_time();
  reset_time = start_time;
  paused=true;
  game_over=false;
  start_game=false;
  init_all();
  draw_all();
  reset_objects_left();
  set_duty_cycle((long) 0);
}

//RESET
void reset() {
  setup();
  reset_score();
  reset_lives();
  reset_plasma();
  reset_objects_left();
  CLEAR_BIT(DDRB, 2);CLEAR_BIT(DDRB,3);
  set_duty_cycle((long) 0);
  TC4H = OVERFLOW_TOP >> 8;

  OCR4C = OVERFLOW_TOP & 0xff;

  TCCR4A = BIT(COM4A1) | BIT(PWM4A);

  SET_BIT(DDRC,7);
}

//QUIT FUNCTION
void quit(){
  clear_screen();
  draw_string(15,20,"n10203478",FG_COLOUR);
  LCD_CMD(lcd_set_display_mode,lcd_display_inverse);
  game_over = true;
  show_screen();
}

//GAME OVER SCREEN
void game_over_screen(){
 clear_screen();
 set_duty_cycle((long)1023);
 draw_string(20,10,"Game over",FG_COLOUR);
 draw_string(20,20,"Reset: SW2",FG_COLOUR);
 draw_string(20,30,"Quit:  SW3",FG_COLOUR);
 if (BIT_IS_SET(PINF,6)) {
   set_duty_cycle((long) 0);
   reset();
 }
 if (BIT_IS_SET(PINF,5)) {
   quit();
 }
 set_duty_cycle((long)1023);
 SET_BIT(PORTB,2);SET_BIT(PORTB,3);
 show_screen();
}

// DO OPERATION
void do_operation(int16_t ch){
  if (BIT_IS_SET(PINF,6) || ch =='r') {
    reset();
  }
  else if (BIT_IS_SET(PINB,0) || ch =='p'){
    do_pause();
  }
  else if (BIT_IS_SET(PINB,7) || ch == 's') {
    show_status();
    show_screen();
  }
  else if ((BIT_IS_SET(PIND,1) || ch == 'w') && !paused){
    shoot_plasma();
  }
  else if ((BIT_IS_SET(PINB,1)|| ch == 'a') && !paused) {
    move_starf_left();
  }
  else if ((BIT_IS_SET(PIND,0) || ch == 'd') && !paused) {
    move_starf_right();
  }
	else if (ch == '?') {
		usb_serial_send("\n\nr: reset \n \rp: pause/unpause \n\rs:show status screen\n\rw: shoot plasma\n\ra: move starfighter left\n\rd: move starfighter right \n\r?: Show this screen \n\n\r" );
	}
  else if (ch == 'l') {
    set_lives();
  }
  else if (ch == 'g') {
    set_score();
  }
  // else if (ch == '?') {
  //   show_help();
  // }
  // else if (ch == 'h') {
  //   move_starf();
  // }
  // else if (ch == 'j') {
  //   add_asteroid();
  // }
  // else if (ch == 'k') {
  //   add_boulder();
  // }
  // else if (ch == 'i') {
  //   add_fragment();
  // }
  // else if (ch == 'm') {
  //   change_delay();
  // }
}

//LOOP
void process(void) {
  int16_t ch = usb_serial_getchar();
  do_operation(ch);
  // if ( ch >= ' ' || ch >= 0  ) {
  // snprintf(buffer, sizeof(buffer), "received '%c'\r\n", ch );
  // usb_serial_send( buffer );
  // }
  if (get_lives()<1) {
    SET_BIT(DDRB, 2);SET_BIT(DDRB,3);
    game_over_screen();
    paused = true;
    return;
  }
  else if (BIT_IS_SET(PINF,5) || game_over || ch =='q') {
    quit();
    return;
  }

  else if (!paused){
    update_starf();
    if (elapsed_time()>start_time+2 ) {
      update_objects();
    }
    check_collision_asteroid();
    check_collision_boulder();
    check_collision_fragment();
    update_plasmas();
  }
  else if (paused){
    show_status();
    show_screen();
  }
  draw_all();
  current_time = elapsed_time();
  show_screen();
}

// Program start and event loop.
int main(void) {
  srand(elapsed_time());
  setup_usb_serial();
	setup();
  while (!start_game) {
    intro_screen();
    int ch = usb_serial_getchar();
    if ( ch == 'r'){
      start_game = true;
    }
  }
	while ( !game_over) {
		process();
	}
return 0;
}
