#include "piPlayer.h"

/*获取目录下文件信息*/
int get_file_info(char *directory_name);
/*文件排序*/
void sort_file();
/*交换文件结构位置*/
void exchange_file(struct file_info **file, int pos1, int pos2);
/*按照文件名比较*/
int compare_by_filename(struct file_info **file, int pos1, int pos2);
/*按照最后修改日期比较*/
int compare_by_modification(struct file_info **file, int pos1, int pos2);
