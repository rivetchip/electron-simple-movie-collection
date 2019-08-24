#include "toolbar.h"
#include "widgets.h"

// type definition
struct _WidgetToolbar {
    GtkBox parent_instance;

    GtkWidget *button_open;
    GtkWidget *button_save;
    GtkWidget *button_new;
    GtkWidget *providers;
};

G_DEFINE_TYPE(WidgetToolbar, widget_toolbar, GTK_TYPE_BOX);


static void widget_toolbar_init(WidgetToolbar *widget) {
    g_message(__func__);
}

static void widget_toolbar_class_init(WidgetToolbarClass *klass) {
	// GObjectClass *object_class = G_OBJECT_CLASS(klass);
	// GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
}

WidgetToolbar *widget_toolbar_new() {
    return g_object_new(widget_toolbar_get_type(), NULL);
}

WidgetToolbar *movie_appplication_new_toolbar() {

    WidgetToolbar *widget = widget_toolbar_new();
    widget_add_class(GTK_WIDGET(widget), "toolbar");

    gtk_widget_set_size_request(GTK_WIDGET(widget), -1, 45); // width height

    // main buttons

    GtkWidget *button_open = gtk_button_new_from_icon_name(
        "@toolbar-open", GTK_ICON_SIZE_LARGE_TOOLBAR
    );
    gtk_button_set_label(GTK_BUTTON(button_open), "Ouvrir");
    widget_add_class(button_open, "toolbar-button");
    gtk_button_set_always_show_image(GTK_BUTTON(button_open), TRUE);

    widget->button_open = button_open;

    GtkWidget *button_save = gtk_button_new_from_icon_name(
        "@toolbar-save", GTK_ICON_SIZE_LARGE_TOOLBAR
    );
    gtk_button_set_label(GTK_BUTTON(button_save), "Enregistrer");
    widget_add_class(button_save, "toolbar-button");
    gtk_button_set_always_show_image(GTK_BUTTON(button_save), TRUE);

    widget->button_save = button_save;

    GtkWidget *button_new = gtk_button_new_from_icon_name(
        "@toolbar-new", GTK_ICON_SIZE_LARGE_TOOLBAR
    );
    gtk_button_set_label(GTK_BUTTON(button_new), "Ajouter un film");
    widget_add_class(button_new, "toolbar-button");
    gtk_button_set_always_show_image(GTK_BUTTON(button_new), TRUE);

    widget->button_new = button_new;

    gtk_box_pack_start(GTK_BOX(widget), button_open, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(widget), button_save, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(widget), button_new, FALSE, FALSE, 0);


    // add movie provider selection todo

    GtkWidget *providers = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    widget->providers = providers;

    GtkWidget *tmdben = gtk_radio_button_new_with_label(NULL, "TMDb EN");
    widget_add_class(tmdben, "toolbar-button");
    widget_add_class(tmdben, "toolbar-provider");
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(tmdben), FALSE);
    // g_signal_connect(tmdben, "toggled", G_CALLBACK(signal_toolbar_provider_change), "tmdb-en");

    GtkWidget *tmdbfr = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(tmdben), "TMDb FR");
    widget_add_class(tmdbfr, "toolbar-button");
    widget_add_class(tmdbfr, "toolbar-provider");
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(tmdbfr), FALSE);
    // g_signal_connect(tmdbfr, "toggled", G_CALLBACK(signal_toolbar_provider_change), "tmdb-fr");

    gtk_container_add(GTK_CONTAINER(providers), tmdben);
    gtk_container_add(GTK_CONTAINER(providers), tmdbfr);

    gtk_box_pack_end(GTK_BOX(widget), providers, FALSE, FALSE, 0);

    return widget;
}







/*


//todo: interac with window instead

static void signal_toolbar_open(GtkButton *button, gpointer user_data) {

    const char *filename = dialog_file_chooser(NULL);

    if(filename == NULL) {
        return;
    }

    // struct MovieCollection *storageMovies = NULL;

    GError *error = NULL;
    FILE *stream = fopen(filename, "rb");

    if(!movie_collection_new_from(stream, &error)) {

        #if PACKAGE_DEVELOPER_MODE
            g_message("%s %s", __func__, error->message);
        #endif
        
        GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(button));
        dialog_message(GTK_WINDOW(toplevel), "Could not open file", error->message);
    
        g_clear_error(&error); //todo check
    }

    fclose(stream);
    g_free((char*) filename);
}

static void signal_toolbar_save(GtkButton *button, gpointer user_data) {

    const char *filename = show_save_dialog(NULL);

    if(filename == NULL) {
        return;
    }

    GError *error = NULL;
    bool save = movie_collection_save(filename, &error);


//todo if already set

g_message("filename %s", filename);
}

static void signal_toolbar_new(GtkButton *button, gpointer user_data) {

//todo
}


static void signal_toolbar_provider_change(GtkToggleButton *togglebutton, char *provider_name) {
    bool is_active = gtk_toggle_button_get_active(togglebutton);
    
    #if PACKAGE_DEVELOPER_MODE
        g_message("%s %s %s", __func__, provider_name, (is_active?"on":"off"));
    #endif



}

*/