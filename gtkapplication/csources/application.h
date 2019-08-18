#pragma once

#include <stdbool.h>

#include <glib.h>
#include <gtk/gtk.h>

#ifdef  __cplusplus
    extern "C" {
#endif


// type definition
struct _MovieApplication {
    GtkApplication parent_instance;

    GNetworkMonitor *monitor;

};

typedef struct _MovieApplication MovieApplication;
typedef GtkApplicationClass MovieApplicationClass;

// public api
GType movie_application_get_type(void) G_GNUC_CONST;

// macro function G_DECLARE_FINAL_TYPE
inline MovieApplication *MOVIE_APPLICATION(gpointer ptr) {
    return G_TYPE_CHECK_INSTANCE_CAST(ptr, movie_application_get_type(), MovieApplication);
}

// public functions
MovieApplication *movie_application_new(const char *application_id, GApplicationFlags flags);



#ifdef  __cplusplus
    } // extern c
#endif
