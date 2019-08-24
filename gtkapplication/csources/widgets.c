#include "widgets.h"


void widget_add_class(GtkWidget *widget, const char *classname) {
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), classname);
}

