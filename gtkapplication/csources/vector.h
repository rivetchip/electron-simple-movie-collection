#pragma once

#include <stdbool.h>

#ifdef  __cplusplus
    extern "C" {
#endif

// type definition
typedef struct _vector vector;

// public functions
vector *vector_new();
int vector_total(vector *v);
bool vector_add(vector *v, void *item);
bool vector_set(vector *v, int index, void *item);
void *vector_get(vector *v, int index);
bool vector_delete(vector *v, int index);
void vector_free(vector *v);


#ifdef  __cplusplus
    } // extern c
#endif
