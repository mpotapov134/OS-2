#ifndef LIST_MUTEX
#define LIST_MUTEX

#include <pthread.h>

#define STR_SIZE            100
#define STORAGE_SIZE        100

typedef struct _Node {
    char value[STR_SIZE];
    struct _Node* next;
    pthread_mutex_t lock;
} Node;

typedef struct _Storage {
    Node *first;
    pthread_mutex_t lock; // используется, когда обращаемся не к элементам списка, а к самой структуре списка
} Storage;

int getRandomNumber(int min, int max);

Storage *listInit();
void listDestroy(Storage *list);
void listAdd(Storage *list, const char value[]);

#endif /* LIST_MUTEX */
