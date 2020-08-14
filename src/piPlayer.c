
#include "piPlayer.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dir.c"
#include "sort.h"
#include "terminal.h"
#include "err.h"


//extern char **environ;

/*播放列表*/
struct play_list_info play_list;

/*程序运行目录名*/
static char *directory_name = "./";
/*指定目录*/
static bool is_specify_directory = 0;
/*使用收藏目录*/
static bool use_star_dir = 0;
/*显示隐藏文件*/
static bool show_hidden_files = 0;

/*绘制界面*/
static int print_interface();

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

    /*读取参数*/
    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opt, NULL)) != -1) {
        switch (opt) {
            case 0:
                break;
            case 'd':
                is_specify_directory = 1;
                directory_name = init_directory_name(optarg);

#ifndef NDEBUG
                printf("now dirctory is %s\n", directory_name);
#endif

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
    print_interface();
}

int print_interface() {
#ifndef NDEBUG
    printf("now  print_interface\n");
#endif

    play_list.file_alloc_num = 100;
    play_list.file_used_num = 0;
    if (get_file_info(directory_name) < 0) {
        return -1;
    }
    sort_file();

#ifndef NDEBUG
    /*打印排序结果*/
    if (play_list.sort_rule == by_last_modification_time)
        for (int i = 0; i < play_list.file_used_num; i++) {
            printf("%d: %25s  time: %d\n", i + 1, play_list.sorted_file[i]->name, play_list.sorted_file[i]->stat.st_mtime);
        }
    else if(play_list.sort_rule == by_name)
        for (int i = 0; i < play_list.file_used_num;i++){
            printf("%d: %s \n", i + 1, play_list.sorted_file[i]->name);
        }
#endif
    play_list.current_music = 0;
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
    return directory_name;
}
