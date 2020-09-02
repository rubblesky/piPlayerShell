
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
#include <sys/wait.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
//#include <ncurses.h>

#include "dir.h"
#include "err.h"
#include "interface.h"
#include "sort.h"
#include "terminal.h"
//extern char **environ;

/*播放列表*/
struct play_list_info play_list;
enum play_setting play_setting;
/*歌曲结束*/
bool is_end;
/*播放器状态*/
enum player_status ps;
/*使用收藏目录*/
bool use_star_dir = 0;
/*显示非音乐文件*/
bool show_all_files = 0;
/*初始终端设置
  作为程序结束时恢复终端属性的依据
*/
/*父进程向子进程发送信息的管道*/
FILE *fpipe = NULL;
static struct termios old_tty_attr;

struct play_list_info *get_play_list() {
    return &play_list;
}

/*切换目录*/
static int change_dirctory(char *dir);
/*等待键盘命令*/
int wait_for_stdin();

/*读取键盘命令*/
static void *get_cmd(void *arg);
/*处理方向键*/
static void deal_arrow_key(FILE *fpipe);

static FILE *play();
/*fork播放音乐的子进程 
  fpipe为父进程向子进程传递信息的管道
  返回值是子进程pid
*/
static int fork_player_process(pid_t last_song, FILE **fpipe);
static void launch_player(int fd_pipe[]);
/*获取播放器状态*/
void get_player_status(int *last_song);
/*向播放器发送命令*/
void send_cmd(char *cmd, FILE *fpipe);
/*获取当前音乐目录*/
char *get_current_dir();

void print_play_setting();
/*播放器收到歌曲结束信号时执行的命令*/
static void player_stop(int signo);
void exit_player(int stauts);

/*打印目录*/
static void print_dir();

#define NEXT_SONG()                                               \
    if (play_list.current_choose < play_list.file_used_num - 1) { \
        play_list.current_choose++;                               \
    }
#define LAST_SONG()                     \
    if (play_list.current_choose > 0) { \
        play_list.current_choose--;     \
    }

static char short_opts[] = "astd:";
static struct option long_opt[] =
    {
        {"directory", required_argument, NULL, 'd'},
        {0, 0, 0, 0}};

int main(int argc, char *argv[]) {
    /*初始化默认值*/
    play_list.sort_rule = by_name;
    play_list.music_dir = get_current_dir();
    play_list.file_alloc_num = 100;
    play_list.file_used_num = 0;
    play_list.current_choose = 0;
    ps = STOP;
    play_setting = SINGLE_PLAY;
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
                play_list.sort_rule = by_last_modification_time;
                break;
            default:
                break;
        }
    }
    /*收到SIGINT信号时执行默认退出程序*/
    signal(SIGINT, exit_player); /*这里还要追加补充其他信号*/
    signal(SIGWINCH, print_by_size);
    pthread_t tidp;
    if (read_dir(play_list.music_dir) < 0) {
        return -1;
    }
    print_interface();
    if (pthread_create(&tidp, NULL, get_cmd, NULL) < 0) {
#ifndef NDEBUG
        printf("can't create control pthread\n");
#endif
        return -1;
    }
    while (1) {
        sleep(10);
    }
}

static int change_dirctory(char *dir) {
    if (chdir(dir) < 0) {
        file_error("can't change directory");
        return -1;
    }
    if (play_list.music_dir != NULL) {
        free(play_list.music_dir);
    }
    play_list.music_dir = get_current_dir();
    if (play_list.music_dir != NULL) {
        return 0;
    } else {
        return -1;
    }
}

int wait_for_stdin() {
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fileno(stdin), &fdset);
    struct timeval tm;
    tm.tv_sec = 1;
    tm.tv_usec = 0;
    return select(1, &fdset, NULL, NULL, &tm);
    //return select(1, &fdset, NULL, NULL, NULL);
}

static void *get_cmd(void *arg) {
    char c;
    int is_stdin_in;
    while (1) {
        if ((is_stdin_in = wait_for_stdin()) < 0) {
            file_error("select");
            continue;
        } 
        else if ((is_stdin_in == 0 && is_end) || is_stdin_in > 0) {
            if ((c = fgetc(stdin)) != EOF) {
                    switch (c) {
                        case 'u':
                            LAST_SONG();
                            break;
                        case 'd':
                            NEXT_SONG();
                            break;
                        case 'p':
                            if (play_setting == RANDOM_PLAY) {
                                play_setting = 0;
                            } else {
                                play_setting++;
                            }
                            print_play_setting();
                            break;
                        case '\033':
                            deal_arrow_key(fpipe);
                            break;
                        case '\n':
                        case 'b':
                            fpipe = play();
                            print_menu("playing");
                            play_list.current_playing = play_list.current_choose;
                            if (fpipe == NULL) {
                                printf("play fail");
                            }
                            break;

                        case '-':
                            send_cmd("/", fpipe);
                            print_menu("volume -");
                            break;
                        case '+':
                            send_cmd("*", fpipe);
                            print_menu("volume +");
                            break;
                        case ' ':
                            send_cmd(" ", fpipe);
                            if (ps != STOP) {
                                ps = (ps == PAUSE) ? PLAYING : PAUSE;
                            }
                            if (ps == PAUSE) {
                                print_menu("pause");
                            } else {
                                print_menu("");
                            }
                            break;
                        case 'q':
                            print_menu("Exit...\n");
                            exit_player(0);

                        default:
                            break;
                    }
            }
        }
    }
}

static void deal_arrow_key(FILE *fpipe) {
    char c;
    if ((c = fgetc(stdin)) != '[') {
        ungetc(c, stdin);
        return;
        //ungetc(c, stdin);
    } else if ((c = fgetc(stdin)) != EOF) {
        switch (c) {
            case 'A':
                LAST_SONG()
                print_list(&play_list);
                break;
            case 'B':
                NEXT_SONG()
                print_list(&play_list);

                break;
            case 'C':
                if (ps != STOP)
                    fprintf(fpipe, "\033[C");
                break;
            case 'D':
                if (ps != STOP)
                    fprintf(fpipe, "\033[D");
                break;
            default:
                fprintf(stderr, "unknow command\n");
                break;
        }
    }
}

static FILE *play() {
    static pid_t last_song = 0;
    static pthread_t tidp;
    FILE *fpipe = NULL;
    if ((last_song = fork_player_process(last_song, &fpipe)) == -1) {
        ps = STOP;
        return NULL;
    } else {
        ps = PLAYING;
    }
    /*监视进程子结束的线程*/
    int error_num;
    if ((error_num = pthread_create(&tidp, NULL, (void *(*)(void *))get_player_status, &last_song)) < 0) {
        fprintf(stderr, strerror(error_num));
        exit(-1);
    }
#ifndef NDEBUG
    //printf("playing %d\n", play_list.current_choose + 1);
#endif
    return fpipe;
}

void get_player_status(int *last_song) {
    int statloc;
    pid_t p = wait(&statloc);
    print_menu("stop");
    ps = STOP;
    //int exit_num = WEXITSTATUS(statloc);
    //print_menu("exit %d", exit_num);
    if (is_end == 1) {
        switch (play_setting) {
            case RANDOM_PLAY:
                srand(time(NULL));
                play_list.current_choose = rand() % play_list.file_used_num;
                print_list(&play_list);
                ungetc('b', stdin);
                break;
            default:
                break;
        }
    }
}

void send_cmd(char *cmd, FILE *fpipe) {
    if (ps != STOP) {
        if (fprintf(fpipe, "%s", cmd) == EOF) {
            file_error("send command fail\n");
        }
        fflush(fpipe);
    }
}
static int fork_player_process(pid_t last_song, FILE **fpipe) {
    if (last_song != 0) {
        if (ps != STOP) {
            is_end = 0;
        } else {
            is_end = 1;
        }
        kill(last_song, SIGINT);
    }
    while (ps != STOP) {
        /*或许还可以少睡一会*/
        sleep(1);
    }
    int fd[2];
    if (pipe(fd) < 0) {
        file_error("pipe error");
        return -1;
    }
    pid_t pid;
    if ((pid = fork()) < 0) {
        sys_error("fork error");
        is_end = 1;
        return -1;
    } else if (pid != 0) {
        /*父进程*/
        close(fd[0]);
        *fpipe = fdopen(fd[1], "w");
        setbuf(*fpipe, NULL);
        return pid;
    } else {
        /*子进程*/
        launch_player(fd);
    }
}

static void player_stop(int signo) {
    exit(24);
    /*24是一个随便的数
    为了不冲突*/
}

static void launch_player(int fd_pipe[]) {
#ifndef NDEBUG
    printf("child pid:%d\n", getpid());
#endif
    /*子进程恢复对SIGINT信号的处理*/
    signal(SIGINT, player_stop);
    /*关闭输出流 使之不显示*/
    int err_copy = dup(STDERR_FILENO);
    FILE *ferr = fdopen(err_copy, "w");
    if (ferr == NULL) {
        file_error("fdopen error");
    }
    setbuf(ferr, NULL);
    fclose(stdout);
    fclose(stderr);
    close(fd_pipe[1]);
    if (dup2(fd_pipe[0], STDIN_FILENO) != STDIN_FILENO) {
        player_error(ferr, "dup2 ");
        exit(-1);
    }
    close(fd_pipe[0]);
    if (execlp("mplayer", "mplayer", play_list.sorted_file[play_list.current_choose]->name, NULL) < 0) {
        player_error(ferr, "mplayer");
        exit(-1);
    }
    exit(0);
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

void print_play_setting() {
    switch (play_setting) {
        case SINGLE_PLAY:
            print_menu("单曲播放");
            break;
        case SINGLE_TUNE_CIRCULATION:
            print_menu("单曲循环");
            break;
        case LOOP_PLAYBACK:
            print_menu("循环播放");
            break;
        case ORDER_PLAYBACK:
            print_menu("顺序播放");
            break;
        case RANDOM_PLAY:
            print_menu("随机播放");
            break;
        default:
            break;
    }
}

static void print_dir() {
    for (int i = 0; i < play_list.file_used_num; i++) {
        printf("%s\n", play_list.file->name);
    }
}

void exit_player(int stauts) {
    /*向所有子进程发送终止信号*/
    kill(0, SIGINT);
    //endwin();
    reset_tty_attr();
    exit(stauts);
}
