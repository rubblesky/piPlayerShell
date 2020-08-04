#include"piPlayer.h"
#include"terminal.h"

#include <unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>
#include<errno.h>
#include<dirent.h>
#include<stdbool.h>


char short_opts[] = "sd:";
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
int get_file_info();
/*向文件列表中添加读取到的文件*/
int add_file();



/*文件错误*/
static void file_error();
struct option long_opt[] =
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
            directory_name = optarg;
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
    play_list.filename_alloc_num = 100;
    if(get_file_info()){

    }
}

int get_file_info(){
    DIR *dir = opendir(directory_name);
    struct dirent *next_file;

    errno = 0;
    if(NULL == dir){
        file_error();
        return -1;
    }
    play_list.file = malloc(play_list.filename_alloc_num * sizeof(*play_list.file));
    while (1)
    {
        errno = 0;
        if(NULL != (next_file = readdir(dir))){
            add_file();
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