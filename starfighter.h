#pragma once

void setup_starf(void);
void draw_starf(void);
void draw_turret();
void erase_starf(void);
void update_starf();

int get_sx();
int get_sy();
int get_sw();
int get_sh();
char * char_starf();

void move_starf_left();
void move_starf_right();
