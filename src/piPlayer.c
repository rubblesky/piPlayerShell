
#include "piPlayer.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "dir.h"
#include "err.h"
#include "sort.h"
#include "terminal.h"

//extern char **environ;

/*播放列表*/
struct play_list_info play_list;

/*指定目录*/
bool is_specify_directory = 0;
/*使用收藏目录*/
bool use_star_dir = 0;
/*显示隐藏文件*/
bool show_hidden_files = 0;

/*绘制界面*/
static int print_interface();
/*读取键盘命令*/
static void *get_cmd(void *arg);
/*播放音乐*/
static int play();
/*获取当前音乐目录*/
char *get_current_dir();

/*打印目录*/
static void print_dir();

/*初始化目录*/
static char *init_directory_name(char *directory);

static char short_opts[] = "std:";
static struct option long_opt[] =
    {
        {"directory", required_argument, NULL, 'd'},
        {0, 0, 0, 0}};

int main(int argc, char *argv[]) {
    /*初始化默认值*/
    play_list.sort_rule = by_name;
    play_list.music_dir = get_current_dir();
    /*读取参数*/
    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opt, NULL)) != -1) {
        switch (opt) {
            case 0:
                break;
            case 'd':
                is_specify_directory = 1;
                //play_list.music_dir = init_directory_name(optarg);

#ifndef NDEBUG
                printf("now pwd is %s\n", getenv("PWD"));
#endif

                if(chdir(optarg) < 0){
                    file_error("can't change directory");
                }
                if(play_list.music_dir!=NULL)
                    free(play_list.music_dir);
                play_list.music_dir = get_current_dir();
                break;
            case 's':
                use_star_dir = 1;
                break;
            case 't':
                play_list.sort_rule = by_last_modification_time;
                break;
            default:
                break;
        }
    }
    pthread_t tidp;
    if (pthread_create(&tidp, NULL, get_cmd, NULL) < 0) {
#ifndef NDEBUG
        printf("can't create control pthread\n");
#endif
        return -1;
    }
    print_interface();
}

int print_interface() {
#ifndef NDEBUG
    printf("now  print_interface\n");
#endif

    play_list.file_alloc_num = 100;
    play_list.file_used_num = 0;
    if (read_dir(play_list.music_dir) < 0) {
        return -1;
    }

#ifndef NDEBUG
    /*打印排序结果*/
    if (play_list.sort_rule == by_last_modification_time)
        for (int i = 0; i < play_list.file_used_num; i++) {
            printf("%d: %25s  time: %d\n", i + 1, play_list.sorted_file[i]->name, play_list.sorted_file[i]->stat.st_mtime);
        }
    else if (play_list.sort_rule == by_name)
        for (int i = 0; i < play_list.file_used_num; i++) {
            printf("%d: %s \n", i + 1, play_list.sorted_file[i]->name);
        }
#endif
    play_list.current_music = 0;
    while (1) {
    }
}

static void *get_cmd(void *arg) {
    char c;
    /*
    if (setvbuf(stdin,NULL,_IONBF,0) != 0) {
#ifndef NDEBUG
        printf("buf modify error\n");
#endif

    }
*/
    struct termios old_tty_attr, new_tty_attr;
    if (tcgetattr(fileno(stdin), &old_tty_attr) < 0) {
#ifndef NDEBUG
        printf("can't get tty attribute\n");
#endif
        exit(-1);
    }
    new_tty_attr = old_tty_attr;
    /*修改模式为非规范模式*/
    new_tty_attr.c_lflag &= ~ICANON;
    new_tty_attr.c_lflag &= ~ECHO;
    if (tcsetattr(fileno(stdin), TCSADRAIN, &new_tty_attr) < 0) {
#ifndef NDEBUG
        printf("can't set tty attribute\n");
#endif
        exit(-1);
    }
    char play_cmd[128] = "mplayer ";
    while ((c = fgetc(stdin)) != EOF) {
        switch (c) {
            case 'u':
                play_list.current_music--;
                break;
            case 'd':
                play_list.current_music++;
                break;
            case 'b':

                strcat(play_cmd, play_list.sorted_file[play_list.current_music]->name);
#ifndef NDBUG
                printf("now playing: %s \n id = %d\n", play_cmd, play_list.current_music);
                printf("song : %s\n", play_list.sorted_file[play_list.current_music]->name);
#endif
                system(play_cmd);
                break;
            case 'q':
                tcsetattr(fileno(stdin), TCSADRAIN, &old_tty_attr);
                exit(0);
            default:
                break;
        }
    }
}

static int play() {
    pid_t pid;
    if ((pid = fork()) < 0) {
        printf("play error\n");
        return -1;
    } else if (pid == 0) {
        return 0;

    } else {
        if (execlp("mplayer", play_list.sorted_file[play_list.current_music]->name) < 0) {
            printf("play fail\n");
            return -1;
        }
        return 0;
    }
}

char *get_current_dir(){

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

static int change_dir(char *dir) {
    char *pwd = getenv("PWD");
    if (pwd == NULL) {
        pwd = malloc(128);
        if (pwd == NULL) {
            alloc_error();
            return -1;
        }
        memcpy(pwd, "name=", 6);
    }
    int len = strlen(dir) + 1;
    if (strlen(pwd) < len + 5) {
        /*多分配一点内存*/
        if ((realloc(pwd, len + 64)) == NULL) {
            alloc_error();
            return -1;
        }
    }
    memcpy(pwd + 5, dir, len);
    if (putenv(pwd) != 0) {
        return -1;
    }
    return 0;
}

static void print_dir() {
    for (int i = 0; i < play_list.file_used_num; i++) {
        printf("%s\n", play_list.file->name);
    }
}

static char *init_directory_name(char *directory) {
    int size = strlen(directory);
    assert(size > 0);
    char *directory_name = malloc(size + 2);
    if (NULL == directory_name) {
        alloc_error();
        return NULL;
    }

    memcpy(directory_name, directory, size + 1);
    if (directory_name[size - 1] != PATH_SEPARATOR) {
        directory_name[size++] = PATH_SEPARATOR;
        directory_name[size] = '\0';
    }
    /*如果是绝对路径*/
    if(directory[0] == '/'){
        setenv("PWD", directory, 1);
        return directory;
    }
    else{
        int size = strlen(play_list.music_dir);
       
        char *tmp = malloc(+size + 64);
        memcpy(tmp, "PWD=", 5);
        strcat(tmp, play_list.music_dir);
    }

    return directory_name;
}
