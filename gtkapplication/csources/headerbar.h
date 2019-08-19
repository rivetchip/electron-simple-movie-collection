#pragma once

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"
#include "window.h"

#ifdef  __cplusplus
    extern "C" {
#endif


// type definition
typedef struct _WidgetHeaderbar WidgetHeaderbar;
typedef GtkHeaderBarClass WidgetHeaderbarClass;

// public api
GType widget_headerbar_get_type(void) G_GNUC_CONST;

// macro function G_DECLARE_FINAL_TYPE
inline WidgetHeaderbar *WIDGET_HEADERBAR(gpointer ptr) {
    return G_TYPE_CHECK_INSTANCE_CAST(ptr, widget_headerbar_get_type(), WidgetHeaderbar);
}

// public functions
WidgetHeaderbar *widget_headerbar_new();
WidgetHeaderbar *movie_appplication_new_headerbar(MovieApplication *app, MovieWindow *window);



#ifdef  __cplusplus
    } // extern c
#endif
