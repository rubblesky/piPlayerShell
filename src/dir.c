#include "dir.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "err.h"
#include "sort.h"

char *music_file_suffix[] = {"acc", "ACC", "mp3", "MP3", "wav", "WAV"};
static unsigned char const prime_offset[] =
    {
        0, 0, 1, 1, 3, 1, 3, 1, 5, 3, 3, 9, 3, 1, 3, 19, 15, 1, 5, 1, 3, 9, 3,
        15, 3, 39, 5, 39, 57, 3, 35, 1, 5, 9, 41, 31, 5, 25, 45, 7, 87, 21,
        11, 57, 17, 55, 21, 115, 59, 81, 27, 129, 47, 111, 33, 55, 5, 13, 27,
        55, 93, 1, 57, 25};
const int suffix_size = sizeof(music_file_suffix) / sizeof(char *);

struct hash_table {
    long (*eqs)[2];
    int *buckets;
    int nbuckets;
};

/*获取目录下文件信息*/
static int
get_file_info(char *directory_name);
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
/*hash文件后缀*/
long hash_suffix(char *filename);
/*获得文件目录hash表*/
struct hash_table *get_hash_table();
void free_hash_tbale(struct hash_table *hash_table);
/*是否为音乐文件*/
static bool is_music(char *filename, struct hash_table *hash_table);

int read_dir(char *directory_name) {
    if (get_file_info(directory_name) < 0) {
        return -1;
    }
    sort_file();
}

static int get_file_info(char *directory_name) {
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
    struct hash_table *hash_table;
    if (!show_all_files) {
        hash_table  = get_hash_table();
    }
    while (1) {
        errno = 0;
        if (NULL != (next_file = readdir(dir))) {
            if(!show_all_files){
                if(is_music(next_file->d_name,hash_table)){
                    add_file(next_file->d_name);
                }
                else{
                    continue;
                }
            }
            else{
                add_file(next_file->d_name);
            }
        } else if (errno != 0) {
            file_error("read dir fail");
            return -1;
        } else {
            break;
        }
    }
    free_hash_tbale(hash_table);
    closedir(dir);
}

static int add_file(char *file_name) {
    if (strcmp(file_name, ".") == 0) {
        return 0;
    }
    if (play_list.file_used_num == play_list.file_alloc_num) {
        play_list.file_alloc_num *= 2;
        play_list.file = realloc(play_list.file, play_list.file_alloc_num * sizeof(*play_list.file));
        play_list.sorted_file = realloc(play_list.sorted_file, play_list.file_alloc_num * sizeof(*play_list.file));
        if (NULL == play_list.file || NULL == play_list.sorted_file) {
            alloc_error();
            return -1;
        }
    }

    assert(strlen(play_list.music_dir) + strlen(file_name) + 1 <= PATHMAX);
    char *file_path = malloc(PATHMAX * sizeof(char *));
    int dir_size = strlen(play_list.music_dir);
    int name_size = strlen(file_name);
    memcpy(file_path, play_list.music_dir, dir_size);
    memcpy(file_path + dir_size, file_name, name_size + 1);

    struct file_info *f = &(play_list.file[play_list.file_used_num]);
    play_list.sorted_file[play_list.file_used_num++] = f;

    f->name = malloc(sizeof(char) * (name_size + 2));
    memcpy(f->name, file_name, name_size + 1);

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
        } else {
            f->file_type == unknown;
            /*处理其他类型文件*/
        }
    }
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
    /*
    让修改时间晚的在前面
    大数在前
    */
    struct file_info *f1 = file[pos1];
    struct file_info *f2 = file[pos2];
#ifdef _GNU_SOURCE
    if (f1->stat.st_mtim.tv_sec > f2->stat.st_mtim.tv_sec) {
        return -1;
    } else if (f1->stat.st_mtim.tv_sec == f2->stat.st_mtim.tv_sec) {
        return 0;
    } else {
        return 1;
    }

#else
    if (f1->stat.st_mtime > f2->stat.st_mtime) {
        return -1;
    } else if (f1->stat.st_mtime == f2->stat.st_mtime) {
        return 0;
    } else {
        return 1;
    }
#endif
}
long hash_suffix (char *filename) {
    long hash = 0;
    for (int i = strlen(filename)-1; filename[i] != '.' && i >= 0; i--) {
        hash += filename[i];
        hash <<= 7;
    }
    return hash;
}
struct hash_table *get_hash_table() {
    int i;
    for (i = 2; (size_t)1 << i < suffix_size / 3; i++)
        continue;
    int nbuckets = ((size_t)1 << i) - prime_offset[i];
    int *buckets = calloc(nbuckets,sizeof(int));
    if (buckets == NULL) {
        alloc_error();
        exit(-1); 
    }
    long(*eqs)[2] = calloc(suffix_size + 1, sizeof(eqs[2]));
    if (eqs == NULL) {
        alloc_error();
        exit(-1);
    }
    long hash;
    int n = 1;
    for (int s = 0; s < suffix_size; s++) {
        hash = hash_suffix(music_file_suffix[s]);
        int b = hash % nbuckets;
        if (buckets[b] == 0) {
            eqs[n][1] = hash;
            buckets[b] = n;
            n++;
        } else {
            eqs[n][1] = hash;
            eqs[n][0] = buckets[b];
            buckets[b] = n;
            n++;
        }
    }
    struct hash_table *hash_table = malloc(sizeof(*hash_table));
    if(hash_table == NULL){
        alloc_error();
        exit(-1);
    }
    hash_table->buckets = buckets;
    hash_table->eqs = eqs;
    hash_table->nbuckets = nbuckets;
    return hash_table;
}
void free_hash_tbale(struct hash_table *hash_table) {
    free(hash_table->buckets);
    free(hash_table->eqs);
    free(hash_table);
}
static bool is_music(char *filename, struct hash_table *hash_table) {
    long hash = hash_suffix(filename);
    int i = hash_table->buckets[hash % hash_table->nbuckets];
    do {
        if(hash_table->eqs[i][1] == hash){
            /*hash值相同基本可以确定相同了
              这里不再进行字符串比对了
            */
            return true;
        }
        i = hash_table->eqs[i][0];
    } while (hash_table->eqs[i][0] != 0);
    return false;
}