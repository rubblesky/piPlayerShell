
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
#include <string.h>

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
void print_list(struct play_list *play_list);
void print_menu(char *msg, ...);
/*设置终端属性*/
static void set_tty_attr();
void reset_tty_attr();
/*只适用于utf-8编码  目前只处理中文和日文*/
int get_size_diff(unsigned char *str);

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
#ifndef NDEBUG
    printf("win:(%d,%d) stdscr:(%d,%d)", size->ws_col, size->ws_row, LINES, COLS);
#endif
    return size;
}

void print_by_size() {
    refresh();
    size = get_terminal_size();
    resizeterm(size->ws_row,size->ws_col);
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
#ifndef NDEBUG
    if(list == NULL){
        printf("list == NULL");
    }
#endif
    player_init_color();
    touchwin(stdscr);
    refresh();
    struct play_list *play_list = get_play_list();
    print_list(play_list);
}
void player_init_color() {
    start_color();
    init_pair(PAIR_CHOOSE, COLOR_WHITE, COLOR_BLUE);
    init_pair(PAIR_OTHER, COLOR_BLACK, COLOR_WHITE);
}

void print_list(struct play_list *play_list) {
    int cell_height;
    if (PAGE_SONGNUM(size->ws_row) != 0){
        cell_height = getmaxy(list) / PAGE_SONGNUM(size->ws_row);
    }else{
        cell_height = 0;
    }
    int width = getmaxx(list) - BORDER_WIDTH;
    //wmove(list,cell_height / 2, 2);
    wattron(list, COLOR_PAIR(PAIR_OTHER));
    for (int y = 0; y < getmaxy(list); y++) {
        mvwprintw(list, y * cell_height + cell_height / 2, 2, "%-*.*s", width, width, " ");
    }
    music_t tmp = play_list->current_choose;
    for (int i = 0; i < PAGE_SONGNUM(size->ws_row) / 2 && tmp != NULL; i++) {
        tmp = list_get_prev(play_list->play_list_file, tmp);
    }
    if (tmp == NULL) {
        tmp = list_get_first(play_list->play_list_file);
    }
    wattron(list, COLOR_PAIR(PAIR_OTHER));
    for (int i = 0; i < PAGE_SONGNUM(size->ws_row) && tmp != NULL; i++) {
        if (tmp == play_list->current_choose) {
            if (((struct file_info *)(tmp->element))->name_size_diff == -1){
                ((struct file_info *)(tmp->element))->name_size_diff = get_size_diff(((struct file_info *)(tmp->element))->name);
            }
            wattron(list, A_BOLD | COLOR_PAIR(PAIR_CHOOSE));
            mvwprintw(list, i * cell_height + cell_height / 2, 2, "%-*.*s", (int)(width + ((struct file_info *)(tmp->element))->name_size_diff), width, ((struct file_info *)(tmp->element))->name);
            wattroff(list, A_BOLD | COLOR_PAIR(PAIR_CHOOSE));
            wattron(list, COLOR_PAIR(PAIR_OTHER));
        } else {
            mvwprintw(list, i * cell_height + cell_height / 2, 2, "%-*.*s", width, width, ((struct file_info *)(tmp->element))->name);
        }
        tmp = list_get_next(play_list->play_list_file, tmp);
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
    keypad(stdscr, true);
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

int get_size_diff(unsigned char *str){
    int size = strlen(str);
    int length = 0;
    for (int i = 0; i < size;i++){
        if (str[i] < 0x80 && str[i] < 0x80 > 0) {
            length++;
        } else if (str[i] >= 0x80 && str[i] < 0xE0) {
            length++;/*这里姑且这么写  八成歌名里不会出现这些字符*/
            i += 1;
        } else if (str[i] >= 0xE0 && str[i] < 0xF0) {
            length += 2; /*中日韩文字 包括假名 终端长度都为2*/
            i += 2;
        } else {
            i += 3;
            length += 2;
        }
    }
    return size - length;
}
