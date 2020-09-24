#include "interface.h"

#include <fcntl.h>
#include <locale.h>
#include <ncursesw/ncurses.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "dir.h"
#include "err.h"
#include "piPlayer.h"

#define PAGE_SONGNUM(size) (size - 4)
#define MENU_HEIGHT 2
#define BORDER_WIDTH 4
/*初始终端设置
  作为程序结束时恢复终端属性的依据
*/
static struct termios old_tty_attr;

struct winsize *size;

enum pair {
    PAIR_CHOOSE = 1,
    PAIR_OTHER
};
/*音乐列表栏*/
static WINDOW *list;
/*菜单栏*/
static WINDOW *menu;
/*绘制界面*/
int print_interface();
struct winsize *get_terminal_size();
void print_by_size();
void print_in_width_termial();
void print_in_narrow_termial();
void player_init_color();
void print_list(struct play_list_info *play_list);
void print_menu(char *msg, ...);
/*设置终端属性*/
static void set_tty_attr();
void reset_tty_attr();
int print_interface() {   
    setlocale(LC_ALL, "");
    initscr();
    set_tty_attr();

    print_by_size();
    refresh();
}

struct winsize *get_terminal_size() {
    struct winsize *size = (struct winsize *)calloc(sizeof(struct winsize), 1);

    if (ioctl(0, TIOCGWINSZ, size) < 0) {
        file_error("ioctl");
        exit_player(-1);
    }
    return size;
}

void print_by_size() {
    size = get_terminal_size();
    clear();
    if (size->ws_col * 2.3 > size->ws_row) {
        print_in_width_termial();
    } else {
        print_in_narrow_termial();
    }
}

void print_in_width_termial() {
    print_in_narrow_termial();
}
void print_in_narrow_termial() {
    if (menu != NULL) {
        delwin(menu);
        menu = NULL;
    }
    if (list != NULL) {
        delwin(list);
        list = NULL;
    }
    menu = subwin(stdscr, MENU_HEIGHT, size->ws_col, size->ws_row - MENU_HEIGHT, 0);
    list = subwin(stdscr, size->ws_row - MENU_HEIGHT, size->ws_col, 0, 0);
    player_init_color();
    touchwin(stdscr);
    refresh();
    struct play_list_info *play_list = get_play_list();
    print_list(play_list);
}
void player_init_color() {
    start_color();
    init_pair(PAIR_CHOOSE, COLOR_WHITE, COLOR_BLUE);
    init_pair(PAIR_OTHER, COLOR_BLACK, COLOR_WHITE);
}

void print_list(struct play_list_info *play_list) {
    int cell_height = getmaxy(list) / PAGE_SONGNUM(size->ws_row);
    int width = getmaxx(list) - BORDER_WIDTH;
    //wmove(list,cell_height / 2, 2);

    int i;
    if (play_list->current_choose < PAGE_SONGNUM(size->ws_row) / 2) {
        i = 0;
    } else if (play_list->music_num - play_list->current_choose < PAGE_SONGNUM(size->ws_row)) {
        i = play_list->music_num - PAGE_SONGNUM(size->ws_row) + 1;
    } else {
        i = play_list->current_choose - PAGE_SONGNUM(size->ws_row) / 2;
    }
    wattron(list, COLOR_PAIR(PAIR_OTHER));
    int n = 0;
    for (; n < PAGE_SONGNUM(size->ws_row) - 1 && i < play_list->music_num /*&&   */; i++) {
        if (i == play_list->current_choose) {
            wattron(list, A_BOLD | COLOR_PAIR(PAIR_CHOOSE));
            mvwprintw(list, n * cell_height + cell_height / 2, 2, "%-*.*s", width, width, play_list->file_list.sorted_file[i]->name);
            wattroff(list, A_BOLD | COLOR_PAIR(PAIR_CHOOSE));
            wattron(list, COLOR_PAIR(PAIR_OTHER));
        } else {
            mvwprintw(list, n * cell_height + cell_height / 2, 2, "%-*.*s", width, width, play_list->file_list.sorted_file[i]->name);
        }
        n++;
        //wrefresh(list);
    }
    wrefresh(list);
}
void print_menu(char *msg, ...) {
    wclear(menu);
    char szBuf[1024];
    va_list _va_list;
    va_start(_va_list, msg);        /* 初始化变长参数列表 */
    vsprintf(szBuf, msg, _va_list); /* 传递变长参数 */
    va_end(_va_list);               /* 结束使用变长参数列表 */
    wprintw(menu, "%s", szBuf);
    wrefresh(menu);
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
    /*
    修改模式为非规范模式
    new_tty_attr.c_lflag &= ~ICANON;
    new_tty_attr.c_lflag &= ~ECHO;
    new_tty_attr.c_cc[VTIME] = 0;
    new_tty_attr.c_cc[VMIN] = 0;
    */
    noecho();
    cbreak();
    keypad(stdscr,true);
    if (tcsetattr(fileno(stdin), TCSADRAIN, &new_tty_attr) < 0) {
#ifndef NDEBUG
        printf("can't set tty attribute\n");
#endif
        exit_player(-1);
    }
}

void reset_tty_attr() {
    /*恢复终端属性*/
    tcsetattr(fileno(stdin), TCSADRAIN, &old_tty_attr);
}