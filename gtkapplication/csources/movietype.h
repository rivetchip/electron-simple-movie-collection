#pragma once

#include <glib.h>
#include <gio/gio.h>
#include <stdbool.h>

#ifdef  __cplusplus
    extern "C" {
#endif


// type definition

struct _Movie {
    GObject parent_instance;
    bool widget_visible;

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

// type definition
typedef struct _Movie Movie;
typedef struct _MovieClass MovieClass;

// public api
GType movie_get_type(void) G_GNUC_CONST;

// macro function G_DECLARE_FINAL_TYPE
inline Movie *MOVIE(gpointer ptr) {
    return G_TYPE_CHECK_INSTANCE_CAST(ptr, movie_get_type(), Movie);
}

// public functions
Movie *movie_new();

bool movie_is_visible(Movie *movie);
void movie_set_visible(Movie *movie, bool visible);

void movie_destroy(Movie *movie);





#ifdef  __cplusplus
    } // extern c
#endif
