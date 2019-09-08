#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <glib.h>

#ifdef  __cplusplus
    extern "C" {
#endif

// type definition
typedef struct _MoviesList MoviesList;

struct Movie {
    const char *movieId;

    char *title;
    bool favorite;
    int rating; // /100
    char *tagline;
    char *originalTitle;
    int ratingPress;
    int duration; // minutes
    char *dateReleased;
    char *dateCreated;
    char *dateModified;
    char *poster;
    char *description;
    char *comment;
    char *director;
    char **countries; // array[string]
    char **genres; // array[string]
    char **actors; // array[string][2]
    char *serie;
    char **companies; // array[string]
    char **keywords; // array[string]
    char *source;
    int sourceId;
    char *webPage;
};

// public functions
MoviesList *movies_list_new();
MoviesList *movies_list_new_from_stream(FILE *stream, GError **error);

int movies_list_total(MoviesList *l);
struct Movie *movies_list_get(MoviesList *l, int index);

bool movies_list_add(MoviesList *l, struct Movie *movie, int *index);
bool movies_list_set(MoviesList *l, int index, struct Movie *movie);
bool movies_list_delete(MoviesList *l, int index);
void movies_list_free(MoviesList *l);



#ifdef  __cplusplus
    } // extern c
#endif
