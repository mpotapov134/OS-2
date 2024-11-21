#ifndef MAP_H
#define MAP_H

#include <pthread.h>
#include "mythread.h"

#define MAP_SIZE 256 // Size of the hash table

// Node structure for linked list
typedef struct Node {
    int key;
    void* value;
    struct Node* next;
} Node;

// Map structure
typedef struct Map {
    Node** table; // Hash table
    pthread_mutex_t mutex;
} Map;

Map* mapCreate();
void mapDestroy(Map* map);
void* mapGet(Map* map, int key);
int mapAdd(Map* map, int key, void* value);
int mapRemove(Map* map, int key);

#endif
