#include "err.h"
#include <string.h>
#include <errno.h>

void file_error(char *msg) {
    fprintf(stderr, "file error:  ");
    perror(msg);
}

void alloc_error() {
    fprintf(stderr, "alloc error\n");
}
/*播放器的标准输出和错误输出都被丢弃
  所以需要指定错误的输出流
*/
void player_error(FILE * out,char *msg){
    fprintf(out,"player error : %s  %s",msg,strerror(errno));
    perror(msg);
}
void sys_error(char *msg){
    perror(msg);
}
