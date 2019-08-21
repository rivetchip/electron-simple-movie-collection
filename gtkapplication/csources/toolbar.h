#pragma once

#include <glib.h>
#include <gtk/gtk.h>

#include "window.h"

#ifdef  __cplusplus
    extern "C" {
#endif

// typedef MovieWindow; @todo

// type definition
typedef struct _WidgetToolbar WidgetToolbar;
typedef GtkBoxClass WidgetToolbarClass;

// public api
GType widget_toolbar_get_type(void) G_GNUC_CONST;

// macro function G_DECLARE_FINAL_TYPE
inline WidgetToolbar *WIDGET_TOOLBAR(gpointer ptr) {
    return G_TYPE_CHECK_INSTANCE_CAST(ptr, widget_toolbar_get_type(), WidgetToolbar);
}

// public functions
WidgetToolbar *widget_toolbar_new();
WidgetToolbar *movie_appplication_new_toolbar(MovieWindow *window);



#ifdef  __cplusplus
    } // extern c
#endif
