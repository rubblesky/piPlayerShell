#include"piPlayer.h"
#include"terminal.h"
#include"sort.h"


#include <unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<getopt.h>
#include<errno.h>
#include<assert.h>
#include<dirent.h>
#include<stdbool.h>


#define PATHMAX 256
/*路径分隔符*/
#define PATH_SEPARATOR '/'


//extern char **environ;

/*播放列表*/
struct play_list_info play_list;

/*程序运行目录名*/
static char *directory_name;
/*指定目录*/
static bool is_specify_directory;
/*使用收藏目录*/
static bool use_star_dir;

/*绘制界面*/
static int print_interface();



/*获取目录下文件信息*/
static int get_file_info();
/*向文件列表中添加读取到的文件*/
static int add_file(char *file_name);
/*文件排序*/
static void sort_file();
/*交换文件结构位置*/
void  exchange_file(struct file_info ** file,int pos1,int pos2);
/*按照文件名比较*/
int compare_by_filename(struct file_info ** file, int pos1, int pos2);
/*按照最后修改日期比较*/
int compare_by_modification(struct file_info **file, int pos1, int pos2);
/*初始化目录*/
static char *init_directory_name(char *directory);

/*文件错误*/
static void file_error();
/*内存分配错误*/
static void alloc_error();


static char short_opts[] = "sd:";
static struct option long_opt[] =
    {
        {"directory", required_argument, NULL, 'd'},
        {0, 0, 0, 0}};

int main(int argc, char *argv[])
{
    int opt;
    while((opt = getopt_long(argc,argv,short_opts,long_opt,NULL))!= -1){
        switch (opt)
        {
        case 0:
            break;
        case 'd':
            is_specify_directory = 1;
            directory_name = init_directory_name(optarg);
            break;
        case 's':
            use_star_dir = 1;
            break;
        default:
            break;
        }
    }

}


int print_interface(){
    play_list.file_alloc_num = 100;
    play_list.file_used_num = 0;
    if (get_file_info() < 0){
        return -1;
    }
    sort_file();
    play_list.current_music = 0;

}

static int get_file_info(){
    DIR *dir = opendir(directory_name);
    struct dirent *next_file;

    errno = 0;
    if(NULL == dir){
        file_error();
        return -1;
    }
    play_list.file = malloc(play_list.file_alloc_num * sizeof(*play_list.file));
    while (1)
    {
        errno = 0;
        if(NULL != (next_file = readdir(dir))){
            add_file(next_file->d_name);
        }
        else if(errno != 0){
            file_error();
        }
        else{
            break;
        }
    }
    closedir(dir);
}




static void file_error(){

}

static void alloc_error(){

}

static int add_file(char *file_name){
    if(play_list.file_used_num == play_list.file_alloc_num){
        play_list.file_alloc_num *= 2;
        play_list.file = realloc(play_list.file , play_list.file_alloc_num * sizeof(*play_list.file));
        play_list.sorted_file = realloc(play_list.sorted_file, play_list.file_alloc_num * sizeof(*play_list.file));
        if (NULL == play_list.file || NULL == play_list.sorted_file)
        {
            alloc_error();
            return -1;
        }
    }
    assert(strlen(directory_name) + strlen(file_name) + 1 <= PATHMAX);
    char *file_path = malloc(PATHMAX * sizeof(char*));
    int dir_size = strlen(directory_name);
    int name_size = strlen(file_name);
    memcpy(file_path, directory_name, dir_size);
    memcpy(file_path + dir_size, file_name, name_size + 1);

    struct file_info *f = &(play_list.file[play_list.file_used_num-1]);
    play_list.sorted_file[play_list.file_used_num++] = f;

    if (0 > stat(file_path, f->stat))
    {
        file_error();
        return -1;
    }
    else{

        //f->mtime = f->stat->st_mtime;
        if (S_ISREG(f->stat->st_mode))
        {
            f->file_type == normal;
        }
        else if(S_ISDIR(f->stat->st_mode)){
            f->file_type == directory;
        }
        else{
            f->file_type == unknown;
            /*处理其他类型文件*/
        }
    }
    return 0;

    //f->name = malloc(name_size + 1);
}

static void sort_file(){
    switch (play_list.sort_rule)
    {
    case by_name:
        quick_sort(play_list.sorted_file, 0, play_list.file_used_num-1, (void(*)(void *,int,int))compare_by_filename, (void(*)(void *,int,int))exchange_file);
        break;
    case by_last_modification_time:
        quick_sort(play_list.sorted_file, 0, play_list.file_used_num-1, (void(*)(void *,int,int))compare_by_modification, (void(*)(void *,int,int))exchange_file);
        break;
    default:
        break;
    }
    return;
}

void  exchange_file(struct file_info ** file,int pos1,int pos2){
    struct file_info *tmp = file[pos1];
    file[pos1] = file[pos2];
    file[pos2] = tmp;
}

int compare_by_filename(struct file_info **file, int pos1, int pos2){

}

int compare_by_modification(struct file_info **file, int pos1, int pos2){
    struct file_info *f1 = file[pos1];
    struct file_info *f2 = file[pos2];
    if(f1->stat->st_mtime > f2->stat->st_mtime){
        return 1;
    }
    else if(f1->stat->st_mtime == f2->stat->st_mtime){
        return 0;
    }
    else{
        return -1;
    }
}

static char * init_directory_name(char * directory){
    int size = strlen(directory);
    assert(size > 0);
    char *directory_name = malloc(size + 2);
    if (NULL == directory_name){
        alloc_error();
        return NULL;
    }

    memcpy(directory_name, directory ,size+1);
    if(directory_name[size-1] != PATH_SEPARATOR){
        directory_name[size++] = PATH_SEPARATOR;
        directory_name[size] = '\0';
    }
    return directory_name;
}


