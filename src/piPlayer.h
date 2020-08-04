#ifndef PIPLARYER_H
#define PIPLARYER_H

#include<stdbool.h>
#include<dirent.h>
#include<sys/stat.h>


typedef long int music_t;


struct file_info{
    char * name;
    struct stat stat;

};


/*播放列表信息*/
struct play_list_info
{
    //char *filename[NAME_MAX];
    //struct dirent *dirp;
    /*目录*/
    //DIR *dir;
    
    
    /*文件数组*/
    struct file_info *file;

    /*文件名数组的分配大小*/
    music_t filename_alloc_num;
    /*文件名数组有效数据大小*/
    music_t filename_used_num;
    
    /*当前选择的音乐*/
    music_t current_music;
};

#endif