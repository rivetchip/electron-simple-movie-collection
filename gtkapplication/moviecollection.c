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
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h> 


typedef struct {
    GtkApplication parent_instance;

    // application main window
    int win_height;
    int win_width;
    bool is_maximized;
    bool is_fullscreen;

} MovieApplication;

// typedef GtkApplication MovieApplication;
typedef GtkApplicationClass MovieApplicationClass;

G_DEFINE_TYPE(MovieApplication, movie_application, GTK_TYPE_APPLICATION);


static void movie_application_init(MovieApplication *app) {
    g_message(__func__);
}

static void movie_application_class_init(MovieApplicationClass *class) {
    // GApplicationClass *app_class = G_APPLICATION_CLASS(class);

    // app_class->startup = demo_application_startup;
    // app_class->activate = demo_application_activate;
}

MovieApplication *movie_application_new(const char *application_id, GApplicationFlags flags) {
    
    g_return_val_if_fail(g_application_id_is_valid(application_id), NULL);
    
    return g_object_new(movie_application_get_type(),
        "application-id", application_id,
        "flags", flags,
    NULL);
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
        gtk_style_context_add_class(gtk_widget_get_style_context(gtk_button), class_name);
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
    gtk_widget_set_name(header_bar, "header_bar");

    // hide window decorationq of header bar
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), FALSE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Movie Collection");
    gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header_bar), FALSE);

    g_object_set(G_OBJECT(header_bar),
        "height-request", 45, //prefered max header size
    NULL);

    // add buttons and callback on click (override gtk-decoration-layout property)
    GtkWidget *btn_close = app_headerbar_create_button(
        "window-close", "titlebutton",
        signal_headerbar_close, mapp
    );
    GtkWidget *btn_minimize = app_headerbar_create_button(
        "window-minimize", "titlebutton",
        signal_headerbar_minimize, mapp
    );
    GtkWidget *btn_maximize = app_headerbar_create_button(
        "window-maximize", "titlebutton",
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






static void signal_searchentry_changed(GtkSearchEntry *entry, MovieApplication *mapp) {

}

static void signal_listbox_entries_row_selected(GtkListBox *listbox, GtkListBoxRow *listrow, MovieApplication *mapp) {

}

static GtkWidget *app_sidebar_create(MovieApplication *mapp) {

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Sidebar search

    GtkWidget *search_enty = gtk_search_entry_new();
    g_signal_connect(search_enty, "search-changed",
        G_CALLBACK(signal_searchentry_changed), mapp
    );

    GtkWidget *search_bar = gtk_search_bar_new();
    gtk_widget_set_hexpand(GTK_WIDGET(search_enty), FALSE);

    g_object_set(G_OBJECT(search_bar),
        "search-mode-enabled", TRUE,
        "show-close-button", FALSE,
    NULL);

    gtk_container_add(GTK_CONTAINER(search_bar), GTK_WIDGET(search_enty));

    // Sidebar entries list

    GtkWidget *list_scroll = gtk_scrolled_window_new(NULL, NULL); // horiz, vertical adjustement
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(list_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *list_box = gtk_list_box_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(list_box)), "categories");
    gtk_widget_set_size_request(GTK_WIDGET(list_box), 200, -1);

    g_signal_connect(list_box, "row-selected", // todo prefer row-activated
        G_CALLBACK(signal_listbox_entries_row_selected), mapp
    );
    //set_header_func

    gtk_container_add(GTK_CONTAINER(list_scroll), GTK_WIDGET(list_box));

    // Add all elements to sidebar
    gtk_box_pack_start(GTK_BOX(box), search_bar, FALSE, FALSE, 0); // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(list_box), TRUE, TRUE, 0);

    return box;
}

static GtkWidget *app_separator_create(MovieApplication *mapp) {

    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_style_context_add_class(gtk_widget_get_style_context(separator), "separator");

    return separator;
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
            GTK_STYLE_PROVIDER_PRIORITY_USER
        );
    }


    // Create main content
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *sidebar = app_sidebar_create(mapp);
    GtkWidget *separator = app_separator_create(mapp);

    gtk_box_pack_start(GTK_BOX(main_box), sidebar, FALSE, FALSE, 0); //expand, fill, padding
    gtk_box_pack_start(GTK_BOX(main_box), separator, FALSE, FALSE, 0);





    // Put the content area into the main window
    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(main_box));


    // Make sure that when the browser area becomes visible, it will get mouse and keyboard events
    // gtk_widget_grab_focus(GTK_WIDGET(webview));

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
