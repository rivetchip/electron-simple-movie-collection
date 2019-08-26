//Adaptation of Edd Mann's implementation of a simple vector
//http://eddmann.com/posts/implementing-a-dynamic-vector-array-in-c/
#include "vector.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

// type definition
struct _vector {
    void **items;
    unsigned int capacity;
    unsigned int total;
};

vector *vector_new() {
    vector *v = malloc(sizeof(*v));
    v->capacity = 20;
    v->total = 0;
    v->items = malloc(sizeof(void *) * v->capacity);
    return v;
}

int vector_total(vector *v) {
    return v->total;
}

static int vector_resize(vector *v, int capacity) {
    printf("vector_resize: %d to %d\n", v->capacity, capacity);

    void **items;
    if((items = realloc(v->items, sizeof(void*) * capacity)) != NULL) {
        v->items = items;
        v->capacity = capacity;
        return true;
    }
    return false;
}

bool vector_add(vector *v, void *item) {
    if(v->capacity == v->total) {
        if(!vector_resize(v, v->capacity * 2)) {
            return false;
        }
    }
    v->items[v->total++] = item;

    // *index = (v->total - 1); // return last key

    return true;
}

bool vector_set(vector *v, int index, void *item) {
    if(index >= 0 && index < v->total) {
        v->items[index] = item;
        return true;
    }
    return false;
}

void *vector_get(vector *v, int index) {
    if(index >= 0 && index < v->total) {
        return v->items[index];
    }
    return NULL;
}

bool vector_delete(vector *v, int index) {
    if(index < 0 || index >= v->total) {
        return false;
    }

    v->items[index] = NULL;

    for(int i = index; i < v->total - 1; i++) {
        v->items[i] = v->items[i + 1];
        v->items[i + 1] = NULL;
    }

    v->total--;

    // if quarter full the contents is reallocated to a vector of half the current size
    if(v->total > 0 && v->total == v->capacity / 4) {
        if(!vector_resize(v, v->capacity / 2)) {
            return false;
        }
    }
    return true;
}

void vector_free(vector *v) {
    free(v->items);
    free(v);

    // while(v->total > 0){
    //     free(v->items[--v->size]);
    // }
}
