#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
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
#include "usb_serial.h"

#define OVERFLOW_TOP (1023)
#define ADC_MAX (1023)
#define BIT(x) (1 << (x))

char buffer[20];
#define BUFF_LENGTH 20
//=======================
// HELPER   FUNCTIONS
//====================

void send_str(const char *s)
{
	char c;
	while (1) {
		c = pgm_read_byte(s++);
		if (!c) break;
		usb_serial_putchar(c);
	}
}

// Receive a string from the USB serial port.  The string is stored
// in the buffer and this function will not exceed the buffer size.
// A carriage return or newline completes the string, and is not
// stored into the buffer.
// The return value is the number of characters received, or 255 if
// the virtual serial connection was closed while waiting.
//
uint8_t recv_str(char *buf, uint8_t size)
{
	int16_t r;
	uint8_t count=0;

	while (count < size) {
		r = usb_serial_getchar();
		if (r != -1) {
			if (r == '\r' || r == '\n') return count;
			if (r >= ' ' && r <= '~') {
				*buf++ = r;
				usb_serial_putchar(r);
				count++;
			}
		} else {
			if (!usb_configured() ||
			  !(usb_serial_get_control() & USB_SERIAL_DTR)) {
				// user no longer connected
				return 255;
			}
			// just a normal timeout, keep waiting
		}
	}
	return count;
}

// parse a user command and execute it, or print an error message
//
void parse_and_execute_command(const char *buf, uint8_t num)
{
	uint8_t port, pin, val;

	if (num < 3) {
		send_str(PSTR("unrecognized format, 3 chars min req'd\r\n"));
		return;
	}
	// first character is the port letter
	if (buf[0] >= 'A' && buf[0] <= 'F') {
		port = buf[0] - 'A';
	} else if (buf[0] >= 'a' && buf[0] <= 'f') {
		port = buf[0] - 'a';
	} else {
		send_str(PSTR("Unknown port \""));
		usb_serial_putchar(buf[0]);
		send_str(PSTR("\", must be A - F\r\n"));
		return;
	}
	// second character is the pin number
	if (buf[1] >= '0' && buf[1] <= '7') {
		pin = buf[1] - '0';
	} else {
		send_str(PSTR("Unknown pin \""));
		usb_serial_putchar(buf[0]);
		send_str(PSTR("\", must be 0 to 7\r\n"));
		return;
	}
	// if the third character is a question mark, read the pin
	if (buf[2] == '?') {
		// make the pin an input
		*(uint8_t *)(0x21 + port * 3) &= ~(1 << pin);
		// read the pin
		val = *(uint8_t *)(0x20 + port * 3) & (1 << pin);
		usb_serial_putchar(val ? '1' : '0');
		send_str(PSTR("\r\n"));
		return;
	}
	// if the third character is an equals sign, write the pin
	if (num >= 4 && buf[2] == '=') {
		if (buf[3] == '0') {
			// make the pin an output
			*(uint8_t *)(0x21 + port * 3) |= (1 << pin);
			// drive it low
			*(uint8_t *)(0x22 + port * 3) &= ~(1 << pin);
			return;
		} else if (buf[3] == '1') {
			// make the pin an output
			*(uint8_t *)(0x21 + port * 3) |= (1 << pin);
			// drive it high
			*(uint8_t *)(0x22 + port * 3) |= (1 << pin);
			return;
		} else {
			send_str(PSTR("Unknown value \""));
			usb_serial_putchar(buf[3]);
			send_str(PSTR("\", must be 0 or 1\r\n"));
			return;
		}
	}
	// otherwise, error message
	send_str(PSTR("Unknown command \""));
	usb_serial_putchar(buf[0]);
	send_str(PSTR("\", must be ? or =\r\n"));
}


void set_backlight_brightness(int how_bright){
	TC4H = how_bright >> 8;
	OCR4A = how_bright & 0xff;

}

void setup_pwm(){
	TC4H = OVERFLOW_TOP >> 8;
	OCR4C = OVERFLOW_TOP & 0xff;
	// (b)	Use OC4A for PWM. Remember to set DDR for C7.
	TCCR4A = BIT(COM4A1) | BIT(PWM4A);
	SET_BIT(DDRC, 7);
	// (c)	Set pre-scale to "no pre-scale"
	TCCR4B = BIT(CS42) | BIT(CS41) | BIT(CS40);

	TCCR4D = 0;
}

void draw_formatted(int x, int y, char * buffer, int buffer_size, const char * format, ...) {
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, buffer_size, format, args);
	draw_string(x, y, buffer, FG_COLOUR);
}

void send_formatted(char * buffer, int buffer_size, const char * format, ...) {
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, buffer_size, format, args);
	usb_serial_write((uint8_t *) buffer, strlen(buffer));
}

void draw_centred(unsigned char y, char * string) {
    unsigned char l = 0, i = 0;
    while (string[i] != '\0') {
        l++;
        i++;
    }
    char x = 42-(l*5/2);
    draw_string((x > 0) ? x : 0, y, string, FG_COLOUR);
}

void send_line(char* string) {
    unsigned char char_count = 0;
    while (*string != '\0') {
        usb_serial_putchar(*string);
        string++;
        char_count++;
    }
    // Start a new line
    usb_serial_putchar('\r');
    usb_serial_putchar('\n');
}

// USB Serial
void send_debug_string(char* string) {
     // Send the debug preamble...
    //char timebuff[80];
    //sprintf(timebuff,"[DEBUG LOG @ %6.3f] ",get_system_time());
    //usb_serial_write(timebuff , 17);
    // Send all of the characters in the string
    unsigned char char_count = 0;
    while (*string != '\0') {
        usb_serial_putchar(*string);
        string++;
        char_count++;
    }
    // Start a new line
    usb_serial_putchar('\r');
    usb_serial_putchar('\n');
 }

void adc_init() {
	// ADC Enable and pre-scaler of 128: ref table 24-5 in datasheet
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(uint8_t channel) {
	// Select AVcc voltage reference and pin combination.
	// Low 5 bits of channel spec go in ADMUX(MUX4:0)
	// 5th bit of channel spec goes in ADCSRB(MUX5).
	ADMUX = (channel & ((1 << 5) - 1)) | (1 << REFS0);
	ADCSRB = (channel & (1 << 5));
	// Start single conversion by setting ADSC bit in ADCSRA
	ADCSRA |= (1 << ADSC);
	// Wait for ADSC bit to clear, signalling conversion complete.
	while ( ADCSRA & (1 << ADSC) ) {}
	// Result now available.
	return ADC;
}

void draw_pixels(int left, int top, int width, int height, char bitmap[]){
    for (int j=0; j<height; j++){
        for(int i=0; i<width; i++){
            if (bitmap[i + j * width] != ' '){
                draw_pixel(left + i, top + j, FG_COLOUR);
            }
        }
    }
}

void usb_serial_send(char*value){
	unsigned char count = 0;
	while(*value != '\0'){
		usb_serial_putchar(*value);
		value++;
		count++;
	}
	usb_serial_putchar('\r');
	usb_serial_putchar('\n');
}

void setup_usb_serial(void) {
	// Set up LCD and display message
	lcd_init(LCD_DEFAULT_CONTRAST);
	draw_string(10, 10, "Connect USB...", FG_COLOUR);
	show_screen();
	usb_init();
	while ( !usb_configured() ) {		// Block until USB is ready.
	}
	clear_screen();
	draw_string(10, 10, "USB connected", FG_COLOUR);
	// SEND WELCOME MESSAGE
	usb_serial_send( "GAME READY!" );
	usb_serial_send( "\r\n" );
	show_screen();
}

//================================================
void draw_double(uint8_t x, uint8_t y, double value, colour_t colour) {
	snprintf(buffer, sizeof(buffer), "%f", value);
	draw_string(x, y, buffer, colour);
}

void draw_int(uint8_t x, uint8_t y, int value, colour_t colour) {
	snprintf(buffer, sizeof(buffer), "%d", value);
	draw_string(x, y, buffer, colour);
}
