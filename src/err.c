#include "err.h"
#include <stdio.h>
void file_error(char *msg) {
    fprintf(stderr, "file error:  ");
    perror(msg);
}

void alloc_error() {
    fprintf(stderr, "alloc error\n");
}

void player_error(char *msg){

    perror(msg);
}
