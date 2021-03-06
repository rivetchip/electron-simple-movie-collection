#include "toolbar.h"
#include "widgets.h"
#include "stddef.h"
#include "stdbool.h"

// type definition
struct _WidgetToolbar {
    GtkBox parent_instance;

    GtkWidget *button_open;
    GtkWidget *button_save;
    GtkWidget *button_new;
    GtkWidget *sourcesbox;
};

enum {
    SIGNAL_SOURCE,
	SIGNAL_LAST
};
static int signals[SIGNAL_LAST];

G_DEFINE_TYPE(WidgetToolbar, widget_toolbar, GTK_TYPE_BOX);

static GtkWidget *create_button(const char *icon_name, const char *label, const char *action_name);
static GtkWidget *create_source_radio(char *source_name, const char *label, GCallback gcallback, gpointer user_data);
// signals
static void signal_source_changed(GtkToggleButton *togglebutton, WidgetToolbar *toolbar);


WidgetToolbar *widget_toolbar_new() {
    g_message(__func__);

    return g_object_new(widget_toolbar_get_type(),
        "spacing", 6,
    NULL);
}

static void widget_toolbar_class_init(WidgetToolbarClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    signals[SIGNAL_SOURCE] = g_signal_new("source",
        G_OBJECT_CLASS_TYPE(object_class),
        G_SIGNAL_RUN_FIRST,
        0, // offset
        NULL, NULL, NULL, //accumulator, accu_data, c_marshaller
        G_TYPE_NONE, // return
        1, G_TYPE_STRING // params
    );
}

static void widget_toolbar_init(WidgetToolbar *widget) {

    widget_add_class(GTK_WIDGET(widget), "toolbar");

    gtk_widget_set_size_request(GTK_WIDGET(widget), -1, 45); // width height

    // main buttons
    GtkWidget *button_open = create_button("@toolbar-open", "Ouvrir", "win.open");
    gtk_box_pack_start(GTK_BOX(widget), button_open, false, false, 0);
    widget->button_open = button_open;

    GtkWidget *button_save = create_button("@toolbar-save", "Enregistrer", "win.save");
    gtk_box_pack_start(GTK_BOX(widget), button_save, false, false, 0);
    widget->button_save = button_save;

    GtkWidget *button_new = create_button("@toolbar-newitem", "Nouveau", "win.newitem");
    gtk_box_pack_start(GTK_BOX(widget), button_new, false, false, 0);
    widget->button_new = button_new;

    // add movie sources selection

    GtkWidget *sourcesbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *tmdben = create_source_radio("tmdb-en", "TMDb EN", G_CALLBACK(signal_source_changed), widget);
    GtkWidget *tmdbfr = create_source_radio("tmdb-fr", "TMDb FR", G_CALLBACK(signal_source_changed), widget);

    gtk_radio_button_join_group(GTK_RADIO_BUTTON(tmdbfr), GTK_RADIO_BUTTON(tmdben)); //radio,source
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmdbfr), true);

    gtk_container_add(GTK_CONTAINER(sourcesbox), tmdben);
    gtk_container_add(GTK_CONTAINER(sourcesbox), tmdbfr);

    widget->sourcesbox = sourcesbox;

    gtk_box_pack_end(GTK_BOX(widget), sourcesbox, false, false, 0);
}



static GtkWidget *create_button(const char *icon_name, const char *label, const char *action_name) {
    
    GtkWidget *button = gtk_button_new_from_icon_name(
        icon_name, GTK_ICON_SIZE_LARGE_TOOLBAR
    );
    gtk_button_set_label(GTK_BUTTON(button), label);
    widget_add_class(button, "toolbar-button");
    gtk_button_set_always_show_image(GTK_BUTTON(button), true);

    // g_signal_connect(button, "clicked", gcallback, user_data);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button), action_name);

    return button;
}

static GtkWidget *create_source_radio(char *source_name, const char *label, GCallback gcallback, gpointer user_data) {
    
    GtkWidget *button = gtk_radio_button_new_with_label(NULL, label);
    widget_add_class(button, "toolbar-button");
    widget_add_class(button, "toolbar-source");
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button), false);

    g_object_set_data(G_OBJECT(button), "source", source_name);

    g_signal_connect(button, "toggled", gcallback, user_data);

    return button;
}

static void signal_source_changed(GtkToggleButton *togglebutton, WidgetToolbar *toolbar) {
    bool is_active;
    if((is_active = gtk_toggle_button_get_active(togglebutton))) {
        const char *source_name = g_object_get_data(G_OBJECT(GTK_WIDGET(togglebutton)), "source");
        g_signal_emit(toolbar, signals[SIGNAL_SOURCE], 0, source_name);
    }
}
