#include "headerbar.h"
#include "widgets.h"
#include <stdbool.h>

// type definition
struct _WidgetHeaderbar {
    GtkHeaderBar parent_instance;

    GtkWidget *button_close;
    GtkWidget *button_minimize;
    GtkWidget *button_maximize;
};

G_DEFINE_TYPE(WidgetHeaderbar, widget_headerbar, GTK_TYPE_HEADER_BAR);

// menu button
static GtkWidget *widget_gearbutton_new(WidgetHeaderbar *widget);


WidgetHeaderbar *widget_headerbar_new() {
    g_message(__func__);

    return g_object_new(widget_headerbar_get_type(),
        "spacing", 6,
        "border-width", 0,
    NULL);
}

static void widget_headerbar_class_init(WidgetHeaderbarClass *klass) {
    // GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
}

static void widget_headerbar_init(WidgetHeaderbar *widget) {
    // set GTK CSD HeaderBar
    widget_add_class(GTK_WIDGET(widget), "headerbar");

    // hide window decorationq of header bar
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(widget), false);
    gtk_header_bar_set_title(GTK_HEADER_BAR(widget), "Movie Collection");
    gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(widget), false);
    gtk_widget_set_size_request(GTK_WIDGET(widget), -1, 45); // w,h

    // add buttons and callback on click (override gtk-decoration-layout property)

    GtkWidget *button_close = gtk_button_new_from_icon_name("@window-close", GTK_ICON_SIZE_BUTTON);
    widget_add_class(button_close, "headerbutton");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button_close), "win.close");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(widget), button_close);

    widget->button_close = button_close;

    GtkWidget *button_minimize = gtk_button_new_from_icon_name("@window-minimize", GTK_ICON_SIZE_BUTTON);
    widget_add_class(button_minimize, "headerbutton");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button_minimize), "win.minimize");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(widget), button_minimize);

    widget->button_minimize = button_minimize;

    GtkWidget *button_maximize = gtk_button_new_from_icon_name("@window-maximize", GTK_ICON_SIZE_BUTTON);
    widget_add_class(button_maximize, "headerbutton");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button_maximize), "win.maximize");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(widget), button_maximize);

    widget->button_maximize = button_maximize;

    // options gear button
    GtkWidget *button_gear = widget_gearbutton_new(widget);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(widget), button_gear);
}

static GtkWidget *widget_gearbutton_new(WidgetHeaderbar *widget) {

    GtkWidget *button_gear = gtk_menu_button_new();
    widget_add_class(button_gear, "headerbutton");
    gtk_widget_set_tooltip_text(button_gear, "Menu");

    // GtkWidget *popover_menu = gtk_popover_new(button_gear);
    // gtk_menu_button_set_popover(GTK_MENU_BUTTON(button_gear), popover_menu);

    GMenu *menu = g_menu_new();
    g_menu_append(menu, "Préférences", "win.prefs");
    g_menu_append(menu, "Raccourcis clavier", "win.shortcuts");
    g_menu_append(menu, "À propos", "win.about");

    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(button_gear), G_MENU_MODEL(menu));

    return button_gear;
}

