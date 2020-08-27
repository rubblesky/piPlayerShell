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


/*指定目录*/
extern bool is_specify_directory;
/*使用收藏目录*/
extern bool use_star_dir;
/*显示隐藏文件*/
extern bool show_hidden_files;
typedef long int music_t;

enum player_status {
    PLAYING,
    STOP,
    PAUSE
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

/*播放列表信息*/
struct play_list_info {
    /*音乐文件夹名*/
    char *music_dir;
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

    /*当前选择的音乐*/
    music_t current_music;
};

extern struct play_list_info play_list;
#endif