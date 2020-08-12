#ifndef PIPLARYER_H
#define PIPLARYER_H

#include<stdbool.h>
#include<dirent.h>
#include<time.h>
#include<sys/stat.h>


typedef long int music_t;


enum file_type
  {
    unknown,
    fifo,
    chardev,
    directory,          /*目录*/
    blockdev,
    normal,             /*普通文件*/
    symbolic_link,
    sock,
    whiteout,
    arg_directory
  };


struct file_info{
    char * name;
    struct stat stat;
    enum file_type file_type;
    //time_t mtime;
};

enum sort_rule{
    by_name,
    by_last_modification_time
};

/*播放列表信息*/
struct play_list_info
{
    
    /*文件数组*/
    struct file_info *file;
    /*排序方式*/
    enum sort_rule sort_rule;
    /*排序之后的文件数组*/
    struct file_info **sorted_file;

    /*文件名数组的分配大小*/
    music_t file_alloc_num;
    /*文件名数组有效数据大小*/
    music_t file_used_num;
    
    /*当前选择的音乐*/
    music_t current_music;
};

#endif