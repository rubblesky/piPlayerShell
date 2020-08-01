#include"piPlayer.h"
#include"terminal.h"

#include <unistd.h>
#include<stdio.h>
#include<getopt.h>
#include<dirent.h>
#include<stdbool.h>


char short_opts[] = "sd:";

struct option long_opt[] =
{
    {"directory", required_argument, NULL, 'd'},
    {0,0,0,0}
};

char * directory = "./";
bool is_specify_directory;
bool use_star_dir;


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
            directory = optarg;
            break;
        case 's':
            use_star_dir = 1;
            break;
        default:
            break;
        }
    }

    DIR *current_dir = opendir(directory);
    struct dirent *restrict next_file;
    while ((next_file = readdir(current_dir)) != NULL)
    {
        if(next_file->d_name[0] != '.')
            printf("%s\n", next_file->d_name);
    }
    closedir(current_dir);
}