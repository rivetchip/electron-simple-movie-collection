#pragma once

#include <glib.h>
#include <gio/gio.h>

#include "movietype.h"

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef  __cplusplus
    extern "C" {
#endif

// type definition
typedef struct _MoviesList MoviesList;
typedef struct _MoviesListClass MoviesListClass;

// public api
GType movies_list_get_type(void) G_GNUC_CONST;

// public functions
MoviesList *movies_list_new();

// macro function G_DECLARE_FINAL_TYPE
inline MoviesList *MOVIES_LIST(gpointer ptr) {
    return G_TYPE_CHECK_INSTANCE_CAST(ptr, movies_list_get_type(), MoviesList);
}

// public functions
unsigned int movies_list_total(MoviesList *list);
Movie *movies_list_get(MoviesList *list, unsigned int index);

bool movies_list_add(MoviesList *list, Movie *movie, unsigned int *index);
bool movies_list_set(MoviesList *list, unsigned int index, Movie *movie);
bool movies_list_remove(MoviesList *list, unsigned int index);
bool movies_list_remove_all(MoviesList *list);
void movies_list_destroy(MoviesList *list);

bool movies_list_foreach(MoviesList *list, unsigned int *index, Movie **movie);

bool movies_list_from_stream(MoviesList *list, FILE *stream, GError **error);




//todo: while
// char *movies_list_stringify(MoviesList *list);

//todo: use size_t everywhere



#ifdef  __cplusplus
    } // extern c
#endif
