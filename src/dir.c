#include "dir.h"

#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sort.h"


/*获取目录下文件信息*/
int get_file_info(char *directory_name);
/*向文件列表中添加读取到的文件*/
static int add_file(char *file_name);
/*文件排序*/
static void sort_file();
/*交换文件结构位置*/
void exchange_file(struct file_info **file, int pos1, int pos2);
/*按照文件名比较*/
int compare_by_filename(struct file_info **file, int pos1, int pos2);
/*按照最后修改日期比较*/
int compare_by_modification(struct file_info **file, int pos1, int pos2);
/*初始化目录*/
static char *init_directory_name(char *directory);



static int get_file_info(char *directory_name) {
#ifndef NDEBUG
    printf("now  get_file_info\n");
#endif
    DIR *dir = opendir(directory_name);
    struct dirent *next_file;

    /*第一次调用要给file开空间*/
    static bool first = true;

    errno = 0;
    if (NULL == dir) {
        file_error("open dir fail");
        return -1;
    }
    if (first) {
        play_list.file = malloc(play_list.file_alloc_num * sizeof(*play_list.file));
        play_list.sorted_file = malloc(play_list.file_alloc_num * sizeof(*play_list.sorted_file));
    }
    while (1) {
        errno = 0;
        if (NULL != (next_file = readdir(dir))) {
            add_file(next_file->d_name);
        } else if (errno != 0) {
            file_error("read dir fail");

        } else {
            break;
        }
    }
    closedir(dir);
}

static int add_file(char *file_name) {
    if (!show_hidden_files) {
        if (file_name[0] == '.') {
            return 0;
        }
    }
#ifndef NDEBUG
    printf("now  add_file\n");
#endif
    if (play_list.file_used_num == play_list.file_alloc_num) {
        play_list.file_alloc_num *= 2;
        play_list.file = realloc(play_list.file, play_list.file_alloc_num * sizeof(*play_list.file));
        play_list.sorted_file = realloc(play_list.sorted_file, play_list.file_alloc_num * sizeof(*play_list.file));
        if (NULL == play_list.file || NULL == play_list.sorted_file) {
            alloc_error();
            return -1;
        }
    }

    assert(strlen(directory_name) + strlen(file_name) + 1 <= PATHMAX);
    char *file_path = malloc(PATHMAX * sizeof(char *));
    int dir_size = strlen(directory_name);
    int name_size = strlen(file_name);
    memcpy(file_path, directory_name, dir_size);
    memcpy(file_path + dir_size, file_name, name_size + 1);

#ifndef NDEBUG
    printf("file_path is %s\n", file_path);
#endif

    struct file_info *f = &(play_list.file[play_list.file_used_num]);
    play_list.sorted_file[play_list.file_used_num++] = f;

    f->name = malloc(sizeof(char) * (name_size + 2));
    memcpy(f->name, file_name, name_size + 1);

#ifndef NDEBUG
    printf("file_name is %s\n", f->name);
#endif

    if (stat(file_path, &(f->stat)) < 0) {
        file_error("stat fail");
        free(file_path);
        return -1;
    } else {
        //f->mtime = f->stat->st_mtime;
        if (S_ISREG(f->stat.st_mode)) {
            f->file_type == normal;
        } else if (S_ISDIR(f->stat.st_mode)) {
            f->file_type == directory;
            f->name[name_size++] = PATH_SEPARATOR;
            f->name[name_size] = '\0';

#ifndef NDEBUG
            printf("dir_name is %s\n", f->name);
#endif
        } else {
            f->file_type == unknown;
            /*处理其他类型文件*/
        }
    }
#ifndef NDEBUG
    if (f->file_type == normal)
        printf("normal file\n");
    else if (f->file_type == directory)
        printf("directory \n");
#endif
    free(file_path);
    return 0;

    //f->name = malloc(name_size + 1);
}

static void sort_file() {
    switch (play_list.sort_rule) {
        case by_name:
            quick_sort(play_list.sorted_file, 0, play_list.file_used_num - 1, (void (*)(void *, int, int))compare_by_filename, (void (*)(void *, int, int))exchange_file);
            break;
        case by_last_modification_time:
            quick_sort(play_list.sorted_file, 0, play_list.file_used_num - 1, (void (*)(void *, int, int))compare_by_modification, (void (*)(void *, int, int))exchange_file);
            break;
        default:
            break;
    }
    return;
}

void exchange_file(struct file_info **file, int pos1, int pos2) {
    struct file_info *tmp = file[pos1];
    file[pos1] = file[pos2];
    file[pos2] = tmp;
}

int compare_by_filename(struct file_info **file, int pos1, int pos2) {
    char *n1 = file[pos1]->name;
    char *n2 = file[pos2]->name;
    int i = 0;
    while (n1[i] != '\0' && n2[i] != '\0') {
        if (n1[i] > n2[i]) {
            return 1;
        } else if (n1[i] < n2[i]) {
            return -1;
        }
        i++;
    }
    /*如果两个文件名前部分相同，较短的比较小，排在前面*/
    if (n1[i] == '\0' && n2[i] == '\0') {
        return 0;
    } else if (n1[i] != '\0') {
        return 1;
    } else {
        return -1;
    }
}

int compare_by_modification(struct file_info **file, int pos1, int pos2) {
    struct file_info *f1 = file[pos1];
    struct file_info *f2 = file[pos2];
    /*这里要再改回去试试*/
#ifdef _GNU_SOURCE
    if (f1->stat.st_mtim.tv_sec > f2->stat.st_mtim.tv_sec) {
        return 1;
    } else if (f1->stat.st_mtim.tv_sec == f2->stat.st_mtim.tv_sec) {
        return 0;
    } else {
        return -1;
    }

#else
    if (f1->stat.st_mtime > f2->stat.st_mtime) {
        return 1;
    } else if (f1->stat.st_mtime == f2->stat.st_mtime) {
        return 0;
    } else {
        return -1;
    }
#endif
}