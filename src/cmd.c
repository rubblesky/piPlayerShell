#include "cmd.h"

#include <ncursesw/ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "err.h"
#include "interface.h"
#include "queue.h"

#define NEXT_SONG()                                           \
    if (play_list.current_choose < play_list.music_num - 1) { \
        play_list.current_choose++;                           \
    }
#define LAST_SONG()                     \
    if (play_list.current_choose > 0) { \
        play_list.current_choose--;     \
    }

void *get_cmd(void *queue);

void *deal_cmd(void *queue);



void *control(void *arg){
    pthread_t get_cmd_tidp,deal_cmd_tidp;
    Queue *q = init_queue(5);
    pthread_create(&get_cmd_tidp, NULL, get_cmd, (void*)q);
    pthread_create(&deal_cmd_tidp, NULL, deal_cmd, (void*)q);
}

void *get_cmd(void *queue) {
    Queue *q = queue;
    int c;
    while ((c = getch()) != ERR) {
        if (queue_insert(q, c) != 0) {
            print_menu("操作过于频繁 请稍后再试");
        }
    }
}

void *deal_cmd(void *queue) {
    int cmd;
    while (1){
        if(queue_get_head( (Queue*)queue,&cmd) == 0){
            switch (cmd)
            {
            case 'q':
                exit_player(0);
                break;
            case 'p':
                if (play_setting == RANDOM_PLAY) {
                    play_setting = 0;
                } else {
                    play_setting++;
                }
            default:
                break;
            }
        }
    }
}




















/*等待键盘命令*/
int wait_for_stdin();

/*读取键盘命令*/
void *get_cmd(void *arg);
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



static void *get_cmd(void *arg) {
    char c;
    int is_stdin_in;
    while (1) {
        if ((is_stdin_in = wait_for_stdin()) < 0) {
            file_error("select");
            continue;
        }
        /*这里还有极小的概率出问题*/
        else if ((!is_stdin_in /*&& is_end*/ && ps == STOP) || is_stdin_in > 0) {
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
    play_list.current_playing = play_list.current_choose;
    static pid_t last_song = 0;
    static pthread_t tidp;
    FILE *fpipe = NULL;
    if ((last_song = fork_player_process(last_song, &fpipe)) == -1) {
        ps = STOP;
        print_menu("play fail");
        return NULL;
    } else {
        ps = PLAYING;
        /*监视进程子结束的线程*/
        int error_num;
        if ((error_num = pthread_create(&tidp, NULL, (void *(*)(void *))get_player_status, &last_song)) < 0) {
            fprintf(stderr, strerror(error_num));
            exit(-1);
        }
        return fpipe;
    }
}

void get_player_status(int *last_song) {
    int statloc;
    pid_t p = wait(&statloc);
    //int exit_num = WEXITSTATUS(statloc);
    //print_menu("exit %d", exit_num);
    if (is_end) {
        switch (play_setting) {
            case RANDOM_PLAY:
                srand(time(NULL));
                play_list.current_choose = rand() % play_list.music_num;
                print_list(&play_list);
                ungetc('b', stdin);
                break;
            default:
                break;
        }
    }
    is_end = 1;
    print_menu("stop");
    ps = STOP;
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
    print_menu("playing");
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
    if (execlp("mplayer", "mplayer", play_list.file_list.sorted_file[play_list.current_choose]->name, NULL) < 0) {
        player_error(ferr, "mplayer");
        exit(-1);
    }
    exit(0);
}
