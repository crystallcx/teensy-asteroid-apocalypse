#pragma once
#include <stdbool.h>
void init_objects();
void update_objects();
void draw_objects();
void do_collided_object(int object_type, int i);
int get_ax(int i);
int get_ay(int i);
int get_bx(int i);
int get_by(int i);
int get_fx(int i);
int get_fy(int i);
int get_max_asteroid();
int get_max_boulder();
int get_max_fragment();
int get_score();
int get_lives();
void reset_score();
void reset_lives();
void set_score();
void set_lives();
int objects_left(int type);
void reset_objects_left();
