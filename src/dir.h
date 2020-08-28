#ifndef DIR_H
#define DIR_H
#include "piPlayer.h"

/*主要函数，读取文件目录并排序*/
int read_dir(char *directory_name);
/*获取目录下文件信息*/
static int get_file_info(char *directory_name);
/*文件排序*/
static void sort_file();
/*交换文件结构位置*/
void exchange_file(struct file_info **file, int pos1, int pos2);
/*按照文件名比较*/
int compare_by_filename(struct file_info **file, int pos1, int pos2);
/*按照最后修改日期比较*/
int compare_by_modification(struct file_info **file, int pos1, int pos2);


#endif