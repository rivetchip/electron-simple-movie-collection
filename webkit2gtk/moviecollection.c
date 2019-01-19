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
    GtkApplication parent_instance;

    // application main window
    int win_height;
    int win_width;
    bool is_maximized;
    bool is_fullscreen;

} WebviewApplication;

// typedef GtkApplication WebviewApplication;
typedef GtkApplicationClass WebviewApplicationClass;

G_DEFINE_TYPE(WebviewApplication, webview_application, GTK_TYPE_APPLICATION);


static void webview_application_init(WebviewApplication *app) {
    g_message(__func__);
}

static void webview_application_class_init(WebviewApplicationClass *class) {
    // GApplicationClass *app_class = G_APPLICATION_CLASS(class);

    // app_class->startup = demo_application_startup;
    // app_class->activate = demo_application_activate;
}

WebviewApplication *webview_application_new(const char *application_id, GApplicationFlags flags) {
    
    g_return_val_if_fail(g_application_id_is_valid(application_id), NULL);
    
    return g_object_new(webview_application_get_type(),
        "application-id", application_id,
        "flags", flags,
    NULL);
}




static void app_window_store_state(WebviewApplication *webapp) {
    const char *appid = g_application_get_application_id(G_APPLICATION(webapp));

    GKeyFile *keyfile = g_key_file_new();

    g_key_file_set_integer(keyfile, "WindowState", "height", webapp->win_height);
    g_key_file_set_integer(keyfile, "WindowState", "width", webapp->win_width);
    g_key_file_set_boolean(keyfile, "WindowState", "maximized", webapp->is_maximized);
    g_key_file_set_boolean(keyfile, "WindowState", "fullscreen", webapp->is_fullscreen);

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

static void app_window_load_state(WebviewApplication *webapp) {
    const char *appid = g_application_get_application_id(G_APPLICATION(webapp));

    char *state_file = g_build_filename(g_get_user_cache_dir(), appid, "state.ini", NULL);

    GKeyFile *keyfile = g_key_file_new();

    if(g_key_file_load_from_file(keyfile, state_file, G_KEY_FILE_NONE, NULL)) {

        GError *error_read = NULL;

        int state_height = g_key_file_get_integer(keyfile, "WindowState", "height", &error_read);
        error_read == NULL ? (webapp->win_height = state_height) : g_clear_error(&error_read);

        int state_width = g_key_file_get_integer(keyfile, "WindowState", "width", &error_read);
        error_read == NULL ? (webapp->win_width = state_width) : g_clear_error(&error_read);

        bool state_maximized = g_key_file_get_boolean(keyfile, "WindowState", "maximized", &error_read);
        error_read == NULL ? (webapp->is_maximized = state_maximized) : g_clear_error(&error_read);

        int state_fullscreen = g_key_file_get_boolean(keyfile, "WindowState", "fullscreen", &error_read);
        error_read == NULL ? (webapp->is_fullscreen = state_fullscreen) : g_clear_error(&error_read);
    }

    g_key_file_free(keyfile);
    g_free(state_file);
}

static bool app_window_state_event_callback(GtkWidget *window, GdkEventWindowState *event, WebviewApplication *webapp) {
    GdkWindowState window_state = event->new_window_state; // event->type=GDK_WINDOW_STATE

    webapp->is_maximized = (window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;

    webapp->is_fullscreen = (window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;

    return GDK_EVENT_PROPAGATE;
}

static void app_window_size_allocate_callback(GtkWidget *window, GdkRectangle *allocation, WebviewApplication *webapp) {
    // save the window geometry only if we are not maximized of fullscreen
    if(!(webapp->is_maximized || webapp->is_fullscreen)) {
        // use gtk_ ; Using the allocation directly can lead to growing windows with client-side decorations
        gtk_window_get_size(GTK_WINDOW(window),
            &webapp->win_width,
            &webapp->win_height
        );
    }
}

static void app_window_destroy_callback(GtkWidget *window, WebviewApplication *webapp) {
    g_application_quit(G_APPLICATION(webapp));
}

// create a gtk button with an icon inside
static GtkWidget *app_headerbar_create_button(
    const char *ressources_dir,
    const char *icon_name,
    const char *class_name,
    const void *click_event,
    WebviewApplication *webapp
) {
    char icon_svg[20];
    sprintf(icon_svg, "%s.svg", icon_name);

    char *icon_path = g_build_filename(ressources_dir, icon_svg, NULL);

    GtkWidget *gtk_button = gtk_button_new();

    if(class_name != NULL) {
        gtk_style_context_add_class(gtk_widget_get_style_context(gtk_button), class_name);
    }

    if(click_event != NULL) {
        g_signal_connect(gtk_button, "clicked", G_CALLBACK(click_event), webapp);
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

static void app_headerbar_close_callback(GtkButton* button, WebviewApplication *webapp) {
    g_application_quit(G_APPLICATION(webapp));
}

static void app_headerbar_minimize_callback(GtkButton* button, WebviewApplication *webapp) {
    GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if(GTK_IS_WINDOW(toplevel)) {
        GtkWindow *gtk_window = GTK_WINDOW(toplevel);
        gtk_window_iconify(gtk_window);
    }
}

static void app_headerbar_maximize_callback(GtkButton* button, WebviewApplication *webapp) {
    GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if(GTK_IS_WINDOW(toplevel)) {
        GtkWindow *gtk_window = GTK_WINDOW(toplevel);
        gtk_window_is_maximized(gtk_window) ? gtk_window_unmaximize(gtk_window) : gtk_window_maximize(gtk_window);
    }
}

static GtkWidget *app_headerbar_create(WebviewApplication *webapp, char* ressources_dir) {

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
        app_headerbar_close_callback, webapp
    );
    GtkWidget *btn_minimize = app_headerbar_create_button(
        ressources_dir, "window-minimize", "titlebutton",
        app_headerbar_minimize_callback, webapp
    );
    GtkWidget *btn_maximize = app_headerbar_create_button(
        ressources_dir, "window-maximize", "titlebutton",
        app_headerbar_maximize_callback, webapp
    );

    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), btn_close);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), btn_minimize);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), btn_maximize);

    return header_bar;
}

static void webkit_view_close_callback(WebKitWebView* Webview, WebviewApplication *webapp) {
    // also quit the window when webview close
    g_application_quit(G_APPLICATION(webapp));
}

static void webkit_context_initialize_extensions_callback(WebKitWebContext *webkit_context, WebviewApplication *webapp) {
    const char *appid = g_application_get_application_id(G_APPLICATION(webapp));

    char *webext_file = g_build_filename(PACKAGE_WEB_EXTENSIONS_DIR, "libweb-extension-proxy.so", NULL);

    if(!g_file_test(webext_file, G_FILE_TEST_IS_REGULAR)) {
        // extension not found, abort()
        g_error("app:initialize web extension '%s' not found", webext_file);
    }

    //Web Extensions get a different ID for each Web Process
    static int unique_id = 0;

    GVariant *webext_data = g_variant_new(
        "(iss)", (unique_id++), appid, webext_file
    );

    // Use one process for each WebKitWebView
    webkit_web_context_set_process_model(webkit_context, WEBKIT_PROCESS_MODEL_MULTIPLE_SECONDARY_PROCESSES);

    webkit_web_context_set_web_extensions_directory(webkit_context, PACKAGE_WEB_EXTENSIONS_DIR);
    webkit_web_context_set_web_extensions_initialization_user_data(webkit_context, webext_data);
}

static WebKitWebView *webview_create_with_settings(WebviewApplication *webapp) {

    WebKitSettings *webkit_settings = webkit_settings_new_with_settings(
        "default-charset", "utf8",
        "enable-javascript", TRUE,
        // always load images
        "auto-load-images", TRUE,
        // disable cache, we don't need it because we only use static html files
        "enable-page-cache", FALSE,
        // Whether file access is allowed from file URLs
        "allow-file-access-from-file-urls", TRUE,
        // Whether or not JavaScript running in the context of a file scheme URL should be allowed to access content from any origin
        "allow-universal-access-from-file-urls", TRUE,

        // disable unused features active by default
        "enable-plugins", FALSE,
        "enable-java", FALSE,
        "enable-html5-database", FALSE,

        #if PACKAGE_DEVELOPER_MODE
        "enable-developer-extras", TRUE,
        // "enable-write-console-messages-to-stdout", TRUE,
        #endif
    NULL);

    // An ephemeral context will handles all websites data as non-persistent, this is normally used to implement private instances
    WebKitWebContext *webkit_context = webkit_web_context_new_ephemeral();

    // Callback when initialize extensions (cannot init extension on new context)
    g_signal_connect(webkit_context, "initialize-web-extensions",
        G_CALLBACK(webkit_context_initialize_extensions_callback), webapp
    );

    // WebKitWebView *webkit_view = WEBKIT_WEB_VIEW(webkit_web_view_new_with_settings(webkit_settings));
    WebKitWebView *webkit_view = WEBKIT_WEB_VIEW(g_object_new(WEBKIT_TYPE_WEB_VIEW,
        "web-context", webkit_context,
        "settings", webkit_settings,
        "is-ephemeral", webkit_web_context_is_ephemeral(webkit_context),
    NULL));

    // set default background color (same as title bar + fade effect on css)
    GdkRGBA background_color;
    gdk_rgba_parse(&background_color, "#2c3133");
    webkit_web_view_set_background_color(webkit_view, &background_color);

    // Callback when browser instance is closed, the program will also exit
    g_signal_connect(webkit_view, "close", G_CALLBACK(webkit_view_close_callback), webapp);

    return webkit_view;
}


static void app_startup_callback(WebviewApplication *webapp) {

    // set application main config variables

    // load previous window state, if any
    app_window_load_state(webapp);
}

static void app_show_interactive_dialog(WebviewApplication* webapp) {
    const char *appid = g_application_get_application_id(G_APPLICATION(webapp));

    // Initialize GTK+
    GtkWidget *main_window = gtk_application_window_new(GTK_APPLICATION(webapp));

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
    GtkWidget *header_bar = app_headerbar_create(webapp, PACKAGE_RESSOURCES_DIR);

    // header bar
    gtk_container_set_border_width(GTK_CONTAINER(main_window), 0);
    gtk_window_set_titlebar(GTK_WINDOW(main_window), header_bar);

    // Callback when the main window is closed
    g_signal_connect(main_window, "destroy",
        G_CALLBACK(app_window_destroy_callback), webapp
    );

    // on change state: minimize, maximize, etc
    g_signal_connect(GTK_WINDOW(main_window), "window-state-event",
        G_CALLBACK(app_window_state_event_callback), webapp
    );

    // on change size
    g_signal_connect(GTK_WINDOW(main_window), "size-allocate",
        G_CALLBACK(app_window_size_allocate_callback), webapp
    );

    // Load window preview state, if any

    if(webapp->win_height > 0 && webapp->win_width > 0) {
        gtk_window_set_default_size(GTK_WINDOW(main_window),
            webapp->win_width,
            webapp->win_height
        );
    }

    if(webapp->is_maximized) {
        gtk_window_maximize(GTK_WINDOW(main_window));
    }

    if(webapp->is_fullscreen) {
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

    // Create main webkit2gtk webview
    WebKitWebView *webview = webview_create_with_settings(webapp);

    // Put the browser area into the main window
    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(webview));

    // Load the selected file into webkit webview
    char *webview_page = g_build_filename("file://", PACKAGE_WEB_BUNDLE_DIR, "index.html", NULL);

    webkit_web_view_load_uri(webview, webview_page);

    // Make sure that when the browser area becomes visible, it will get mouse and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(webview));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(main_window);
}

static void app_activate_callback(WebviewApplication* webapp) {

    // Check if window is already active
    GtkWindow *window = gtk_application_get_active_window(GTK_APPLICATION(webapp));

    if(window != NULL) {
        gtk_window_present(window);
        return;
    }

    // Else, we show the main window
    app_show_interactive_dialog(webapp);
}

static void app_shutdown_callback(WebviewApplication* webapp) {

    // save current window state
    app_window_store_state(webapp);
}

static int app_commandline_callback(WebviewApplication* webapp, GApplicationCommandLine *cmdline) {
    return 0; // exit
}

static void app_commandline_print_version(WebviewApplication* webapp) {
    const char *appid = g_application_get_application_id(G_APPLICATION(webapp));

    g_print("%s - GTK:%d.%d.%d WebKit:%d.%d.%d \n", appid,
        gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version(),
        webkit_get_major_version(), webkit_get_minor_version(), webkit_get_micro_version()
    );
}

static int app_handle_local_options_callback(WebviewApplication* webapp, GVariantDict *options) {
    // handle command lines locally

    if(g_variant_dict_lookup(options, "version", "b", NULL)) {
        app_commandline_print_version(webapp);
        return 0;
    }

    if(g_variant_dict_lookup(options, "inspect", "b", NULL)) {
        gtk_window_set_interactive_debugging(TRUE);
    }

    return -1; //let the default option processing continue
}



int main(int argc, char* argv[]) {

    // hacks workaround slow computers
    putenv("WEBKIT_DISABLE_COMPOSITING_MODE=1");


    #if PACKAGE_DEVELOPER_MODE
        g_message("Dev mode");

        // inspector debug
        // putenv("GTK_DEBUG=all");
        putenv("GOBJECT_DEBUG=instance-count");
    #endif


    int status;

    // Instantiate the main app
    WebviewApplication *webapp = webview_application_new(
        PACKAGE_APPLICATION_ID,
        G_APPLICATION_FLAGS_NONE //| G_APPLICATION_HANDLES_COMMAND_LINE
    );

    // Add aplication main events flow (start with "_init")
    g_signal_connect(webapp, "startup", G_CALLBACK(app_startup_callback), NULL);
    g_signal_connect(webapp, "activate", G_CALLBACK(app_activate_callback), NULL);
    g_signal_connect(webapp, "shutdown", G_CALLBACK(app_shutdown_callback), NULL);
    // g_signal_connect(webapp, "open", G_CALLBACK(app_open_callback), NULL);

    // Add app main arguments
    GOptionEntry entries[] = {
        {"version", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL, "Show program version", NULL},
        {"inspect", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL, "Open the interactive debugger", NULL},
        {NULL}
    };
    g_application_add_main_option_entries(G_APPLICATION(webapp), entries);

    g_signal_connect(webapp, "handle-local-options", G_CALLBACK(app_handle_local_options_callback), NULL);
    g_signal_connect(webapp, "command-line", G_CALLBACK(app_commandline_callback), NULL); // received from remote

    // Run the app and get its exit status
    status = g_application_run(G_APPLICATION(webapp), argc, argv);

    g_object_unref(webapp);

    return status;
}

