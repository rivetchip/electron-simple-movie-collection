#include "application.h"
#include "window.h"

// type definition
struct _MovieApplication {
    GtkApplication parent_instance;

    GNetworkMonitor *monitor;
    const char *build_version;
};

enum {
    PROP_BUILD_VERSION = 1,
    PROP_LAST
};
static GParamSpec *properties[PROP_LAST];

G_DEFINE_TYPE(MovieApplication, movie_application, GTK_TYPE_APPLICATION);

// internals
static void get_property(GObject *object, unsigned int property_id, GValue *value, GParamSpec *pspec);
static void set_property(GObject *object, unsigned int property_id, const GValue *value, GParamSpec *pspec);
// signals
static void signal_startup(MovieApplication *app);
static void signal_activate(MovieApplication *app);
static void signal_shutdown(MovieApplication *app);
static void signal_open(MovieApplication *app, GFile **files, int n_files, const gchar *hint);
static int signal_command_line(MovieApplication *app, GApplicationCommandLine *cmdline);
static int signal_handle_local_options(MovieApplication *app, GVariantDict *options);
static void signal_network_changed(GNetworkMonitor *monitor, bool available, MovieApplication *app);
// styling
static GtkCssProvider *load_styles_resources();
static void signal_css_parsing_error(GtkCssProvider *provider, GtkCssSection *section, GError *error);
static void commandline_print_version(MovieApplication *app);


static void movie_application_class_init(MovieApplicationClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // GApplicationClass *app_class = G_APPLICATION_CLASS(klass);

    object_class->get_property = get_property;
    object_class->set_property = set_property;

    properties[PROP_BUILD_VERSION] = g_param_spec_string(
        "build-version", "build version", "Application build version",
        NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS // default, flags
    );

    g_object_class_install_properties(object_class, PROP_LAST, properties);
}

MovieApplication *movie_application_new(const char *application_id, const char *build_version, GApplicationFlags flags) {
    g_message(__func__);

    g_return_val_if_fail(g_application_id_is_valid(application_id), NULL);
    
    return g_object_new(movie_application_get_type(),
        "application-id", application_id,
        "build-version", build_version,
        "flags", flags,
    NULL);
}

static void movie_application_init(MovieApplication *app) {
    const char *appid = g_application_get_application_id(G_APPLICATION(app));

    gtk_window_set_default_icon_name(appid);

    // set network monitor
	GNetworkMonitor *monitor = g_network_monitor_get_default();
	g_signal_connect(monitor, "network-changed", G_CALLBACK(signal_network_changed), app);
    app->monitor = monitor;

    // set command line options
    GOptionEntry options[] = {
        {"version", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL, "Show program version", NULL},
        {"inspect", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL, "Open the interactive debugger", NULL},
        {NULL}
    };
    g_application_add_main_option_entries(G_APPLICATION(app), options);

    // Add aplication events flow
    g_signal_connect(app, "startup", G_CALLBACK(signal_startup), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(signal_activate), NULL);
    g_signal_connect(app, "shutdown", G_CALLBACK(signal_shutdown), NULL);
    g_signal_connect(app, "open", G_CALLBACK(signal_open), NULL);

    g_signal_connect(app, "command-line", G_CALLBACK(signal_command_line), NULL); // received from remote
    g_signal_connect(app, "handle-local-options", G_CALLBACK(signal_handle_local_options), NULL);
}

static void get_property(GObject *object, unsigned int property_id, GValue *value, GParamSpec *pspec) {
    MovieApplication *app = MOVIE_APPLICATION(object);

    switch(property_id) {
        case PROP_BUILD_VERSION:
            g_value_set_string(value, app->build_version);
        break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void set_property(GObject *object, unsigned int property_id, const GValue *value, GParamSpec *pspec) {
    MovieApplication *app = MOVIE_APPLICATION(object);

    switch(property_id) {
        case PROP_BUILD_VERSION:
            app->build_version = g_value_dup_string(value);
        break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}


static void signal_startup(MovieApplication *app) {
    g_message(__func__);

    // set window settings
    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(G_OBJECT(settings),
        "gtk-application-prefer-dark-theme", true,
        // "gtk-font-name", "Lato 12",
    NULL);

    // styling application
    GtkCssProvider *css_provider = load_styles_resources();
    g_signal_connect(css_provider, "parsing-error", G_CALLBACK(signal_css_parsing_error), NULL);


    // load settings

    // add_accelerator

}

static GtkWindow *show_interactive_dialog() {
    //todo with open
}

static void signal_activate(MovieApplication *app) {
    g_message(__func__);

    GtkWindow *window;
    if((window = gtk_application_get_active_window(GTK_APPLICATION(app))) == NULL) {
        // create if not exist
        window = GTK_WINDOW(movie_application_new_window(app, NULL));
        gtk_widget_show(GTK_WIDGET(window));
    }

    gtk_window_present(window);
}

static void signal_shutdown(MovieApplication *app) {
    g_message(__func__);

    // save current window state
    // mainwindow_store_state(app);
}

static void signal_open(MovieApplication *app, GFile **files, int n_files, const gchar *hint) {
    g_message(__func__);

    //todo
    for(int i = 0; i < n_files; i++) {
        char *uri = g_file_get_uri(files[i]);
        g_print("open %s\n", uri);
        g_free (uri);
    }
}

static int signal_command_line(MovieApplication *app, GApplicationCommandLine *cmdline) {
    return 0; // exit
}

static int signal_handle_local_options(MovieApplication *app, GVariantDict *options) {
    g_message(__func__);

    // handle command lines locally

    if(g_variant_dict_lookup(options, "version", "b", NULL)) {
        commandline_print_version(app);
        return 0;
    }

    if(g_variant_dict_lookup(options, "inspect", "b", NULL)) {
        gtk_window_set_interactive_debugging(true);
    }

    return -1; //let the default option processing continue
}

static void signal_network_changed(GNetworkMonitor *monitor, bool available, MovieApplication *app) {
    g_message(__func__);

    bool enabled = g_network_monitor_get_network_available(monitor);
}


static GtkCssProvider *load_styles_resources() {

    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(css_provider, "/shell/style.css");

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    gtk_icon_theme_add_resource_path(gtk_icon_theme_get_default(), "/icons");

    return css_provider;
}

static void signal_css_parsing_error(GtkCssProvider *provider, GtkCssSection *section, GError *error) {

    g_warning("Theme parsing error: %u:%u %s",
        gtk_css_section_get_start_line(section),
        gtk_css_section_get_start_position(section),
        error->message
    );
}

static void commandline_print_version(MovieApplication *app) {
    const char *appid = g_application_get_application_id(G_APPLICATION(app));

    g_print("%s:%s - GTK:%d.%d.%d \n", appid, app->build_version,
        gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version()
    );
}






GKeyFile *movie_application_get_keyfile(MovieApplication *app, const char *keyname) {
    const char *appid = g_application_get_application_id(G_APPLICATION(app));

    char *filename = g_build_filename(g_get_user_config_dir(), appid, keyname, NULL);

    GError *error = NULL;
    GKeyFile *keyfile = g_key_file_new();
    if(!g_key_file_load_from_file(keyfile, filename, G_KEY_FILE_NONE, &error)) {
        g_message("%s %s: %s", __func__, error->message, keyname);
        g_clear_error(&error);
        return NULL;
    }

    g_free(filename);
    return keyfile;
}

bool movie_application_set_keyfile(MovieApplication *app, const char *keyname, GKeyFile *keyfile) {
    const char *appid = g_application_get_application_id(G_APPLICATION(app));

    char *filepath = g_build_filename(g_get_user_config_dir(), appid, NULL);
    char *filename = g_build_filename(filepath, keyname, NULL);

    // create save path if not set
    if(g_mkdir_with_parents(filepath, 0755) != 0) { // error=-1 exist=0
        return false;
    }

    GError *error = NULL;
    if(!g_key_file_save_to_file(keyfile, filename, &error)) {
        g_message("%s %s: %s", __func__, error->message, keyname);
        g_clear_error(&error);
        return false;
    }

    g_free(filepath);
    g_free(filename);

    return true;
}


