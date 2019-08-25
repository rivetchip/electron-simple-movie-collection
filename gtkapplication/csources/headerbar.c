#include "headerbar.h"
#include "widgets.h"

// type definition
struct _WidgetHeaderbar {
    GtkHeaderBar parent_instance;

    GtkWidget *button_close;
    GtkWidget *button_minimize;
    GtkWidget *button_maximize;
};

G_DEFINE_TYPE(WidgetHeaderbar, widget_headerbar, GTK_TYPE_HEADER_BAR);

static void signal_close(GtkButton *button);
static void signal_minimize(GtkButton *button);
static void signal_maximize(GtkButton *button);


static void widget_headerbar_init(WidgetHeaderbar *widget) {
    g_message(__func__);
}

static void widget_headerbar_class_init(WidgetHeaderbarClass *klass) {
	// GObjectClass *object_class = G_OBJECT_CLASS(klass);
	// GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
}

WidgetHeaderbar *widget_headerbar_new() {
    return g_object_new(widget_headerbar_get_type(), NULL);
}

WidgetHeaderbar *movie_application_new_headerbar() {
    // set GTK CSD HeaderBar
    WidgetHeaderbar *widget = widget_headerbar_new();
    widget_add_class(GTK_WIDGET(widget), "headerbar");

    // hide window decorationq of header bar
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(widget), FALSE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(widget), "Movie Collection");
    gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(widget), FALSE);
    gtk_widget_set_size_request(GTK_WIDGET(widget), -1, 45); // w,h

    // add buttons and callback on click (override gtk-decoration-layout property)

    GtkWidget *button_close = gtk_button_new_from_icon_name("@window-close", GTK_ICON_SIZE_BUTTON);
    widget_add_class(button_close, "headerbutton");
    g_signal_connect(button_close, "clicked", G_CALLBACK(signal_close), NULL);

    widget->button_close = button_close;

    GtkWidget *button_minimize = gtk_button_new_from_icon_name("@window-minimize", GTK_ICON_SIZE_BUTTON);
    widget_add_class(button_minimize, "headerbutton");
    g_signal_connect(button_minimize, "clicked", G_CALLBACK(signal_minimize), NULL);

    widget->button_minimize = button_minimize;

    GtkWidget *button_maximize = gtk_button_new_from_icon_name("@window-maximize", GTK_ICON_SIZE_BUTTON);
    widget_add_class(button_maximize, "headerbutton");
    g_signal_connect(button_maximize, "clicked", G_CALLBACK(signal_maximize), NULL);

    widget->button_maximize = button_maximize;

    gtk_header_bar_pack_start(GTK_HEADER_BAR(widget), button_close);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(widget), button_minimize);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(widget), button_maximize);

    // options gear button
    GtkWidget *button_menu = gtk_menu_button_new();
    widget_add_class(button_menu, "headerbutton");

    GtkWidget *popover = gtk_popover_new(button_menu);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(button_menu), popover);

    // gtk_container_add(GTK_CONTAINER(popover), xxx);
    // gtk_widget_show_all(xxx); //todo

    gtk_header_bar_pack_end(GTK_HEADER_BAR(widget), button_menu);




    return widget;
}

static void signal_close(GtkButton *button) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if(GTK_IS_WINDOW(window)) {
        gtk_window_close(GTK_WINDOW(window));
    }
}

static void signal_minimize(GtkButton *button) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if(GTK_IS_WINDOW(window)) {
        gtk_window_iconify(GTK_WINDOW(window));
    }
}

static void signal_maximize(GtkButton *button) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if(GTK_IS_WINDOW(window)) {
        gtk_window_is_maximized(GTK_WINDOW(window)) ? gtk_window_unmaximize(GTK_WINDOW(window)) : gtk_window_maximize(GTK_WINDOW(window));
    }
}
