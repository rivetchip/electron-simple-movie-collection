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



G_DEFINE_TYPE(WidgetMovieItem, widget_movie_item, GTK_TYPE_LIST_BOX_ROW);

static void widget_movie_item_init(WidgetMovieItem *self) {
    // gtk_widget_init_template(GTK_WIDGET(self));
}

static void widget_movie_item_class_init(WidgetMovieItemClass *klass) {
    //
}

static WidgetMovieItem *widget_movie_item_new(char *movie_id, char *movie_title, bool is_favorite) {

    WidgetMovieItem *item = g_object_new(widget_movie_item_get_type(), NULL);

    // todo use gobject props
    item->movie_id = movie_id;
    item->movie_title = movie_title;
    item->is_favorite = is_favorite;

    return item;
}











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
static GtkWidget *app_headerbar_create_button(char *icon_name, char *class_name, void *click_event, MovieApplication *mapp) {
    char icon_svg[20];
    sprintf(icon_svg, "%s.svg", icon_name);

    char *icon_path = g_build_filename(PACKAGE_RESSOURCES_DIR, icon_svg, NULL);

    GtkWidget *gtk_button = gtk_button_new();

    if(class_name != NULL) {
        widget_add_class(gtk_button, class_name);
    }

    if(click_event != NULL) {
        g_signal_connect(gtk_button, "clicked", G_CALLBACK(click_event), mapp);
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



static void signal_searchentry_changed(GtkEntry *entry, MovieApplication *mapp) {

}

static void signal_searchentry_keyrelease(GtkEntry *entry, GdkEventKey *event, MovieApplication *mapp) {
    if(event->keyval == GDK_KEY_Escape) {
        gtk_entry_set_text(entry, ""); // empty
    }
}




static void signal_listbox_entries_row_selected(GtkListBox *listbox, GtkListBoxRow *listrow, MovieApplication *mapp) {
    // GtkListBoxRow *list_row = gtk_list_box_get_selected_row(listbox);
    WidgetMovieItem *movie_item = WIDGET_MOVIE_ITEM(listrow);
    
    
    g_message("select %s", movie_item->movie_id);
}


static GtkWidget *app_toolbar_create() {

    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    widget_add_class(toolbar, "toolbar");

    GtkWidget *button_open = gtk_button_new_with_label("Ouvrir");
    widget_add_class(button_open, "toolbar-button");
    gtk_button_set_always_show_image(GTK_BUTTON(button_open), TRUE);

    char *favorite_icon = widget_get_iconpath("emblem-favorite");
    GtkWidget *gtk_image = gtk_image_new_from_file(favorite_icon);
    gtk_button_set_image(GTK_BUTTON(button_open), GTK_WIDGET(gtk_image));



    GtkWidget *button_save = gtk_button_new_with_label("Enregistrer");
    widget_add_class(button_save, "toolbar-button");
    gtk_button_set_always_show_image(GTK_BUTTON(button_save), TRUE);

    GtkWidget *button_new = gtk_button_new_with_label("Ajouter un film");
    widget_add_class(button_new, "toolbar-button");
    gtk_button_set_always_show_image(GTK_BUTTON(button_new), TRUE);

    gtk_box_pack_start(GTK_BOX(toolbar), button_open, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), button_save, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), button_new, FALSE, FALSE, 0);

    // add movie provider selection


    //todo


    gtk_widget_set_size_request(toolbar, -1, 45); // width height

    return toolbar;
}


static GtkWidget *app_sidebar_rowitem_create(char *movie_id, char *movie_title, bool is_favorite) {

    WidgetMovieItem *list_row = widget_movie_item_new(movie_id, movie_title, is_favorite);

    widget_add_class(GTK_WIDGET(list_row), "category-item");

    GtkWidget *list_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *label = gtk_label_new(movie_title);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    // gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);

    // align left, verticial center
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);

    char *favorite_icon = widget_get_iconpath("emblem-favorite");
    GtkWidget *gtk_image = gtk_image_new_from_file(favorite_icon);
    gtk_widget_set_visible(gtk_image, is_favorite);

    gtk_box_pack_start(GTK_BOX(list_box), label, TRUE, TRUE, 0); // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(list_box), gtk_image, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(list_row), list_box);

    return GTK_WIDGET(list_row);
}

static GtkWidget *app_sidebar_create(MovieApplication *mapp) {

    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(sidebar, "sidebar");

    // Sidebar search

    GtkWidget *search_enty = gtk_entry_new();
    widget_add_class(search_enty, "searchentry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_enty), "Recherche");

    char *search_icon = widget_get_iconpath("edit-find");
    if(search_icon != NULL) {
        GIcon *gicon = g_file_icon_new(g_file_new_for_path(search_icon));
        gtk_entry_set_icon_from_gicon(GTK_ENTRY(search_enty), GTK_ENTRY_ICON_PRIMARY, gicon);
    }

    g_signal_connect(search_enty, "key-release-event",
        G_CALLBACK(signal_searchentry_keyrelease), mapp
    );

    g_signal_connect(search_enty, "changed",
        G_CALLBACK(signal_searchentry_changed), mapp
    );

    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(search_box, "searchbar");
    gtk_widget_set_hexpand(GTK_WIDGET(search_enty), FALSE);

    gtk_container_add(GTK_CONTAINER(search_box), search_enty);

    // Sidebar entries list

    GtkWidget *list_scroll = gtk_scrolled_window_new(NULL, NULL); // horiz, vertical adjustement
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(list_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *list_box = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list_box), GTK_SELECTION_SINGLE);
    widget_add_class(list_box, "categories");
    gtk_widget_set_name(list_box, "categories");

    gtk_widget_set_size_request(list_box, 300, -1); // width height

    g_signal_connect(list_box, "row-selected",
        G_CALLBACK(signal_listbox_entries_row_selected), mapp
    );

    gtk_container_add(GTK_CONTAINER(list_scroll), list_box);

    // Add all elements to sidebar
    gtk_box_pack_start(GTK_BOX(sidebar), search_box, FALSE, FALSE, 0); // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(sidebar), list_scroll, TRUE, TRUE, 0);

    return sidebar;
}

static GtkWidget *app_statusbar_create(MovieApplication *mapp) {

    GtkWidget *statusbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    widget_add_class(statusbar, "statusbar");

    GtkWidget *label = gtk_label_new("");
    widget_add_class(label, "statusbarmessage");
    gtk_widget_set_name(label, "statusbarmessage");

    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);

    // align left, verticial center
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);

    gtk_widget_set_size_request(statusbar, -1, 30); // width height

    gtk_container_add(GTK_CONTAINER(statusbar), label);

    return statusbar;
}

static GtkWidget *app_panels_create(MovieApplication *mapp) {
    GtkWidget *panels = gtk_notebook_new();

    // show tabs only if dev mode
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(panels), (PACKAGE_DEVELOPER_MODE));
    gtk_notebook_set_show_border(GTK_NOTEBOOK(panels), FALSE);

    gtk_notebook_append_page(GTK_NOTEBOOK(panels), gtk_label_new("test"), gtk_label_new("welcome"));
    gtk_notebook_append_page(GTK_NOTEBOOK(panels), gtk_label_new("test"), gtk_label_new("preview"));
    gtk_notebook_append_page(GTK_NOTEBOOK(panels), gtk_label_new("test2"), gtk_label_new("edition"));


    return panels;
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

    // Load window preview state, if any

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


    // Create main content
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *toolbar = app_toolbar_create();
    gtk_box_pack_start(GTK_BOX(main_box), toolbar, FALSE, FALSE, 0);

    GtkWidget *layout_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *sidebar_box = app_sidebar_create(mapp);
    GtkWidget *panels_box = app_panels_create(mapp);
    gtk_box_pack_start(GTK_BOX(layout_box), sidebar_box, FALSE, FALSE, 0); //expand, fill, padding
    gtk_box_pack_start(GTK_BOX(layout_box), panels_box, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), layout_box, TRUE, TRUE, 0);

    GtkWidget *statusbar = app_statusbar_create(mapp);
    gtk_box_pack_start(GTK_BOX(main_box), statusbar, FALSE, FALSE, 0);


    GtkWidget *listbox = widget_get_child(sidebar_box, "categories");
    GtkWidget *status_message = widget_get_child(statusbar, "statusbarmessage");

    gtk_label_set_text(GTK_LABEL(status_message), "sqdqsds");


    mapp->listbox = listbox;



    GtkWidget *list_item = app_sidebar_rowitem_create("11", "qsdsqsfsdsdsdfdsfd fsf sdfdsdfdsfdsfsfdsffsfdsdfsfdsq", TRUE);
    gtk_container_add(GTK_CONTAINER(listbox), GTK_WIDGET(list_item));
    GtkWidget *list_item2 = app_sidebar_rowitem_create("22", "qsdsqdsq", FALSE);
    gtk_container_add(GTK_CONTAINER(listbox), GTK_WIDGET(list_item2));
    GtkWidget *list_item3 = app_sidebar_rowitem_create("33", "OK OK TEST", TRUE);
    gtk_container_add(GTK_CONTAINER(listbox), GTK_WIDGET(list_item3));






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

