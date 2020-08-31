
#include "piPlayer.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <ncursesw/ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <locale.h>
//#include <ncurses.h>

#include "dir.h"
#include "err.h"
#include "sort.h"
#include "terminal.h"

//extern char **environ;

#define PAGE_SONGNUM 10
enum pair {
    PAIR_CHOOSE = 1,
    PAIR_OTHER
};

/*播放列表*/
struct play_list_info play_list;
/*播放器状态*/
enum player_status ps;
/*使用收藏目录*/
bool use_star_dir = 0;
/*显示非音乐文件*/
bool show_all_files = 0;
/*初始终端设置
  作为程序结束时恢复终端属性的依据
*/
static struct termios old_tty_attr;

/*设置终端属性*/
static void set_tty_attr();
/*切换目录*/
static int change_dirctory(char *dir);

/*绘制界面*/
static int
print_interface();
void print_by_size();
void print_in_width_termial(struct winsize *size);
void print_in_narrow_termial(struct winsize *size);
void player_init_color();
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
void send_cmd(char *cmd,FILE* fpipe);
/*获取当前音乐目录*/
char *get_current_dir();

/*退出程序
  静态函数 程序必须从主函数文件退出
*/
static void exit_player(int stauts);

/*打印目录*/
static void print_dir();

#define NEXT_SONG()                                              \
    if (play_list.current_choose < play_list.file_used_num - 1) { \
        play_list.current_choose++;                               \
    }
#define LAST_SONG()                    \
    if (play_list.current_choose > 0) { \
        play_list.current_choose--;     \
    }

static char short_opts[] = "std:";
static struct option long_opt[] =
    {
        {"directory", required_argument, NULL, 'd'},
        {0, 0, 0, 0}};

int main(int argc, char *argv[]) {
    /*初始化默认值*/
    play_list.sort_rule = by_name;
    play_list.music_dir = get_current_dir();
    ps = STOP;
    /*读取参数*/
    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opt, NULL)) != -1) {
        switch (opt) {
            case 0:
                break;
            case 'd':
                if(change_dirctory(optarg)<0){
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
    signal(SIGINT, exit_player);/*这里还要追加补充其他信号*/
    set_tty_attr();
    pthread_t tidp;
    if (pthread_create(&tidp, NULL, get_cmd, NULL) < 0) {
#ifndef NDEBUG
        printf("can't create control pthread\n");
#endif
        return -1;
    }
    print_interface();
}

static void set_tty_attr() {
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

static int print_interface() {
    play_list.file_alloc_num = 100;
    play_list.file_used_num = 0;
    if (read_dir(play_list.music_dir) < 0) {
        return -1;
    }
    play_list.current_choose = 0;
    print_by_size();
    while (1) {
    }
    endwin();
}
void print_by_size() {
    struct winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) < 0) {
        file_error("ioctl");
        exit_player(-1);
    }
    setlocale(LC_ALL, "");
    if (size.ws_col >= 2.3 * size.ws_row) {
        print_in_width_termial(&size);
    } else {
        print_in_narrow_termial(&size);
    }

}
void print_in_width_termial(struct winsize *size) {
    print_in_narrow_termial(size);
}
void print_in_narrow_termial(struct winsize *size) {
    initscr();
    cbreak();
    player_init_color();
    refresh();
    clear();
    int cell_height = size->ws_row / PAGE_SONGNUM;

    //attron(A_BOLD|COLOR_PAIR(PAIR_CHOOSE));
    int width = size->ws_col - 6;
    move(cell_height / 2, 2);
    int i;
    if (play_list.current_choose < PAGE_SONGNUM / 2) {
        i = 0;
    } else if (play_list.file_used_num - play_list.current_choose < PAGE_SONGNUM) {
        i = play_list.file_used_num - PAGE_SONGNUM;
    } else {
        i = play_list.current_choose - PAGE_SONGNUM / 2;
    }

        //mvprintw(cell_height / 2, 2, "%-.*s..." , width, play_list.sorted_file[i]->name);
        //attroff(A_BOLD | COLOR_PAIR(PAIR_CHOOSE));
        attron(COLOR_PAIR(PAIR_OTHER));
    int n = 0;
    for (; n < PAGE_SONGNUM && i < play_list.file_used_num /*&&   */; i++) {
        if(i == play_list.current_choose){
            attron(A_BOLD | COLOR_PAIR(PAIR_CHOOSE));
            mvprintw(n * cell_height + cell_height / 2, 2, "%-.*s...", width, play_list.sorted_file[i]->name);
            attroff(A_BOLD | COLOR_PAIR(PAIR_CHOOSE));
            attron(COLOR_PAIR(PAIR_OTHER));
        }
        else{
            mvprintw(n * cell_height + cell_height / 2, 2, "%-.*s...", width, play_list.sorted_file[i]->name);
        }
        n++;
    }
    refresh();
}

void player_init_color() {
    start_color();
    init_pair(PAIR_CHOOSE, COLOR_WHITE, COLOR_BLUE);
    init_pair(PAIR_OTHER, COLOR_BLACK, COLOR_WHITE);
}

static void *get_cmd(void *arg) {
    char c;
    FILE *fpipe = NULL;
    while ((c = fgetc(stdin)) != EOF) {
        switch (c) {
            case 'u':
                LAST_SONG();
                break;
            case 'd':
                NEXT_SONG();
                break;
            case '\033':
                deal_arrow_key(fpipe);
                break;
            case '\n':
            case 'b':
                fpipe = play();
                play_list.current_playing = play_list.current_choose;
#ifndef NDEUG
                printf("%s", play_list.sorted_file[play_list.current_playing]->name);
                printf("%s", play_list.music_dir);
#endif
                if (fpipe == NULL) {
                    printf("play fail");
                }
                break;

            case '-':
                send_cmd("/", fpipe);
            case '+':
                send_cmd("*",fpipe);
            case ' ':
                send_cmd(" ", fpipe);
                if(ps!=STOP){
                    ps = (ps == PAUSE) ? PLAYING : PAUSE;
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
                print_by_size();
                break;
            case 'B':
                NEXT_SONG()
                print_by_size();
               
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
    printf("playing %d\n", play_list.current_choose + 1);
#endif
    return fpipe;
}

void get_player_status(int *last_song) {
    int statloc;
    pid_t p = wait(&statloc);
#ifndef NDEBUG
    printf("wait pid :%d  exit stauts : %d\n", p, WEXITSTATUS(statloc));
#endif
    ps = STOP;
}

void send_cmd(char *cmd, FILE *fpipe) {
    if (ps != STOP) {
        if (fprintf(fpipe,"%s",cmd) == EOF) {
            file_error("send command fail\n");
        } 
        fflush(fpipe);
    }
}
static int fork_player_process(pid_t last_song, FILE **fpipe) {
    if (last_song != 0) {
        kill(last_song, SIGINT);
    }
    int fd[2];
    if (pipe(fd) < 0) {
        file_error("pipe error");
        return -1;
    }
    pid_t pid;
    if ((pid = fork()) < 0) {
        sys_error("fork error");
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

static void launch_player(int fd_pipe[]) {
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

static void print_dir() {
    for (int i = 0; i < play_list.file_used_num; i++) {
        printf("%s\n", play_list.file->name);
    }
}

void exit_player(int stauts) {
    /*向所有子进程发送终止信号*/
    kill(0, SIGINT);
    //endwin();
    /*恢复终端属性*/
    tcsetattr(fileno(stdin), TCSADRAIN, &old_tty_attr);
    exit(stauts);
}
