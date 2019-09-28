#pragma once

#include <glib.h>
#include <gtk/gtk.h>

#ifdef  __cplusplus
    extern "C" {
#endif


// type definition
typedef struct _WidgetStatusbar WidgetStatusbar;
typedef GtkBoxClass WidgetStatusbarClass;

// public api
GType widget_statusbar_get_type(void) G_GNUC_CONST;

// macro function G_DECLARE_FINAL_TYPE
inline WidgetStatusbar *WIDGET_STATUSBAR(gpointer ptr) {
    return G_TYPE_CHECK_INSTANCE_CAST(ptr, widget_statusbar_get_type(), WidgetStatusbar);
}

// public functions
WidgetStatusbar *widget_statusbar_new();

void widget_statusbar_set_text(WidgetStatusbar *statusbar, const char *text);



#ifdef  __cplusplus
    } // extern c
#endif
