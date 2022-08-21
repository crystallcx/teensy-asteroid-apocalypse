#pragma once

bool collision(int x0, int y0, int w0, int h0, char pixels0[], int x1, int y1, int w1, int h1, char pixels1[]);
void check_collision_asteroid();
void check_collision_boulder();
void check_collision_fragment();
char * char_asteroid();
char * char_boulder();
char * char_fragment();
char * char_plasma();
