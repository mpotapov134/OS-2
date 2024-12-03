#ifndef LIST_SPIN
#define LIST_SPIN

#include <pthread.h>

#define STR_SIZE            100
#define STORAGE_SIZE        100000

typedef struct _Node {
    char value[STR_SIZE];
    struct _Node* next;
    pthread_spinlock_t lock;
} Node;

typedef struct _Storage {
    Node *first;
    pthread_spinlock_t lock; // используется, когда обращаемся не к элементам списка, а к самой структуре списка
} Storage;

int getRandomNumber(int min, int max);

Storage *listInit();
void listDestroy(Storage *list);
void listAdd(Storage *list, const char value[]);

#endif /* LIST_SPIN */
