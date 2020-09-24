#ifndef PIPLARYER_H
#define PIPLARYER_H

#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>

#include "config.h"

#define PATHMAX 1024
/*路径分隔符*/
#define PATH_SEPARATOR '/'

struct play_list_info *get_play_list();
/*获取playlist.file_list*/
struct file_list *get_file_list();
void exit_player(int stauts);

/*使用收藏目录*/
extern bool use_star_dir;
/*显示非音乐文件*/
extern bool show_all_files;
typedef long int music_t;

enum terminal_attr {
    WIDTH,
    NARROW
};

enum player_status {
    PLAYING,
    STOP,
    PAUSE
};

enum play_setting {
    SINGLE_PLAY,            /*单曲播放*/
    SINGLE_TUNE_CIRCULATION, /*单曲循环*/
    LOOP_PLAYBACK,           /*循环播放*/
    ORDER_PLAYBACK,          /*顺序播放*/
    RANDOM_PLAY              /*随机播放*/

};

enum file_type {
    unknown,
    fifo,
    chardev,
    directory, /*目录*/
    blockdev,
    normal, /*普通文件*/
    symbolic_link,
    sock,
    whiteout,
    arg_directory
};

struct file_info {
    char *name;
    struct stat stat;
    enum file_type file_type;
    //time_t mtime;
};

enum sort_rule {
    by_name,
    by_last_modification_time
};

struct file_list{
    /*文件数组*/
    struct file_info *file;
    /*排序方式*/
    enum sort_rule sort_rule;
    /*排序之后的文件数组*/
    struct file_info **sorted_file;
    /*文件名数组的分配大小*/
    music_t file_alloc_num;
    /*文件名数组有效数据大小
    相应下标应该-1
    */
    music_t file_used_num;
};


struct play_list_file{
    struct file_info **file;
    struct play_list_file *next_file;
};


struct player_status{

};

/*播放列表信息*/
struct play_list_info {
    /*音乐文件夹名*/
    char *music_dir;
    /*文件列表*/
    struct file_list file_list;

    /*播放列表*/
    struct play_list_file *play_list_file;
    /*列表中音乐数量*/
    music_t music_num;

    /*当前选择的音乐*/
    music_t current_choose;
    /*当前播放的音乐*/
    music_t current_playing;

};

//extern struct play_list_info play_list;

#endif