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
MoviesList *movies_list_new();

GSequenceIter *movies_list_append(MoviesList *list, Movie *movie);
GSequenceIter *movies_list_insert(MoviesList *list, Movie *movie, unsigned int position);
GSequenceIter *movies_list_append_sorted(MoviesList *list, Movie *movie, GCompareDataFunc compare_func, gpointer user_data);

bool movies_list_remove(MoviesList *list, GSequenceIter *iter);
bool movies_list_remove_all(MoviesList *list);

bool movies_list_sort(MoviesList *list, GCompareDataFunc compare_func, gpointer user_data);
bool movies_list_stream(MoviesList *list, FILE *stream, GError **error);



#ifdef  __cplusplus
    } // extern c
#endif
