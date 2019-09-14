#include "statusbar.h"
#include "widgets.h"

// type definition
struct _WidgetStatusbar {
    GtkBox parent_instance;

    GtkWidget *label;
};

G_DEFINE_TYPE(WidgetStatusbar, widget_statusbar, GTK_TYPE_BOX);


static void widget_statusbar_init(WidgetStatusbar *widget) {
    //
}

static void widget_statusbar_class_init(WidgetStatusbarClass *klass) {
    // GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
}

WidgetStatusbar *widget_statusbar_new() {
    g_message(__func__);

    return g_object_new(widget_statusbar_get_type(),
        "orientation", GTK_ORIENTATION_HORIZONTAL,
    NULL);
}

WidgetStatusbar *movie_application_new_statusbar() {

    WidgetStatusbar *widget = widget_statusbar_new();
    widget_add_class(GTK_WIDGET(widget), "statusbar");

    GtkWidget *label = gtk_label_new("");
    widget_add_class(label, "statusbar-message");
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);

    // align left, verticial center
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);

    gtk_widget_set_size_request(GTK_WIDGET(widget), -1, 30); // width height
    gtk_container_add(GTK_CONTAINER(widget), label);

    widget->label = label;

    return widget;
}

void widget_statusbar_set_text(WidgetStatusbar *statusbar, const char *text) {
    gtk_label_set_text(GTK_LABEL(statusbar->label), text);
}

