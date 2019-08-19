#pragma once

#include <stdbool.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

#ifdef  __cplusplus
    extern "C" {
#endif


// type definition
struct _MovieWindow {
    GtkApplicationWindow parent_instance;

    // main app
    MovieApplication *movieapp;

    // window state
    int height;
    int width;
    bool is_maximized;
    bool is_fullscreen;

    // widgets
    int paned_position;
};

typedef struct _MovieWindow MovieWindow;
typedef GtkApplicationWindowClass MovieWindowClass;


// public api
GType movie_window_get_type(void) G_GNUC_CONST;

// macro function G_DECLARE_FINAL_TYPE
inline MovieWindow *MOVIE_WINDOW(gpointer ptr) {
    return G_TYPE_CHECK_INSTANCE_CAST(ptr, movie_window_get_type(), MovieWindow);
}

// public functions
MovieWindow *movie_window_new(MovieApplication *application);
MovieWindow *movie_appplication_create_window(MovieApplication *app, GdkScreen *screen);


#ifdef  __cplusplus
    } // extern c
#endif
