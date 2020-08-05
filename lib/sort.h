#ifndef SORT_H
#define SORT_H
/*快速排序
    compare函数有三个返回值，1，0，-1
*/
void quick_sort(void *array, int p, int r,void(*compare)(void *,int,int), void (*exchange)(void *, int, int));


#endif