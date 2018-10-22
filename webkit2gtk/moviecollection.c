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
#include <webkit2/webkit2.h>


typedef struct {
    int height;
    int width;
    bool is_maximized;
    bool is_fullscreen;
} WebviewWindowState;

typedef struct {
    // application main window
    GtkWidget *header_bar;
    GtkWidget *webview;
    WebviewWindowState window_state;
    // options
    GHashTable *package_config;

} WebviewApplication;


static void app_window_store_state(GtkApplication *gtk_app, WebviewWindowState *window_state) {
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
            g_warning("app:window_store_state Error: %s", error_save->message);
        }
    }

    g_key_file_unref(keyfile);
    g_free(state_path);
    g_free(state_file);
}

static void app_window_load_state(GtkApplication *gtk_app, WebviewWindowState *window_state) {
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

static bool app_window_state_event_callback(GtkWidget *window, GdkEventWindowState *event, WebviewWindowState *window_state) {
    GdkWindowState new_window_state = event->new_window_state; // event->type=GDK_WINDOW_STATE

    window_state->is_maximized = (new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;

    window_state->is_fullscreen = (new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;

    return GDK_EVENT_PROPAGATE;
}

static void app_window_size_allocate_callback(GtkWidget *window, GdkRectangle *allocation, WebviewWindowState *window_state) {
    // save the window geometry only if we are not maximized of fullscreen
    if(!(window_state->is_maximized || window_state->is_fullscreen)) {
        // use gtk_ ; Using the allocation directly can lead to growing windows with client-side decorations
        gtk_window_get_size(GTK_WINDOW(window),
            &window_state->width,
            &window_state->height
        );
    }
}

static void app_window_destroy_callback(GtkWidget *window, GtkApplication *gtk_app) {
    g_application_quit(G_APPLICATION(gtk_app));
}

// create a gtk button with an icon inside
static GtkWidget *app_headerbar_create_button(
    const char *ressources_dir,
    const char *icon_name,
    const char *class_name,
    const void *click_event,
    GtkApplication *gtk_app
) {
    char icon_svg[20];
    sprintf(icon_svg, "%s.svg", icon_name);

    char *icon_path = g_build_filename(ressources_dir, icon_svg, NULL);

    GtkWidget *gtk_button = gtk_button_new();

    if(class_name != NULL) {
        gtk_style_context_add_class(gtk_widget_get_style_context(gtk_button), class_name);
    }

    if(click_event != NULL) {
        g_signal_connect(gtk_button, "clicked", G_CALLBACK(click_event), gtk_app);
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

    gtk_container_add(GTK_CONTAINER(gtk_button), gtk_image);

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

static GtkWidget *app_headerbar_create(GtkApplication *gtk_app, char* ressources_dir) {

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
        ressources_dir, "window-close", "titlebutton",
        app_headerbar_close_callback, gtk_app
    );
    GtkWidget *btn_minimize = app_headerbar_create_button(
        ressources_dir, "window-minimize", "titlebutton",
        app_headerbar_minimize_callback, gtk_app
    );
    GtkWidget *btn_maximize = app_headerbar_create_button(
        ressources_dir, "window-maximize", "titlebutton",
        app_headerbar_maximize_callback, gtk_app
    );

    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), btn_close);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), btn_minimize);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), btn_maximize);

    return header_bar;
}

static void app_webview_close_callback(WebKitWebView* Webview, GtkApplication *gtk_app) {
    // also quit the window when webview close
    g_application_quit(G_APPLICATION(gtk_app));
}

static void app_webview_initialize_extensions_callback(WebKitWebContext *webkit_context, char *webextension_dir) {

    char *webextension_file = g_build_filename(webextension_dir, "libweb-extension-proxy.so", NULL);

    if(!g_file_test(webextension_file, G_FILE_TEST_IS_REGULAR)) {
        // extension not found, abort()
        g_error("app:initialize_web_extensions 'libweb-extension-proxy.so' not found");
    }

    //Web Extensions get a different ID for each Web Process
    static int unique_id = 0;

    GVariant *webextension_data = g_variant_new(
        "(is)", (unique_id++), webextension_dir
    );

    // Use one process for each WebKitWebView
    webkit_web_context_set_process_model(webkit_context, WEBKIT_PROCESS_MODEL_MULTIPLE_SECONDARY_PROCESSES);

    webkit_web_context_set_web_extensions_directory(webkit_context, webextension_dir);
    webkit_web_context_set_web_extensions_initialization_user_data(webkit_context, webextension_data);
}

static WebKitWebView *app_webview_create_with_settings(GtkApplication *gtk_app, char *webextension_dir) {

    WebKitSettings *webkit_settings = webkit_settings_new_with_settings(
        "default-charset", "utf8",
        "enable-javascript", TRUE,
        "auto-load-images", TRUE,
        "enable-page-cache", FALSE, // disable cache, we simply use local files
        "allow-file-access-from-file-urls", TRUE, // todo allow xhr request
        "allow-universal-access-from-file-urls", TRUE, // access ressources locally
        "enable-write-console-messages-to-stdout", TRUE, // debug settings
        "enable-developer-extras", TRUE, // todo
    NULL);

    WebKitWebContext *webkit_context = webkit_web_context_get_default();

    // Callback when initialize extensions
    g_signal_connect(webkit_context, "initialize-web-extensions",
        G_CALLBACK(app_webview_initialize_extensions_callback), webextension_dir
    );

    WebKitWebView *webkit_view = WEBKIT_WEB_VIEW(webkit_web_view_new_with_settings(webkit_settings));

    // set default background color (same as title bar + fade effect on css)
    GdkRGBA background_color;
    gdk_rgba_parse(&background_color, "#2c3133");
    webkit_web_view_set_background_color(webkit_view, &background_color);

    // Callback when browser instance is closed, the program will also exit
    g_signal_connect(webkit_view, "close",
        G_CALLBACK(app_webview_close_callback), gtk_app
    );

    return webkit_view;
}



static void app_startup_callback(GtkApplication *gtk_app, WebviewApplication *app) {

    // set application main config variables
    GHashTable *p = g_hash_table_new(g_direct_hash, g_direct_equal);
    g_hash_table_insert(p, "developer_mode", GINT_TO_POINTER(PACKAGE_DEVELOPER_MODE));
    g_hash_table_insert(p, "project_name", PACKAGE_NAME);
    g_hash_table_insert(p, "project_version", PACKAGE_VERSION);
    g_hash_table_insert(p, "pkgressourcesdir", PACKAGE_RESSOURCES_DIR);
    g_hash_table_insert(p, "pkgstylesdir", PACKAGE_STYLES_DIR);
    g_hash_table_insert(p, "pkgwebbundledir", PACKAGE_WEB_BUNDLE_DIR);
    g_hash_table_insert(p, "pkgwebextensionsdir", PACKAGE_WEB_EXTENSIONS_DIR);

    app->package_config = p;

    // load previous window state, if any
    app_window_load_state(gtk_app, &app->window_state);
}

static void app_show_show_interactive_dialog(GtkApplication* gtk_app, WebviewApplication *app) {
    const char *appid = g_application_get_application_id(G_APPLICATION(gtk_app));

    // Get application config
    GHashTable *p = app->package_config;

    // Initialize GTK+
    GtkWidget *main_window = gtk_application_window_new(gtk_app);

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
    char *pkgressourcesdir = g_hash_table_lookup(p, "pkgressourcesdir");
    app->header_bar = app_headerbar_create(gtk_app, pkgressourcesdir);

    // header bar
    gtk_container_set_border_width(GTK_CONTAINER(main_window), 0);
    gtk_window_set_titlebar(GTK_WINDOW(main_window), app->header_bar);

    // Callback when the main window is closed
    g_signal_connect(main_window, "destroy",
        G_CALLBACK(app_window_destroy_callback), gtk_app
    );

    // on change state: minimize, maximize, etc
    g_signal_connect(GTK_WINDOW(main_window), "window-state-event",
        G_CALLBACK(app_window_state_event_callback), &app->window_state
    );

    // on change size
    g_signal_connect(GTK_WINDOW(main_window), "size-allocate",
        G_CALLBACK(app_window_size_allocate_callback), &app->window_state
    );

    // Load window preview state, if any
    WebviewWindowState window_state = app->window_state;

    if(window_state.height > 0 && window_state.width > 0) {
        gtk_window_set_default_size(GTK_WINDOW(main_window),
            window_state.width,
            window_state.height
        );
    }

    if(window_state.is_maximized) {
        gtk_window_maximize(GTK_WINDOW(main_window));
    }

    if(window_state.is_fullscreen) {
        gtk_window_fullscreen(GTK_WINDOW(main_window));
    }

    // Styling application (if file available)

    char *pkgstylesdir = g_hash_table_lookup(p, "pkgstylesdir");
    char *window_style_file = g_build_filename(pkgstylesdir, "style.css", NULL);

    if(g_file_test(window_style_file, G_FILE_TEST_IS_REGULAR)) {

        GtkCssProvider *window_css_provider = gtk_css_provider_get_default();

        GError *css_error = NULL;
        gtk_css_provider_load_from_path(window_css_provider, window_style_file, &css_error);

        if(css_error != NULL) {
            g_warning("app:import style.css %s", css_error->message);
        }

        gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(window_css_provider),
            GTK_STYLE_PROVIDER_PRIORITY_USER
        );
    }

    // Create main webkit2gtk webview
    char *pkgwebextensionsdir = g_hash_table_lookup(p, "pkgwebextensionsdir");
    WebKitWebView *webview = app_webview_create_with_settings(gtk_app, pkgwebextensionsdir);

    // Put the browser area into the main window
    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(webview));

    // Load the selected file into webkit webview
    char *pkgwebbundledir = g_hash_table_lookup(p, "pkgwebbundledir");
    char *webview_page = g_build_filename("file://", pkgwebbundledir, "index.html", NULL);

    webkit_web_view_load_uri(webview, webview_page);

    // Make sure that when the browser area becomes visible, it will get mouse and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(webview));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(main_window);
}

static void app_activate_callback(GtkApplication* gtk_app, WebviewApplication *app) {

    // Check if window is already active
    GtkWindow *window = gtk_application_get_active_window(gtk_app);

    if(window != NULL) {
        gtk_window_present(window);
        return;
    }

    // Else, we show the main window
    app_show_show_interactive_dialog(gtk_app, app);
}

static void app_shutdown_callback(GtkApplication* gtk_app, WebviewApplication *app) {

    // save current window state
    app_window_store_state(gtk_app, &app->window_state);
}

static int app_commandline_callback(GtkApplication* gtk_app, GApplicationCommandLine *cmdline) {
    return 0; // exit
}

static void app_cmdline_print_version(GtkApplication* gtk_app) {
    const char *appid = g_application_get_application_id(G_APPLICATION(gtk_app));

    g_print("%s - GTK:%d.%d.%d WebKit:%d.%d.%d \n", appid,
        gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version(),
        webkit_get_major_version(), webkit_get_minor_version(), webkit_get_micro_version()
    );
}

static int app_handle_local_options_callback(GtkApplication* gtk_app, GVariantDict *options) {
    // handle command lines locally

    if(g_variant_dict_lookup(options, "version", "b", NULL)) {
        app_cmdline_print_version(gtk_app);
        return 0;
    }

    if(g_variant_dict_lookup(options, "inspect", "b", NULL)) {
        gtk_window_set_interactive_debugging(TRUE);
    }

    return -1; //let the default option processing continue
}



int main(int argc, char* argv[]) {

    // main gtk app
    GtkApplication *gtk_app;
    int status;

    #if PACKAGE_DEVELOPER_MODE
        g_message("Dev mode");
    #endif

    // new_ user application (by default the gtk_app is unique)
    WebviewApplication *app = g_malloc(sizeof(WebviewApplication)); // {0}

    // Instantiate the main app
    gtk_app = gtk_application_new("fr.spidery.moviecollection",
        G_APPLICATION_FLAGS_NONE //| G_APPLICATION_HANDLES_COMMAND_LINE
    );

    // Add aplication main events flow
    g_signal_connect(gtk_app, "startup", G_CALLBACK(app_startup_callback), app);
    g_signal_connect(gtk_app, "activate", G_CALLBACK(app_activate_callback), app);
    g_signal_connect(gtk_app, "shutdown", G_CALLBACK(app_shutdown_callback), app);

    // Add app main arguments
    GOptionEntry entries[] = {
        {"version", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL, "Show program version", NULL},
        {"inspect", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL, "Open the interactive debugger", NULL},
        {NULL}
    };
    g_application_add_main_option_entries(G_APPLICATION(gtk_app), entries);

    g_signal_connect(gtk_app, "handle-local-options", G_CALLBACK(app_handle_local_options_callback), NULL);
    g_signal_connect(gtk_app, "command-line", G_CALLBACK(app_commandline_callback), NULL); // received from remote

    // Run the app and get its exit status
    status = g_application_run(G_APPLICATION(gtk_app), argc, argv);

    g_object_unref(gtk_app);

    return status;
}

