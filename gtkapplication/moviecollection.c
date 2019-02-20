/*
sudo dnf install gtk3-devel gstreamer-devel clutter-devel webkitgtk3-devel libgda-devel gobject-introspection-devel
webkit2gtk3-devel
builddir > clear && ninja && GTK_DEBUG=interactive ./moviecollection --debug
gcc moviecollection.c -o main `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` && ./main
/usr/local
meson --buildtype
coredumpctl list => gdb / coredumpctl gdb
*/

#include <config.h> //build generated
#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>

#include "moviecollection.h"
#include "fetchmovie.c"


G_DEFINE_TYPE(MovieApplication, movie_application, GTK_TYPE_APPLICATION);

static void movie_application_init(MovieApplication *app) {
    g_message(__func__);
}

static void movie_application_class_init(MovieApplicationClass *class) {
    // GApplicationClass *app_class = G_APPLICATION_CLASS(class);

    // app_class->startup = demo_application_startup;
    // app_class->activate = demo_application_activate;
}

static MovieApplication *movie_application_new(const char *application_id, GApplicationFlags flags) {

    g_return_val_if_fail(g_application_id_is_valid(application_id), NULL);
    
    return g_object_new(movie_application_get_type(),
        "application-id", application_id,
        "flags", flags,
    NULL);
}


/*

static bool movie_collection_get() {

}

static bool movie_collection_add() {

}

static bool movie_collection_remove() {
    
}

*/






static void widget_add_class(GtkWidget *widget, char *class_name) {
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), class_name);
}

static char *widget_get_iconpath(char *icon_name) {
    char icon_svg[20]; //todo
    sprintf(icon_svg, "%s.svg", icon_name);

    char *icon_path = g_build_filename(PACKAGE_RESSOURCES_DIR, icon_svg, NULL);

    if(g_file_test(icon_path, G_FILE_TEST_IS_REGULAR)) {
        return icon_path;
    }

    g_free(icon_path);

    return NULL;
}

static GtkWidget *widget_get_child(GtkWidget *parent, char *child_name) {

    if(strcmp(gtk_widget_get_name(parent), child_name) == 0) { 
        return parent;
    }

    if(GTK_IS_BIN(parent)) { // container with one child
        GtkWidget *child = gtk_bin_get_child(GTK_BIN(parent));
        return widget_get_child(child, child_name);
    }

    if(GTK_IS_CONTAINER(parent)) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(parent));

        // while((children = g_list_next(children)) != NULL) {
        for ( ; children ; children = g_list_next(children)) {
            GtkWidget* widget = widget_get_child(children->data, child_name);
            if(widget != NULL) {
                return widget;
            }
        }
    }

    return NULL;
}





static void mainwindow_store_state(MovieApplication *mapp) {
    const char *appid = g_application_get_application_id(G_APPLICATION(mapp));

    GKeyFile *keyfile = g_key_file_new();

    g_key_file_set_integer(keyfile, "WindowState", "height", mapp->win_height);
    g_key_file_set_integer(keyfile, "WindowState", "width", mapp->win_width);
    g_key_file_set_boolean(keyfile, "WindowState", "maximized", mapp->is_maximized);
    g_key_file_set_boolean(keyfile, "WindowState", "fullscreen", mapp->is_fullscreen);

    // save file under $XDG_CACHE_HOME
    char *state_path = g_build_filename(g_get_user_cache_dir(), appid, NULL);
    char *state_file = g_build_filename(state_path, "state.ini", NULL);

    // create save path if not set
    if(g_mkdir_with_parents(state_path, 0755) == 0) { // error=-1 exist=0

        GError *error_save = NULL;
        if(!g_key_file_save_to_file(keyfile, state_file, &error_save)) {
            g_warning("app:window store state / %s", error_save->message);
            g_clear_error(&error_save);
        }
    }

    g_key_file_free(keyfile);
    g_free(state_path);
    g_free(state_file);
}

static void mainwindow_load_state(MovieApplication *mapp) {
    const char *appid = g_application_get_application_id(G_APPLICATION(mapp));

    char *state_file = g_build_filename(g_get_user_cache_dir(), appid, "state.ini", NULL);

    GKeyFile *keyfile = g_key_file_new();

    if(g_key_file_load_from_file(keyfile, state_file, G_KEY_FILE_NONE, NULL)) {

        GError *error_read = NULL;

        int state_height = g_key_file_get_integer(keyfile, "WindowState", "height", &error_read);
        error_read == NULL ? (mapp->win_height = state_height) : g_clear_error(&error_read);

        int state_width = g_key_file_get_integer(keyfile, "WindowState", "width", &error_read);
        error_read == NULL ? (mapp->win_width = state_width) : g_clear_error(&error_read);

        bool state_maximized = g_key_file_get_boolean(keyfile, "WindowState", "maximized", &error_read);
        error_read == NULL ? (mapp->is_maximized = state_maximized) : g_clear_error(&error_read);

        int state_fullscreen = g_key_file_get_boolean(keyfile, "WindowState", "fullscreen", &error_read);
        error_read == NULL ? (mapp->is_fullscreen = state_fullscreen) : g_clear_error(&error_read);
    }

    g_key_file_free(keyfile);
    g_free(state_file);
}

static bool signal_mainwindow_state_event(GtkWidget *window, GdkEventWindowState *event, MovieApplication *mapp) {
    GdkWindowState window_state = event->new_window_state; // event->type=GDK_WINDOW_STATE

    mapp->is_maximized = (window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;

    mapp->is_fullscreen = (window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;

    return GDK_EVENT_PROPAGATE;
}

static void signal_mainwindow_size_allocate(GtkWidget *window, GdkRectangle *allocation, MovieApplication *mapp) {
    // save the window geometry only if we are not maximized of fullscreen
    if(!(mapp->is_maximized || mapp->is_fullscreen)) {
        // use gtk_ ; Using the allocation directly can lead to growing windows with client-side decorations
        gtk_window_get_size(GTK_WINDOW(window),
            &mapp->win_width,
            &mapp->win_height
        );
    }
}

static void signal_mainwindow_destroy(GtkWidget *window) {
    GtkApplication *gtkapp = gtk_window_get_application(GTK_WINDOW(window));
    g_application_quit(G_APPLICATION(gtkapp));
}

// create a gtk button with an icon inside
static GtkWidget *app_headerbar_create_button(char *icon_name, char *class_name, void *click_event, gpointer user_data) {
    char icon_svg[20];
    sprintf(icon_svg, "%s.svg", icon_name);

    char *icon_path = g_build_filename(PACKAGE_RESSOURCES_DIR, icon_svg, NULL);

    GtkWidget *gtk_button = gtk_button_new();

    if(class_name != NULL) {
        widget_add_class(gtk_button, class_name);
    }

    if(click_event != NULL) {
        g_signal_connect(gtk_button, "clicked", G_CALLBACK(click_event), user_data);
    }

    GtkWidget *gtk_image;

    if(g_file_test(icon_path, G_FILE_TEST_IS_REGULAR)) {
        gtk_image = gtk_image_new_from_file(icon_path);
    } else {
        // default fallback picture (symbolic)
        char icon_symbolic[25];
        sprintf(icon_symbolic, "%s-symbolic", icon_name);

        gtk_image = gtk_image_new_from_icon_name(icon_symbolic, GTK_ICON_SIZE_MENU);
    }

    g_free(icon_path);

    gtk_container_add(GTK_CONTAINER(gtk_button), gtk_image);

    return gtk_button;
}

static void signal_headerbar_close(GtkButton* button, MovieApplication *mapp) {
    g_application_quit(G_APPLICATION(mapp));
    // todo remove mapp var
}

static void signal_headerbar_minimize(GtkButton* button, MovieApplication *mapp) {
    GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if(GTK_IS_WINDOW(toplevel)) {
        GtkWindow *gtk_window = GTK_WINDOW(toplevel);
        gtk_window_iconify(gtk_window);
    }
}

static void signal_headerbar_maximize(GtkButton* button, MovieApplication *mapp) {
    GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if(GTK_IS_WINDOW(toplevel)) {
        GtkWindow *gtk_window = GTK_WINDOW(toplevel);
        gtk_window_is_maximized(gtk_window) ? gtk_window_unmaximize(gtk_window) : gtk_window_maximize(gtk_window);
    }
}

static GtkWidget *app_headerbar_create(MovieApplication *mapp) {

    // Set GTK CSD HeaderBar
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_widget_set_name(header_bar, "headerbar");

    // hide window decorationq of header bar
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), FALSE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Movie Collection");
    gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header_bar), FALSE);
    // prefered min header height
    gtk_widget_set_size_request(header_bar, -1, 45);

    // add buttons and callback on click (override gtk-decoration-layout property)
    GtkWidget *btn_close = app_headerbar_create_button(
        "window-close", "headerbutton",
        signal_headerbar_close, mapp
    );
    GtkWidget *btn_minimize = app_headerbar_create_button(
        "window-minimize", "headerbutton",
        signal_headerbar_minimize, mapp
    );
    GtkWidget *btn_maximize = app_headerbar_create_button(
        "window-maximize", "headerbutton",
        signal_headerbar_maximize, mapp
    );

    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), btn_close);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), btn_minimize);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), btn_maximize);

    return header_bar;
}

static void signal_app_startup(MovieApplication *mapp) {

    // set application main config variables

    // load previous window state, if any
    mainwindow_load_state(mapp);
}


static void signal_searchentry_keyrelease(GtkEntry *entry, GdkEventKey *event, MovieApplication *mapp) {
    if(event->keyval == GDK_KEY_Escape) {
        gtk_entry_set_text(entry, ""); // empty
    }
}

static void signal_searchentry_changed(GtkEntry *entry, MovieApplication *mapp) {
    const char *keyword = gtk_entry_get_text(entry);

    #if PACKAGE_DEVELOPER_MODE
        g_message("%s %s", __func__, keyword);
    #endif

}






static void signal_sidebar_list_items_selected(GtkListBox *listbox, GtkListBoxRow *listrow, MovieApplication *mapp) {
    const char *item_id = gtk_widget_get_name(GTK_WIDGET(listrow));

    #if PACKAGE_DEVELOPER_MODE
        g_message("%s %s", __func__, item_id);
    #endif


}

static GtkWidget *app_toolbar_button_new(char *icon_name, char *label, void *click_event, gpointer user_data) {

    GtkWidget *button = gtk_button_new_with_label(label);
    widget_add_class(button, "toolbar-button");
    gtk_button_set_always_show_image(GTK_BUTTON(button), TRUE);

    char *icon_path = widget_get_iconpath(icon_name);
    GtkWidget *image = gtk_image_new_from_file(icon_path);
    gtk_button_set_image(GTK_BUTTON(button), GTK_WIDGET(image));

    if(click_event != NULL) {
        g_signal_connect(button, "clicked", G_CALLBACK(click_event), user_data);
    }

    return button;
}

static void signal_toolbar_provider_change(GtkToggleButton *togglebutton, char* provider_name) {
    bool is_active = gtk_toggle_button_get_active(togglebutton);
    
    #if PACKAGE_DEVELOPER_MODE
        g_message("%s %s %s", __func__, provider_name, (is_active?"on":"off"));
    #endif



}

static GtkWidget *app_toolbar_create() {

    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    widget_add_class(toolbar, "toolbar");
    gtk_widget_set_size_request(toolbar, -1, 45); // width height

    GtkWidget *button_open = app_toolbar_button_new(
        "toolbar-open", "Ouvrir",
        NULL, NULL
    );
    GtkWidget *button_save = app_toolbar_button_new(
        "toolbar-save", "Enregistrer",
        NULL, NULL
    );
    GtkWidget *button_new = app_toolbar_button_new(
        "toolbar-new", "Ajouter un film",
        NULL, NULL
    );

    gtk_box_pack_start(GTK_BOX(toolbar), button_open, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), button_save, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), button_new, FALSE, FALSE, 0);

    // add movie provider selection

    GtkWidget *providers = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *tmdben = gtk_radio_button_new_with_label(NULL, "TMDb EN");
    widget_add_class(tmdben, "toolbar-button");
    widget_add_class(tmdben, "toolbar-provider");
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(tmdben), FALSE);
    g_signal_connect(tmdben, "toggled", G_CALLBACK(signal_toolbar_provider_change), "tmdb-en");

    GtkWidget *tmdbfr = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(tmdben), "TMDb FR");
    widget_add_class(tmdbfr, "toolbar-button");
    widget_add_class(tmdbfr, "toolbar-provider");
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(tmdbfr), FALSE);
    g_signal_connect(tmdbfr, "toggled", G_CALLBACK(signal_toolbar_provider_change), "tmdb-fr");

    gtk_container_add(GTK_CONTAINER(providers), tmdben);
    gtk_container_add(GTK_CONTAINER(providers), tmdbfr);

    gtk_box_pack_end(GTK_BOX(toolbar), providers, FALSE, FALSE, 0);

    return toolbar;
}




static struct WidgetSidebar *widget_sidebar_new() {

    struct WidgetSidebar *widget = malloc(sizeof(struct WidgetSidebar));

    // sidebar main container

    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(sidebar, "sidebar");
    gtk_widget_set_size_request(sidebar, 300, -1); // width, height

    widget->sidebar = sidebar;

    // Sidebar search

    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(search_box, "searchbar");

    widget->search_box = search_box;

    GtkWidget *search_entry = gtk_entry_new();
    widget_add_class(search_entry, "searchbar-entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Recherche");

    widget->search_entry = search_entry;

    char *search_icon = widget_get_iconpath("edit-find");
    if(search_icon != NULL) {
        GIcon *gicon = g_file_icon_new(g_file_new_for_path(search_icon));
        gtk_entry_set_icon_from_gicon(GTK_ENTRY(search_entry), GTK_ENTRY_ICON_PRIMARY, gicon);
    }

    gtk_widget_set_hexpand(GTK_WIDGET(search_entry), FALSE);

    gtk_container_add(GTK_CONTAINER(search_box), search_entry);

    // sidebar categories

    GtkWidget *scrolled_frame = gtk_scrolled_window_new(NULL, NULL); // horiz, vertical adjustement
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_frame), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *list_items = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list_items), GTK_SELECTION_SINGLE);
    widget_add_class(list_items, "categories");
    gtk_widget_set_size_request(list_items, 300, -1); // width height
    // gtk_list_box_set_placeholder

    widget->list_items = list_items;

    gtk_container_add(GTK_CONTAINER(scrolled_frame), list_items);

    // Add all elements to sidebar
    gtk_box_pack_start(GTK_BOX(sidebar), search_box, FALSE, FALSE, 0); // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(sidebar), scrolled_frame, TRUE, TRUE, 0);

    return widget;
}

static struct WidgetSidebarItem *widget_sidebar_item_new(char *item_id, char *label_text, bool is_favorite) {

    struct WidgetSidebarItem *widget = malloc(sizeof(struct WidgetSidebarItem));

    // list row
    GtkWidget *list_row = gtk_list_box_row_new();
    widget_add_class(list_row, "category-item");
    gtk_widget_set_name(list_row, item_id); // set id (todo)

    widget->list_row = list_row;

    // elements inside row
    GtkWidget *list_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *label = gtk_label_new(label_text);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    // gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    // align left, verticial center
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);

    widget->label = label;

    char *icon_path = widget_get_iconpath("emblem-favorite");
    GtkWidget *favorite_icon = gtk_image_new_from_file(icon_path);
    gtk_widget_set_visible(favorite_icon, (is_favorite));

    widget->favorite_icon = favorite_icon;

    gtk_box_pack_start(GTK_BOX(list_box), label, TRUE, TRUE, 0); // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(list_box), favorite_icon, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(list_row), list_box);

    return widget;
}

static void widget_sidebar_items_add(struct WidgetSidebar *sidebar, struct WidgetSidebarItem *item) {
    // gtk_list_box_insert(GTK_LIST_BOX(sidebar->list_items), GTK_LIST_BOX_ROW(item->list_row), -1);
    gtk_container_add(GTK_CONTAINER(sidebar->list_items), GTK_WIDGET(item->list_row));
}



static struct WidgetStatusbar *widget_statusbar_new() {

    struct WidgetStatusbar *widget = malloc(sizeof(struct WidgetStatusbar));

    GtkWidget *statusbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    widget_add_class(statusbar, "statusbar");

    widget->statusbar = statusbar;

    GtkWidget *label = gtk_label_new("");
    widget_add_class(label, "statusbar-message");
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);

    widget->label = label;

    // align left, verticial center
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);

    gtk_widget_set_size_request(statusbar, -1, 30); // width height

    gtk_container_add(GTK_CONTAINER(statusbar), label);

    return widget;
}

static void widget_statusbar_set_text(struct WidgetStatusbar *statusbar, const char* text) {
    gtk_label_set_text(GTK_LABEL(statusbar->label), text);
}




static struct WidgetPanels *widget_panels_new() {

    struct WidgetPanels *widget = malloc(sizeof(struct WidgetPanels));

    GtkWidget *panels = gtk_notebook_new();

    // show tabs only if dev mode
    #if !PACKAGE_DEVELOPER_MODE
        gtk_notebook_set_show_tabs(GTK_NOTEBOOK(panels), FALSE);
    #endif
    gtk_notebook_set_show_border(GTK_NOTEBOOK(panels), FALSE);

    widget->panels = panels;

    // create panels

    struct WidgetPanelWelcome *panel_welcome = widget_panel_welcome_new();
    struct WidgetPanelPreview *panel_preview = widget_panel_preview_new();
    struct WidgetPanelEdition *panel_edition = widget_panel_edition_new();

    gtk_notebook_append_page(GTK_NOTEBOOK(panels), panel_welcome->panel, gtk_label_new("welcome"));
    gtk_notebook_append_page(GTK_NOTEBOOK(panels), panel_preview->panel, gtk_label_new("preview"));
    gtk_notebook_append_page(GTK_NOTEBOOK(panels), panel_edition->panel, gtk_label_new("edition"));

    return widget;
}

static struct WidgetPanelWelcome *widget_panel_welcome_new() {

    struct WidgetPanelWelcome *widget = malloc(sizeof(struct WidgetPanelWelcome));

    GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(panel, "panel");
    widget_add_class(panel, "panel-welcome");
    widget->panel = panel;


    gtk_container_add(GTK_CONTAINER(panel), gtk_label_new("HELLO!!"));


    return widget;
}

static struct WidgetPanelPreview *widget_panel_preview_new() {

    struct WidgetPanelPreview *widget = malloc(sizeof(struct WidgetPanelPreview));

    GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(panel, "panel");
    widget_add_class(panel, "panel-preview");
    widget->panel = panel;






    return widget;
}

static struct WidgetPanelEdition *widget_panel_edition_new() {

    struct WidgetPanelEdition *widget = malloc(sizeof(struct WidgetPanelEdition));

    GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(panel, "panel");
    widget_add_class(panel, "panel-edition");
    widget->panel = panel;










    return widget;
}

static void widget_panel_edition_populate() {


}






static void app_show_interactive_dialog(MovieApplication* mapp) {
    const char *appid = g_application_get_application_id(G_APPLICATION(mapp));

    // Initialize GTK+
    GtkWidget *main_window = gtk_application_window_new(GTK_APPLICATION(mapp));

    // Create an 800x600 window that will contain the browser instance
    gtk_window_set_title(GTK_WINDOW(main_window), "Movie Collection");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(main_window), TRUE);
    gtk_window_set_icon_name(GTK_WINDOW(main_window), appid);

    // Set window settings
    GtkSettings *window_settings = gtk_settings_get_default();
    g_object_set(G_OBJECT(window_settings),
        "gtk-application-prefer-dark-theme", TRUE, //because webview is dark :)
    NULL);

    // hide window decorations of main app and use our own
    GtkWidget *header_bar = app_headerbar_create(mapp);

    // header bar
    gtk_container_set_border_width(GTK_CONTAINER(main_window), 0);
    gtk_window_set_titlebar(GTK_WINDOW(main_window), header_bar);

    // Callback when the main window is closed
    g_signal_connect(main_window, "destroy",
        G_CALLBACK(signal_mainwindow_destroy), mapp
    );

    // on change state: minimize, maximize, etc
    g_signal_connect(GTK_WINDOW(main_window), "window-state-event",
        G_CALLBACK(signal_mainwindow_state_event), mapp
    );

    // on change size
    g_signal_connect(GTK_WINDOW(main_window), "size-allocate",
        G_CALLBACK(signal_mainwindow_size_allocate), mapp
    );

    // Load window previous state, if any

    if(mapp->win_height > 0 && mapp->win_width > 0) {
        gtk_window_set_default_size(GTK_WINDOW(main_window),
            mapp->win_width,
            mapp->win_height
        );
    }

    if(mapp->is_maximized) {
        gtk_window_maximize(GTK_WINDOW(main_window));
    }

    if(mapp->is_fullscreen) {
        gtk_window_fullscreen(GTK_WINDOW(main_window));
    }

    // Styling application (if file available)

    char *window_style = g_build_filename(PACKAGE_STYLES_DIR, "style.css", NULL);

    if(g_file_test(window_style, G_FILE_TEST_IS_REGULAR)) {

        GtkCssProvider *css_provider = gtk_css_provider_new();

        GError *css_error = NULL;
        gtk_css_provider_load_from_path(css_provider, window_style, &css_error);

        if(css_error != NULL) {
            g_warning("app:import style.css %s", css_error->message);
            g_clear_error(&css_error);
        }

        gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(css_provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );
    }




    // Window Main
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    // Right Aside
    GtkWidget *layout_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // Upper toolbar
    GtkWidget *toolbar = app_toolbar_create();

    // Sidebar
    struct WidgetSidebar *widget_sidebar = widget_sidebar_new();

    GtkWidget *sidebar = widget_sidebar->sidebar;

    g_signal_connect(widget_sidebar->search_entry, "key-release-event",
        G_CALLBACK(signal_searchentry_keyrelease), mapp
    );

    g_signal_connect(widget_sidebar->search_entry, "changed",
        G_CALLBACK(signal_searchentry_changed), mapp
    );

    g_signal_connect(widget_sidebar->list_items, "row-selected", // categories
        G_CALLBACK(signal_sidebar_list_items_selected), mapp
    );

    // All all elements to right content (aside)

    struct WidgetPanels *widget_panels = widget_panels_new();

    GtkWidget *panels = widget_panels->panels;

    gtk_box_pack_start(GTK_BOX(layout_box), sidebar, FALSE, FALSE, 0); //expand, fill, padding
    gtk_box_pack_start(GTK_BOX(layout_box), panels, TRUE, TRUE, 0);
    
    struct WidgetStatusbar *widget_statusbar = widget_statusbar_new();

    GtkWidget *statusbar = widget_statusbar->statusbar;

    // Add all elements to main
    gtk_box_pack_start(GTK_BOX(main_box), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), layout_box, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), statusbar, FALSE, FALSE, 0);










struct WidgetSidebarItem *xxx = widget_sidebar_item_new("qedeqerezrz", "XXX", FALSE);
widget_sidebar_items_add(widget_sidebar, xxx);

xxx = widget_sidebar_item_new("d", "azertyuiopqsdfghjklmwxcvbnazertyuiopqsdfghjklmwxcvbnazertyuiopqsdfghjklmwxcvbn", FALSE);
widget_sidebar_items_add(widget_sidebar, xxx);




    // Put the content area into the main window
    gtk_container_add(GTK_CONTAINER(main_window), main_box);

    // Make sure that when the browser area becomes visible, it will get mouse and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(main_box));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(main_window);
}

static void signal_app_activate(MovieApplication* mapp) {

    // Check if window is already active
    GtkWindow *window = gtk_application_get_active_window(GTK_APPLICATION(mapp));

    if(window != NULL) {
        gtk_window_present(window);
        return;
    }

    // Else, we show the main window
    app_show_interactive_dialog(mapp);
}

static void signal_app_shutdown(MovieApplication* mapp) {

    // save current window state
    mainwindow_store_state(mapp);
}

static int signal_app_command_line(MovieApplication* mapp, GApplicationCommandLine *cmdline) {
    return 0; // exit
}

static void app_commandline_print_version(MovieApplication* mapp) {
    const char *appid = g_application_get_application_id(G_APPLICATION(mapp));

    g_print("%s - GTK:%d.%d.%d \n", appid,
        gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version()
    );
}

static int signal_app_handle_local_options(MovieApplication* mapp, GVariantDict *options) {
    // handle command lines locally

    if(g_variant_dict_lookup(options, "version", "b", NULL)) {
        app_commandline_print_version(mapp);
        return 0;
    }

    if(g_variant_dict_lookup(options, "inspect", "b", NULL)) {
        gtk_window_set_interactive_debugging(TRUE);
    }

    return -1; //let the default option processing continue
}



int main(int argc, char* argv[]) {

    #if PACKAGE_DEVELOPER_MODE
        g_message("Dev mode");
        // hacks workaround slow computers
        putenv("WEBKIT_DISABLE_COMPOSITING_MODE=1");
        // inspector debug
        putenv("GTK_DEBUG=fatal-warnings");
        putenv("GOBJECT_DEBUG=instance-count");
        // putenv("G_ENABLE_DIAGNOSTIC=1");
    #endif


    int status;

    // Instantiate the main app
    MovieApplication *mapp = movie_application_new(
        PACKAGE_APPLICATION_ID,
        G_APPLICATION_FLAGS_NONE //| G_APPLICATION_HANDLES_COMMAND_LINE
    );

    // Add aplication main events flow (start with "_init")
    g_signal_connect(mapp, "startup", G_CALLBACK(signal_app_startup), NULL);
    g_signal_connect(mapp, "activate", G_CALLBACK(signal_app_activate), NULL);
    g_signal_connect(mapp, "shutdown", G_CALLBACK(signal_app_shutdown), NULL);
    // g_signal_connect(mapp, "open", G_CALLBACK(signal_app_open), NULL);

    // Add app main arguments
    GOptionEntry entries[] = {
        {"version", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL, "Show program version", NULL},
        {"inspect", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL, "Open the interactive debugger", NULL},
        {NULL}
    };
    g_application_add_main_option_entries(G_APPLICATION(mapp), entries);

    g_signal_connect(mapp, "handle-local-options", G_CALLBACK(signal_app_handle_local_options), NULL);
    g_signal_connect(mapp, "command-line", G_CALLBACK(signal_app_command_line), NULL); // received from remote

    // Run the app and get its exit status
    status = g_application_run(G_APPLICATION(mapp), argc, argv);

    g_object_unref(mapp);

    return status;
}

