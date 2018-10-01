/*
sudo dnf install gtk3-devel gstreamer-devel clutter-devel webkitgtk3-devel libgda-devel gobject-introspection-devel
webkit2gtk3-devel
builddir > clear && ninja && GTK_DEBUG=interactive ./moviecollection --debug
gcc moviecollection.c -o main `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` && ./main
meson --buildtype
coredumpctl list => gdb
*/

#include <stdlib.h>
#include <glib.h>

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>


// static gboolean close_webview_callback(WebKitWebView* Webview, GtkWidget* window) {
//     gtk_widget_destroy(window);
//     return true;
// }

//g_assert


static bool fileexists(const char *filename) {
    // try to open file to read
    FILE *file = fopen(filename, "r");
    if(file) {
        fclose(file);
        return true;
    }
    return false;
}
static bool direxists(const char *dirname) {
    // try to open directory
    DIR *dir = opendir(dirname);
    if(dir) {
        closedir(dir);
        return true;
    }
    return false;
}




static void initialize_web_extensions(WebKitWebContext *webkit_context, GVariant *user_data) {
    int unique_id;
    char *webextension_dir;

    g_variant_get(user_data, "(is)", &unique_id, &webextension_dir);

    char webextension_file[255];
    strcpy(webextension_file, webextension_dir);
    strcat(webextension_file, "libweb-extension-proxy.so");

    if(!fileexists(webextension_file)) {
        // extension not found, abort()
        g_error("app:initialize_web_extensions 'libweb-extension-proxy.so' not found");
    }

    webkit_web_context_set_web_extensions_directory(webkit_context, webextension_dir);
    webkit_web_context_set_web_extensions_initialization_user_data(webkit_context, user_data);
}














struct WebviewWindowState {
    int height;
    int width;
    bool is_maximized;
    bool is_fullscreen;
};

typedef struct {
    // application main window
    GtkWidget *window;
    GtkWidget *header_bar;
    GtkWidget *webview;
    struct WebviewWindowState window_state;

    // other settings
    bool debug;
    char launcher_dir[200];
    char ressources_dir[255];
    char webextension_dir[255];

} WebviewApplication;

static void app_window_store_state(GtkApplication *gtk_app, struct WebviewWindowState *window_state) {
    const char *appid = g_application_get_application_id(G_APPLICATION(gtk_app));

    GKeyFile *keyfile = g_key_file_new();

    g_key_file_set_integer(keyfile, "WindowState", "height", window_state->height);
    g_key_file_set_integer(keyfile, "WindowState", "width", window_state->width);
    g_key_file_set_boolean(keyfile, "WindowState", "maximized", window_state->is_maximized);
    g_key_file_set_boolean(keyfile, "WindowState", "fullscreen", window_state->is_fullscreen);

    // save file under $XDG_CACHE_HOME
    char *state_path = g_build_filename(g_get_user_cache_dir(), appid, NULL);
    char *state_file = g_build_filename(state_path, "state.ini", NULL);

    // create save âth if not set
    if(g_mkdir_with_parents(state_path, 0755) > -1) { // error=-1 exist=0

        GError *error_save = NULL;
        if(!g_key_file_save_to_file(keyfile, state_file, &error_save)) {
            g_warning ("app:window_store_state Error: %s", error_save->message);
        }
    }

    g_key_file_unref(keyfile);
    g_free(state_path);
    g_free(state_file);
}

static void app_window_load_state(GtkApplication *gtk_app, struct WebviewWindowState *window_state) {
    const char *appid = g_application_get_application_id(G_APPLICATION(gtk_app));

    char *state_file = g_build_filename(g_get_user_cache_dir(), appid, "state.ini", NULL);

    GKeyFile *keyfile = g_key_file_new();


    if(g_key_file_load_from_file(keyfile, state_file, G_KEY_FILE_NONE, NULL)) {

        GError *error_read = NULL;

        int state_height = g_key_file_get_integer(keyfile, "WindowState", "height", &error_read);
        error_read == NULL ? (window_state->height = state_height) : g_clear_error(&error_read);

        int state_width = g_key_file_get_integer(keyfile, "WindowState", "width", &error_read);
        error_read == NULL ? (window_state->width = state_width) : g_clear_error(&error_read);

        bool state_maximized = g_key_file_get_boolean(keyfile, "WindowState", "maximized", &error_read);
        error_read == NULL ? (window_state->is_maximized = state_maximized) : g_clear_error(&error_read);

        int state_fullscreen = g_key_file_get_boolean(keyfile, "WindowState", "fullscreen", &error_read);
        error_read == NULL ? (window_state->is_fullscreen = state_fullscreen) : g_clear_error(&error_read);
    }

    g_key_file_unref(keyfile);
    g_free(state_file);
}

static bool app_window_state_event_callback(GtkWidget *window, GdkEventWindowState *event, struct WebviewWindowState *window_state) {
    GdkWindowState new_window_state = event->new_window_state; // event->type=GDK_WINDOW_STATE

    window_state->is_maximized = (new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;

    window_state->is_fullscreen = (new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;

    return GDK_EVENT_PROPAGATE;
}

static void app_window_size_allocate_callback(GtkWidget *window, GdkRectangle *allocation, struct WebviewWindowState *window_state) {
    // save the window geometry only if we are not maximized of fullscreen
    if(!(window_state->is_maximized || window_state->is_fullscreen)) {
        // use gtk_ ; Using the allocation directly can lead to growing windows with client-side decorations
        gtk_window_get_size(GTK_WINDOW(window),
            &window_state->width,
            &window_state->height
        );
    }
}

// create a gtk button with an icon inside
static GtkWidget *app_create_button_icon(
    const char *ressources_dir,
    const char *icon_name,
    const char *class_name,
    GtkApplication *gtk_app,
    const void *click_event
) {
    char icon_path[255];
    sprintf(icon_path, "%s/%s.svg", ressources_dir, icon_name);
    // gtk_image_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_MENU);

    GtkWidget *gtk_button = gtk_button_new();

    if(class_name != NULL) {
        gtk_style_context_add_class(gtk_widget_get_style_context(gtk_button), class_name);
    }

    if(click_event != NULL) {
        g_signal_connect(gtk_button, "clicked", G_CALLBACK(click_event), gtk_app);
    }

    gtk_container_add(GTK_CONTAINER(gtk_button), gtk_image_new_from_file(icon_path));

    return gtk_button;
}

static void app_headerbar_close_callback(GtkButton* button, GtkApplication *gtk_app) {
    g_application_quit(G_APPLICATION(gtk_app));
}
static void app_headerbar_minimize_callback(GtkButton* button, GtkApplication *gtk_app) {
    GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if(GTK_IS_WINDOW(toplevel)) {
        GtkWindow *gtk_window = GTK_WINDOW(toplevel);
        gtk_window_iconify(gtk_window);
    }
}
static void app_headerbar_maximize_callback(GtkButton* button, GtkApplication *gtk_app) {
    GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if(GTK_IS_WINDOW(toplevel)) {
        GtkWindow *gtk_window = GTK_WINDOW(toplevel);
        gtk_window_is_maximized(gtk_window) ? gtk_window_unmaximize(gtk_window) : gtk_window_maximize(gtk_window);
    }
}



static GtkWidget *app_headerbar_create(GtkApplication *gtk_app, WebviewApplication *app) {

    // Set GTK CSD HeaderBar
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_widget_set_name(header_bar, "header_bar");

    // hide window decorationq of header bar
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), false);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Movie Collection");
    gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header_bar), false);

    // add buttons and callback on click
    GtkWidget *btn_close = app_create_button_icon(
        app->ressources_dir, "window-close", "titlebutton",
        gtk_app, app_headerbar_close_callback
    );
    GtkWidget *btn_minimize = app_create_button_icon(
        app->ressources_dir, "window-minimize", "titlebutton",
        gtk_app, app_headerbar_minimize_callback
    );
    GtkWidget *btn_maximize = app_create_button_icon(
        app->ressources_dir, "window-maximize", "titlebutton",
        gtk_app, app_headerbar_maximize_callback
    );

    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), btn_close);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), btn_minimize);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), btn_maximize);

    return header_bar;
}




static void app_startup_callback(GtkApplication *gtk_app, WebviewApplication *app) {

    // get current application path
    getcwd(app->launcher_dir, sizeof(app->launcher_dir));

    g_message("app:launcher_dir %s", app->launcher_dir);

    //get a different ID for each Web Process
    static int unique_id = 0;

    // get webkit extensions .so directory
    sprintf(app->webextension_dir, "%s/", app->launcher_dir);

    // get app ressources directory
    sprintf(app->ressources_dir, "%s/ressources/", app->launcher_dir);

    // load previous window state, if any
    app_window_load_state(gtk_app, &app->window_state);
}

static void app_activate_callback(GtkApplication* gtk_app, WebviewApplication *app) {

    app->window = gtk_application_window_new(gtk_app);

    struct WebviewWindowState window_state = app->window_state;




    app->header_bar = app_headerbar_create(gtk_app, app);

    // hide window decorations of main app and use our own
    gtk_window_set_titlebar(GTK_WINDOW(app->window), app->header_bar);








  gtk_window_set_title (GTK_WINDOW (app->window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (app->window), 200, 200);




    if(window_state.height > 0 && window_state.width > 0) {
        gtk_window_set_default_size(GTK_WINDOW(app->window),
            window_state.width,
            window_state.height
        );
    }


    if(window_state.is_maximized) {
        gtk_window_maximize(GTK_WINDOW(app->window));
    }

    if(window_state.is_fullscreen) {
        gtk_window_fullscreen(GTK_WINDOW(app->window));
    }


  gtk_widget_show_all (app->window);



    // on change state: minimize, maximize, etc
    g_signal_connect(GTK_WINDOW(app->window), "window-state-event",
        G_CALLBACK(app_window_state_event_callback), &app->window_state
    );

    // on change size
    g_signal_connect(GTK_WINDOW(app->window), "size-allocate",
        G_CALLBACK(app_window_size_allocate_callback), &app->window_state
    );
}

static void app_shutdown_callback(GtkApplication* gtk_app, WebviewApplication *app) {

    // save current window state
    app_window_store_state(gtk_app, &app->window_state);



}

static int app_commandline_callback(GtkApplication* gtk_app, GApplicationCommandLine *cmdline, WebviewApplication *app) {
    return 0;
}


int main(int argc, char* argv[]) {

    // main gtk app
    GtkApplication *gtk_app;
    int status;

    // new_ user application
    WebviewApplication *app = g_malloc(sizeof(WebviewApplication));

    gtk_app = gtk_application_new("fr.spidery.moviecollection",
        G_APPLICATION_FLAGS_NONE //| G_APPLICATION_HANDLES_COMMAND_LINE
    );
    
    g_signal_connect(gtk_app, "startup", G_CALLBACK(app_startup_callback), app);
    g_signal_connect(gtk_app, "activate", G_CALLBACK(app_activate_callback), app);
    g_signal_connect(gtk_app, "shutdown", G_CALLBACK(app_shutdown_callback), app);
    g_signal_connect(gtk_app, "command-line", G_CALLBACK(app_commandline_callback), app);

    status = g_application_run(G_APPLICATION(gtk_app), argc, argv);

    g_object_unref(gtk_app);

    return status;
}











/*

int ___main(int argc, char* argv[]) {

    bool is_debug = argv[1] != NULL && strcmp(argv[1], "--debug") == 0;








    // Initialize GTK+
    gtk_init(&argc, &argv);

    // Create an 800x600 window that will contain the browser instance
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    // Set GTK window options
    gtk_window_set_title(GTK_WINDOW(main_window), "Movie Collection");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(main_window), true);

    GtkSettings *window_settings = gtk_settings_get_default();
    g_object_set(G_OBJECT(window_settings),
        "gtk-application-prefer-dark-theme", TRUE, NULL //because webview is dark :)
    );

    // Callback when the main window is closed
    g_signal_connect(main_window, "destroy", G_CALLBACK(destroy_window_callback), NULL);




//gtk_window_set_icon_from_file




// direxists





    // Styling application (if file available)

    char window_style_dir[256];
    sprintf(window_style_dir, "%s/style.css", launcher_dir);

    if(fileexists(window_style_dir)) {

        if(is_debug) {
            g_message("app:window_style_dir %s", window_style_dir);
        }

        GtkCssProvider *window_css_provider = gtk_css_provider_get_default();

        GError *css_error = NULL;
        gtk_css_provider_load_from_path(window_css_provider, window_style_dir, &css_error);

        if(css_error != NULL) {
            g_warning("app:import style.css %s", css_error->message);
        }

        gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(window_css_provider),
            GTK_STYLE_PROVIDER_PRIORITY_USER
        );
    }

    // Webview settings & create the webview
    WebKitSettings *websettings = webkit_settings_new_with_settings(
        "default-charset", "utf8",
        "enable-javascript", true,
        "auto-load-images", true,
        "enable-page-cache", false,
        "allow-file-access-from-file-urls", true, // todo allow xhr request
        "allow-universal-access-from-file-urls", true, // access ressources locally
        "enable-write-console-messages-to-stdout", is_debug, // debug settings
        "enable-developer-extras", is_debug,
    NULL);

    // arguments passed to proxy extension

    WebKitWebContext *webkit_context = webkit_web_context_get_default();// todo webkit_web_view_get_context(webview);
    
    GVariant *webextension_data = g_variant_new(
        "(is)", unique_id++, webextension_dir
    );

    // Callback when initialize extensions
    g_signal_connect(webkit_context, "initialize-web-extensions",
        G_CALLBACK(initialize_web_extensions), webextension_data
    );

    WebKitWebView *webview = WEBKIT_WEB_VIEW(webkit_web_view_new_with_settings(websettings));

    // Callback when browser instance is closed, the program will also exit
    g_signal_connect(webview, "close",
        G_CALLBACK(close_webview_callback), main_window
    );





// todo check si extension .so not found






    // Put the browser area into the main window
    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(webview));

    // Load a web page into the browser instance
    char webview_page[256];
    sprintf(webview_page, "file://%s/bundle/index.html", launcher_dir);

    webkit_web_view_load_uri(webview, webview_page);

    // Make sure that when the browser area becomes visible, it will get mouse and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(webview));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(main_window);

    // Run the main GTK+ event loop
    gtk_main();

    return 0;
}

*/