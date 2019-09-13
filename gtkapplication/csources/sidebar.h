#pragma once

#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#ifdef  __cplusplus
    extern "C" {
#endif

// type definition
typedef struct _WidgetSidebar WidgetSidebar;
typedef GtkBoxClass WidgetSidebarClass;

// public api
GType widget_sidebar_get_type(void) G_GNUC_CONST;

// macro function G_DECLARE_FINAL_TYPE
inline WidgetSidebar *WIDGET_SIDEBAR(gpointer ptr) {
    return G_TYPE_CHECK_INSTANCE_CAST(ptr, widget_sidebar_get_type(), WidgetSidebar);
}

// public functions
WidgetSidebar *widget_sidebar_new();
WidgetSidebar *movie_application_new_sidebar();
// items
void widget_sidebar_listbox_bind_model(WidgetSidebar *sidebar, GListModel *model, GtkListBoxCreateWidgetFunc create_widget_func, gpointer user_data, GDestroyNotify user_data_free_func);
GtkWidget *widget_sidebar_listbox_widget(WidgetSidebar *sidebar, GSequenceIter *iter, char *title, bool is_favorite);

#ifdef  __cplusplus
    } // extern c
#endif
