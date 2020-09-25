#include "cmd.h"

#include <errno.h>
#include <ncursesw/ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "err.h"
#include "interface.h"
#include "queue.h"

bool is_end;

struct child_thread_args {
    Queue *q;
    struct play_list *play_list;
};

void *control(void *arg);

void *get_cmd(void *queue);
void *deal_cmd(void *queue);

void choose_next(struct play_list *play_list);
void choose_prev(struct play_list *play_list);

void change_play_status(struct play_list *play_list);
static FILE *play(struct play_list *play_list, Queue *q);
static int fork_player_process(struct play_list *play_list, pid_t last_song, FILE **fpipe);

static void launch_player(struct play_list *play_list, int fd_pipe[]);

/*获取播放器状态*/
void *get_player_status(void *arg);
/*向播放器发送命令*/
void send_cmd(struct play_list *play_list,char *cmd, FILE *fpipe);

void *control(void *arg) {
    pthread_t get_cmd_tidp, deal_cmd_tidp;
    Queue *q = init_queue(5);
    if (q != NULL) {
        pthread_create(&get_cmd_tidp, NULL, get_cmd, (void *)q);
        pthread_create(&deal_cmd_tidp, NULL, deal_cmd, (void *)q);
    } else {
        alloc_error();
        exit_player(-1);
    }
}

void *get_cmd(void *queue) {
    Queue *q = queue;
    int c;
    while ((c = getch()) != ERR) {
        if (queue_insert(q, c) != 0) {
            print_menu("操作过于频繁 请稍后再试");
            sleep(1);
        }
    }
}

void *deal_cmd(void *queue) {
    /*父进程向子进程发送信息的管道*/
    FILE *fpipe = NULL;
    struct play_list *play_list = get_play_list();
    struct timespec ts;
    ts.tv_sec = 0;
    /*十毫秒*/
    ts.tv_nsec = 10000000;
    int cmd;
    while (1) {
        if (queue_get_head((Queue *)queue, &cmd) == 0) {
            switch (cmd) {
                case 'q':
                case 'Q':
                    exit_player(0);
                    print_menu("Exit...\n");
                    break;
                case 'P':
                case 'p':
                    change_play_status(play_list);
                    break;
                case KEY_DOWN:
                    choose_next(play_list);
                    break;
                case KEY_UP:
                    choose_prev(play_list);
                    break;
                case 'B':
                case 'b':
                    fpipe = play(play_list, (Queue*)queue);
                    break;
                case 'R':
                case 'r':



                case '-':
                    send_cmd(play_list,"/", fpipe);
                    print_menu("volume -");
                    break;
                case '+':
                    send_cmd(play_list,"*", fpipe);
                    print_menu("volume +");
                    break;
                case ' ':
                    send_cmd(play_list," ", fpipe);
                    if (play_list->play_setting != STOP) {
                        play_list->play_setting = (play_list->play_setting == PAUSE) ? PLAYING : PAUSE;
                    }
                    if (play_list->play_setting == PAUSE) {
                        print_menu("pause");
                    } else {
                        print_menu("");
                    }
                    break;

                default:
                    break;
            }
            queue_remove((Queue*)queue);
        }
        if (nanosleep(&ts, &ts) == -1) {
            ts.tv_nsec = 10000000;
        }
    }
}

void choose_next(struct play_list *play_list) {
    music_t tmp = list_get_next(play_list->play_list_file, play_list->current_choose);
    if (tmp != NULL) {
        play_list->current_choose = tmp;
        print_list(play_list);
    }

}

void choose_prev(struct play_list *play_list) {
    music_t tmp = list_get_prev(play_list->play_list_file, play_list->current_choose);
    if (tmp != NULL) {
        play_list->current_choose = tmp;
        print_list(play_list);
    }
}

void change_play_status(struct play_list *play_list) {
    if (play_list->play_setting == ORDER_PLAYBACK) {
        play_list->play_setting = 0;
    } else {
        play_list->play_setting++;
    }

    switch (play_list->play_setting) {
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
            /*
        case RANDOM_PLAY:
            print_menu("随机播放");
            break;
            */
        default:
            break;
    }
}


static FILE *play(struct play_list *play_list, Queue *q) {
    play_list->current_playing = play_list->current_choose;
    static pid_t last_song = 0;
    static pthread_t tidp;
    FILE *fpipe = NULL;
    if ((last_song = fork_player_process(play_list, last_song, &fpipe)) == -1) {
        play_list->player_status = STOP;
        print_menu("play fail");
        return NULL;
    } else {
        play_list->player_status = PLAYING;
        /*这可能是个坑  不行就换成malloc*/
        static struct child_thread_args args;
        args.play_list = play_list;
        args.q = q;
        /*监视进程子结束的线程*/
        int error_num;
        if ((error_num = pthread_create(&tidp, NULL, (void *(*)(void *))get_player_status, (void *)(&args))) < 0) {
            fprintf(stderr, strerror(error_num));
            exit(-1);
        }
        return fpipe;
    }
}

static int fork_player_process(struct play_list *play_list, pid_t last_song, FILE **fpipe) {
    if (last_song != 0) {
        if (play_list->player_status != STOP) {
            is_end = 0;
        } else {
            is_end = 1;
        }
        kill(last_song, SIGINT);
    }
    while (play_list->player_status != STOP) {
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
        launch_player(play_list, fd);
    }
}

static void launch_player(struct play_list *play_list, int fd_pipe[]) {
#ifndef NDEBUG
    printf("child pid:%d\n", getpid());
#endif
    /*子进程恢复对SIGINT信号的处理*/
    signal(SIGINT,SIG_DFL);
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
    if (execlp("mplayer", "mplayer", ((struct file_info *)(play_list->current_choose->element))->name, NULL) < 0) {
        player_error(ferr, "mplayer");
        exit(-1);
    }
    exit(0);
}

void *get_player_status(void *arg) {
    int statloc;
    pid_t p = wait(&statloc);
    struct child_thread_args *args = arg;
    Queue *q = args->q;
    struct play_list *play_list = args->play_list;
    print_menu("stop");
    play_list->player_status = STOP;
    music_t tmp;
    if (is_end) {
        switch ( play_list->play_setting) {
            case SINGLE_PLAY:
                break;
            case SINGLE_TUNE_CIRCULATION:
                play_list->current_choose = play_list->current_playing;
                queue_insert(q, (int)('b'));
                break;
            case LOOP_PLAYBACK:
                tmp = list_get_next(play_list->play_list_file, play_list->current_playing);
                if(tmp != NULL){
                    play_list->current_choose = tmp;
                }
                else{
                    play_list->current_choose = list_get_first(play_list->play_list_file);
                }
                print_list(play_list);
                queue_insert(q, (int)('b'));
                break;
            case ORDER_PLAYBACK:
                tmp = list_get_next(play_list->play_list_file, play_list->current_playing);
                if (tmp != NULL) {
                    play_list->current_choose = tmp;
                    queue_insert(q, (int)('b'));
                }                 
                break;
            
            default:
                break;
        }
    }
}

void send_cmd(struct play_list *play_list,char *cmd, FILE *fpipe) {
    if (play_list->player_status != STOP) {
        if (fprintf(fpipe, "%s", cmd) == EOF) {
            file_error("send command fail\n");
        }
        fflush(fpipe);
    }
}