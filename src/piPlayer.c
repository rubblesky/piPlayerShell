
#include "piPlayer.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <locale.h>
#include <ncursesw/ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#include <termios.h>
#include <unistd.h>
//#include <ncurses.h>

#include "dir.h"
#include "err.h"
#include "interface.h"
#include "cmd.h"
#include "sort.h"
#include "terminal.h"
//extern char **environ;

/*播放列表*/
struct play_info player;
/*歌曲结束*/
bool is_end;

/*使用收藏目录*/
bool use_star_dir = 0;
/*显示非音乐文件*/
bool show_all_files = 0;
/*初始终端设置
  作为程序结束时恢复终端属性的依据
*/


struct play_info *get_player() {
    return &player;
}
struct file_list *get_file_list() {
    return &(player.file_list);
}

struct play_list *get_play_list() {
    return &(player.play_list);
}
void init_player();
/*初始化播放列表相关默认值*/
void init_play_list();

/*切换目录*/
static int change_dirctory(char *dir);






/*获取当前音乐目录*/
char *get_current_dir();

void exit_player(int stauts);


static char short_opts[] = "astd:";
static struct option long_opt[] =
    {
        {"directory", required_argument, NULL, 'd'},
        {0, 0, 0, 0}};

int main(int argc, char *argv[]) {
    /*初始化默认值*/
    init_player();
    is_end = 1;
    /*读取参数*/
    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opt, NULL)) != -1) {
        switch (opt) {
            case 0:
                break;
            case 'a':
                show_all_files = true;
                break;
            case 'd':
                if (change_dirctory(optarg) < 0) {
                    exit_player(-1);
                }
                break;
            case 's':
                use_star_dir = 1;
                break;
            case 't':
                player.file_list.sort_rule = by_last_modification_time;
                break;
            default:
                break;
        }
    }
    /*收到SIGINT信号时执行默认退出程序*/
    signal(SIGINT, exit_player); /*这里还要追加补充其他信号*/
    signal(SIGWINCH, print_by_size);
    pthread_t tidp;
    if (read_dir(player.file_list.music_dir) < 0) {
        return -1;
    } else {
        init_play_list();
    }
    print_interface();
    if (pthread_create(&tidp, NULL, control, NULL) < 0) {
#ifndef NDEBUG
        printf("can't create control pthread\n");
#endif
        return -1;
    }
    while (1) {
        sleep(10);
    }
}

void init_player() {
    struct file_list *file_list = &(player.file_list);
    struct play_list *play_list = &(player.play_list);
    player.file_list.music_dir = get_current_dir();

    file_list->sort_rule = by_name;
    file_list->file_alloc_num = 100;
    file_list->file_used_num = 0;

    play_list->play_setting = SINGLE_PLAY;
    play_list->player_status = STOP;
}
static int change_dirctory(char *dir) {
    if (chdir(dir) < 0) {
        file_error("can't change directory");
        return -1;
    }
    if (player.file_list.music_dir != NULL) {
        free(player.file_list.music_dir);
    }
    player.file_list.music_dir = get_current_dir();
    if (player.file_list.music_dir != NULL) {
        return 0;
    } else {
        return -1;
    }
}

void init_play_list() {
    struct file_list *fl = get_file_list();
    struct play_list *pl = get_play_list();
    //pl->music_num = fl->file_used_num;
    pl->play_list_file = init_list();
    for (int i = 0; i < fl->file_used_num;i++){
        if(list_append(pl->play_list_file, fl->sorted_file[i]) == ERROR_ALLOC){
            alloc_error();
            exit_player(-1);
        }
    }
    pl->current_choose = list_get_first(pl->play_list_file);
    pl->current_playing = NULL;
}

char *get_current_dir() {
    char *dir = malloc(PATHMAX);
    if (dir == NULL) {
        alloc_error();
        return NULL;
    }
    if (getcwd(dir, PATHMAX) == NULL) {
        file_error("can't get pwd");
        return NULL;
    }
    int size = strlen(dir);
    dir[size++] = '/';
    dir[size] = '\0';
    return dir;
}

void exit_player(int stauts) {
    /*向所有子进程发送终止信号*/
    kill(0, SIGINT);

    reset_tty_attr();
    endwin();
    exit(stauts);
}
