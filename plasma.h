#pragma once

void draw_plasma(int mx, int my);
void draw_plasmas();

double get_px(int i);
double get_py(int i);
int get_max_plasma();
char * char_plasma();
void plasma_collide(int i);

void reset_plasma();

void shoot_plasma();
void update_plasmas();
double game_time();
int get_plasma_count();
