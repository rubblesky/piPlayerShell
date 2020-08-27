
#include "piPlayer.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "dir.h"
#include "err.h"
#include "sort.h"
#include "terminal.h"

//extern char **environ;

/*播放列表*/
struct play_list_info play_list;
/*播放器状态*/
enum player_status ps;
/*指定目录*/
bool is_specify_directory = 0;
/*使用收藏目录*/
bool use_star_dir = 0;
/*显示隐藏文件*/
bool show_hidden_files = 0;
/*初始终端设置
  作为程序结束时恢复终端属性的依据*/
struct termios old_tty_attr;

/*绘制界面*/
static int print_interface();
/*读取键盘命令*/
static void *get_cmd(void *arg);
/*播放音乐 
  fd1为父进程向子进程的通讯管道
  fd2为子进程向父进程的通讯管道
*/
static int play(pid_t last_song, FILE **fpipe);
/*获取播放器状态*/
void get_player_status(int *last_song);


/*获取当前音乐目录*/
char *get_current_dir();

/*退出程序*/
void exit_player(int stauts);

/*打印目录*/
static void print_dir();

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
                printf("now cwd is %s\n", play_list.music_dir);
#endif

                if (chdir(optarg) < 0) {
                    file_error("can't change directory");
                }
                if (play_list.music_dir != NULL)
                    free(play_list.music_dir);
                play_list.music_dir = get_current_dir();

#ifndef NDEBUG
                printf("now cwd is %s\n", play_list.music_dir);
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
    /*收到SIGINT信号时执行默认退出程序*/
    signal(SIGINT, exit_player);
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
    pthread_t tidp;
    char c;
    /*
    if (setvbuf(stdin,NULL,_IONBF,0) != 0) {
#ifndef NDEBUG
        printf("buf modify error\n");
#endif

    }
*/
    struct termios new_tty_attr;
    if (tcgetattr(fileno(stdin), &old_tty_attr) < 0) {
#ifndef NDEBUG
        printf("can't get tty attribute\n");
#endif
        exit_player(-1);
    }
    new_tty_attr = old_tty_attr;
    /*修改模式为非规范模式*/
    new_tty_attr.c_lflag &= ~ICANON;
    new_tty_attr.c_lflag &= ~ECHO;
    if (tcsetattr(fileno(stdin), TCSADRAIN, &new_tty_attr) < 0) {
#ifndef NDEBUG
        printf("can't set tty attribute\n");
#endif
        exit_player(-1);
    }
    pid_t last_song = 0;
    FILE *fpipe = NULL;
    while ((c = fgetc(stdin)) != EOF) {
        switch (c) {
            case 'u':
                if (play_list.current_music > 0)
                    play_list.current_music--;
                break;
            case 'd':
                if (play_list.current_music < play_list.file_used_num - 1)
                    play_list.current_music++;
                break;
            case 'b':

                last_song = play(last_song, &fpipe);
                ps = PLAYING;
                /*监视进程子结束的线程*/
                if (pthread_create(&tidp, NULL, (void *(*)(void *))get_player_status, &last_song) < 0) {
#ifndef NDEBUG
                    printf("can't create player pthread\n");
#endif
                    exit(-1);
                }
#ifndef DEBUG
                printf("playing %d\n", play_list.current_music + 1);
#endif
                break;
            
            case ' ':
                if (ps == PLAYING){
                    fputc(c, fpipe);
#ifndef DEBUG
                    printf("player status :%s\n", (ps == PLAYING)?"playing":"stop");
#endif
                }
                break;
            case 'q':
                printf("Exit...\n");
                exit_player(0);

            default:
                break;
        }
    }
}

void get_player_status(int *last_song) {
        int statloc;
        pid_t p = wait(&statloc);
#ifndef NDEBUG
        printf("wait pid :%d  exit stauts : %d\n", p,WEXITSTATUS(statloc));
#endif
        ps = STOP;

}

static int play(pid_t last_song, FILE **fpipe) {
    if (last_song != 0) {
        kill(last_song, SIGINT);
    }

    int fd[2];
    if (pipe(fd) < 0) {
        file_error("pipe error");
    }
    pid_t pid;
    if ((pid = fork()) < 0) {
        printf("play error\n");
        return -1;
    } else if (pid != 0) {
        /*父进程*/
        close(fd[0]);
        *fpipe = fdopen(fd[1], "w");
        return pid;

    } else {
        /*子进程*/

#ifndef NDEBUG
        printf("child pid:%d\n", getpid());
#endif
        /*子进程恢复对SIGINT信号的处理*/
        signal(SIGINT, SIG_DFL);
        /*关闭输出流 使之不显示*/
        int err_copy = dup(STDERR_FILENO);
        FILE *ferr = fdopen(err_copy, "w");
        if (ferr == NULL) {
            file_error("fdopen error");
        }
        setbuf(ferr, NULL);
        fclose(stdout);
        fclose(stderr);
        if(dup2(fd[0],STDIN_FILENO) != STDIN_FILENO){
            file_error("dup2 ");
            exit(-1);
        }

        close(fd[1]);
        if (execlp("mplayer", "mplayer", play_list.sorted_file[play_list.current_music]->name, NULL) < 0) {
            player_error(ferr, "mplayer");
            exit(-1);
        }
        exit(0);
    }
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

static void print_dir() {
    for (int i = 0; i < play_list.file_used_num; i++) {
        printf("%s\n", play_list.file->name);
    }
}

void exit_player(int stauts) {
    /*向所有子进程发送终止信号*/
    kill(0, SIGINT);
    /*恢复终端属性*/
    tcsetattr(fileno(stdin), TCSADRAIN, &old_tty_attr);
    exit(stauts);
}
