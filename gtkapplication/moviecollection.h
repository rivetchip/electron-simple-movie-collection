#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>

#include "macros.h"
#include "vector.h"

#ifdef  __cplusplus
    extern "C" {
#endif




////////////////////



////////////////////

struct MoviesMetadata {
    int version;
    char *created;
    char *imported;
    char *source;
};

struct Movie {
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
    char **countries; // array
    char *genres; // array
    char **actors; // array[][2]
    char *serie;
    char **companies; // array
    char *keywords; // array
    char *source;
    int sourceId;
    char *webPage;
};

struct MoviesStorage {
    size_t size;
    struct MoviesMetadata *metadata;
    vector *movies; // array[Movie]
};



#ifdef  __cplusplus
    } // extern c
#endif
