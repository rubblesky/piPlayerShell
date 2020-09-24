#ifndef INTERFACE_H
#define INTERFACE_H
#include "piPlayer.h"
int print_interface();
void reset_tty_attr();
void print_by_size();
void print_list(struct play_list *play_list);
void print_menu(char *msg, ...);
#endif