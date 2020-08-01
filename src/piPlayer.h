#ifndef PIPLARYER_H
#define PIPLARYER_H

#include<stdbool.h>


#define XTERN extern


/*使用收藏目录*/
XTERN bool use_star_dir;
/*指定目录*/
XTERN bool is_specify_directory;
/*程序运行目录*/
XTERN char* directory;
#endif