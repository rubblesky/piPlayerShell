#define ERROR_ALLOC -1
#define ERROR_POSITION -2


struct list_node {
    struct list_node* next;
    struct list_node* prev;
    void* element;
};

struct link_list {
    struct list_node* head;
    struct list_node* tail;
};
typedef struct link_list List;

List* init_list();
void free_list(List* ls);
/*在尾部增加节点*/
int list_append(List* ls, void* element);
/*在ln位置之前增加节点*/
int list_insert(List* ls, struct list_node* ln, void* element);

struct list_node* list_get_first(List* ls);
struct list_node* list_get_end(List* ls);
struct list_node* list_get_next(List* ls, struct list_node* ln);
struct list_node* list_get_prev(List* ls, struct list_node* ln);
void list_move_to(List* ls, struct list_node* ln1, struct list_node* ln2);
void list_delete(List* ls, struct list_node* ln);
void list_swap(List* ls, struct list_node* ln1, struct list_node* ln2);