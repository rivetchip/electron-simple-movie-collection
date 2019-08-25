#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <glib.h>

#ifdef  __cplusplus
    extern "C" {
#endif

// type definition
typedef struct _MoviesTable MoviesTable;

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

MoviesTable *movie_collection_new();
MoviesTable *movie_collection_new_from_stream(FILE *stream, GError **error);

struct Movie *movie_collection_get(MoviesTable *table, char *movieId);
bool movie_collection_add(MoviesTable *table, const char *movieId, struct Movie *movie);
bool movie_collection_remove(MoviesTable *table, char *movieId);
bool movie_collection_destroy(MoviesTable *table);

void movie_collection_foreach(MoviesTable *table, void (*foreach)(const char*, struct Movie*, void*), void *user_data);
char *movie_collection_stringify(MoviesTable *table);



#ifdef  __cplusplus
    } // extern c
#endif
