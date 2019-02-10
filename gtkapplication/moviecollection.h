#pragma once

#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>

G_BEGIN_DECLS


// Type definition
struct _MovieApplication {
    GtkApplication parent_instance;

    GtkWidget *listbox;

    // window state
    int win_height;
    int win_width;
    bool is_maximized;
    bool is_fullscreen;
};

typedef struct _MovieApplication MovieApplication;
typedef GtkApplicationClass MovieApplicationClass;

// Public API
GType movie_application_get_type(void) G_GNUC_CONST;

// Macro Functions G_DECLARE_FINAL_TYPE
static inline MovieApplication *MOVIE_APPLICATION(gpointer ptr) {
    return G_TYPE_CHECK_INSTANCE_CAST(ptr, movie_application_get_type(), MovieApplication);
}

// internal API
static MovieApplication *movie_application_new(const char *application_id, GApplicationFlags flags);
static void movie_application_init(MovieApplication *app);
static void movie_application_class_init(MovieApplicationClass *class);




////////////////////

struct _WidgetMovieItem {
    GtkListBoxRow parent_instance;

    char *movie_id;
    char *movie_title;
    bool movie_favorite;
};

typedef struct _WidgetMovieItem WidgetMovieItem;
typedef GtkListBoxRowClass WidgetMovieItemClass;

GType widget_movie_item_get_type(void) G_GNUC_CONST;

// Macro Functions G_DECLARE_FINAL_TYPE
static inline WidgetMovieItem *WIDGET_MOVIE_ITEM(gpointer ptr) {
    return G_TYPE_CHECK_INSTANCE_CAST(ptr, widget_movie_item_get_type(), WidgetMovieItem);
}







G_END_DECLS
