#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef  __cplusplus
    extern "C" {
#endif


typedef struct vector {
    void **items;
    unsigned int capacity;
    unsigned int total;
} vector;

vector *vector_new();
int vector_total(vector *v);
int vector_add(vector *v, void *item);
int vector_set(vector *v, int index, void *item);
void *vector_get(vector *v, int index);
int vector_delete(vector *v, int index);
void vector_free(vector *v);



#ifdef  __cplusplus
    } // extern c
#endif
