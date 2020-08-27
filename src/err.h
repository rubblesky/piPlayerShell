#ifndef ERR_H
#define ERR_H
#include <stdio.h>

#include "piPlayer.h"
/*文件错误*/
void file_error(char *msg);
/*内存分配错误*/
void alloc_error();

/*播放器错误*/
void player_error(FILE *out, char *msg);
#endif
