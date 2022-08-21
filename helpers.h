#pragma once

#include "lcd_model.h"
#include <lcd.h>
#include <graphics.h>
#include "ram_utils.h"
#include "usb_serial.h"

void send_str(const char *s);

uint8_t recv_str(char *buf, uint8_t size);

void parse_and_execute_command(const char *buf, uint8_t num);


void set_backlight_brightness(int how_bright);
void draw_pixels(int left, int top, int width, int height, char bitmap[]);
// void draw_pixels(int left, int top, int width, int height, uint8_t bitmap[], bool space_is_transparent);

void draw_double(uint8_t x, uint8_t y, double value, colour_t colour);
void draw_int(uint8_t x, uint8_t y, int value, colour_t colour);
void usb_serial_send(char*value);
void setup_usb_serial(void);

void adc_init();
uint16_t adc_read(uint8_t channel);

void draw_formatted(int x, int y, char * buffer, int buffer_size, const char * format, ...) ;
void send_formatted(char * buffer, int buffer_size, const char * format, ...);
void send_debug_string(char* string);
void setup_pwm();
