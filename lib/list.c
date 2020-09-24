#include "list.h"
#include <stdlib.h>

List * init_lsit(){
    List * ls = malloc(sizeof(List));
    if(ls != NULL){
        struct list_node *ln = malloc(sizeof(struct list_node));
        if(ln != NULL){
            ln->prev = NULL;
            ln->next = NULL;
            ln->element = NULL;

            ls->head = ln;
            ls->tail = ln;
        }
        else {
            free(ls);
            return NULL;
        }
        return ls;
    } else {
        return NULL;
    }
}

void free_list(List *ls){
    while(ls->tail != ls->head){
        ls->tail = ls->tail->prev;
        free(ls->tail->next);
    }
    free(ls->head);
    free(ls);
}
/*在尾部增加节点*/
int list_append(List *ls,void *element){

    struct list_node *new_node = malloc(sizeof(struct list_node));
    if(new_node == NULL){
        return ERROR_ALLOC;
    }
    else{
        ls->tail->next = new_node;
        new_node->prev = ls->tail;
        ls->tail = new_node;
        new_node->next = NULL;
        new_node->element = element;
        return 0;
    }
}



/*在ln位置之前增加节点*/
int list_insert(List *ls,struct list_node *ln,void *element){
    struct list_node *new_node = malloc(sizeof(struct list_node));
    if (new_node == NULL) {
        return ERROR_ALLOC;
    }
    else{
        ln->prev->next = new_node;
        new_node->prev = ln->prev;
        new_node->next = ln;
        ln->prev = new_node;
        new_node->element = element;
        return 0;
    }
}

struct list_node* list_get_first(List *ls){
    if(ls->head != ls->tail){
        return ls->head->next;
    }
    else{
        return NULL;
    }
}

struct list_node* list_get_end(List *ls){
    if (ls->head != ls->tail) {
        return ls->tail;
    } else {
        return NULL;
    }
}

struct list_node* list_get_next(List *ls, struct list_node* ln){
    /*其实这里用不到ls 就当是防呆吧*/
    if(ln != ls->tail){
        return ln->next;
    }
    else{
        return NULL;
    }
}

struct list_node* list_get_prev(List *ls, struct list_node* ln){
    if(ln->prev!= ls->head){
        return ln->prev;
    }
    else{
        return NULL;
    }
}

/*把ln1移动到ln2后面*/
int list_move_to(List *ls,struct list_node* ln1,struct list_node* ln2){
    if(ln1->next != NULL){
        ln1->prev->next = ln1->next;
        ln1->next->prev = ln1->prev;
    }
    else{
        ln1->prev->next = NULL;
        ls->tail = ln.prev;
    }
    if(ln2->next != NULL){
        ln1->next = ln2->next;
        ln1->prev = ln2;
        ln2->next->prev = ln1;
        ln2->next = ln1;
    }
    else{
        ln1->next = NULL;
        ln1->prev = ln2;
        ln2->next = ln1;
        ls->tail = ln1;
    }
}

int list_delete(List * ls,struct list_node *ln){
    if(ln->next != NULL){
        ln->prev->next = ln->next;
        ln->next->prev = ln->prev;
    }
    else{
        ln->prev->next = NULL;
        ls->tail = ln->prev;
    }

    free(ln);
}