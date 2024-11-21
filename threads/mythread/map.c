#include <stdio.h>
#include <stdlib.h>

#include "map.h"

unsigned long intHash(int key) {
    return (unsigned long)(key % MAP_SIZE);
}

Map* mapCreate() {
    Map* map = (Map*)malloc(sizeof(Map));
    if (!map) return NULL;

    map->table = (Node**)calloc(MAP_SIZE, sizeof(Node*));
    if (!map->table) {
        free(map);
        return NULL;
    }

    pthread_mutex_init(&map->mutex, NULL);
    return map;
}

void mapDestroy(Map* map) {
    if (!map) return;

    pthread_mutex_lock(&map->mutex);

    for (int i = 0; i < MAP_SIZE; i++) {
        Node* current = map->table[i];
        while (current) {
            Node* temp = current;
            current = current->next;
            free(temp);
        }
    }

    free(map->table);
    pthread_mutex_unlock(&map->mutex);
    pthread_mutex_destroy(&map->mutex);
    free(map);
}

void* mapGet(Map* map, int key) {
    if (!map) return NULL;

    pthread_mutex_lock(&map->mutex);

    unsigned long index = intHash(key);
    Node* current = map->table[index];

    while (current) {
        if (current->key == key) {
            pthread_mutex_unlock(&map->mutex);
            return current->value; // Return the value found
        }
        current = current->next;
    }

    pthread_mutex_unlock(&map->mutex);
    return NULL; // Key not found
}

int mapAdd(Map* map, int key, void* value) {
    if (!map) return -1;

    pthread_mutex_lock(&map->mutex);

    unsigned long index = intHash(key);
    Node* current = map->table[index];

    // Check if the key already exists
    while (current) {
        if (current->key == key) {
            current->value = value; // Update the value if key exists
            pthread_mutex_unlock(&map->mutex);
            return 0; // Success
        }
        current = current->next;
    }

    // Key does not exist, create a new node
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        pthread_mutex_unlock(&map->mutex);
        return -1; // Memory allocation failure
    }

    newNode->key = key; // Set the integer key
    newNode->value = value;
    newNode->next = map->table[index]; // Insert at the beginning of the list
    map->table[index] = newNode;

    pthread_mutex_unlock(&map->mutex);
    return 0; // Success
}

int mapRemove(Map* map, int key) {
    if (!map) return -1;

    pthread_mutex_lock(&map->mutex);

    unsigned long index = intHash(key);
    Node* current = map->table[index];
    Node* previous = NULL;

    while (current) {
        if (current->key == key) {
            // Found the node to remove
            if (previous) {
                previous->next = current->next; // Bypass the current node
            } else {
                map->table[index] = current->next; // Update head of the list
            }
            free(current); // Free the node
            pthread_mutex_unlock(&map->mutex);
            return 0; // Success
        }
        previous = current;
        current = current->next;
    }

    pthread_mutex_unlock(&map->mutex);
    return -1; // Key not found
}
