#include <config.h>
#include "application.h"
#include "window.h"

// type definition
struct _MovieApplication {
    GtkApplication parent_instance;

    GNetworkMonitor *monitor;
    const char *build_version;

    GKeyFile *settings;
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
static void commandline_print_version(MovieApplication *app);
// signals
static void signal_startup(MovieApplication *app);
static void signal_activate(MovieApplication *app);
static void signal_shutdown(MovieApplication *app);
static void signal_open(MovieApplication *app, GFile **files, int n_files, const gchar *hint);
static int signal_command_line(MovieApplication *app, GApplicationCommandLine *cmdline);
static int signal_handle_local_options(MovieApplication *app, GVariantDict *options);
static void signal_network_changed(GNetworkMonitor *monitor, bool available, MovieApplication *app);
// window
static MovieWindow *application_new_window(MovieApplication *app);
static void signal_window_destroy(MovieWindow *window);
// settings file
static GKeyFile *application_get_settings_keyfile(MovieApplication *app);
static bool application_save_settings_keyfile(MovieApplication *app, GKeyFile *settings);
// styling
static GtkCssProvider *load_styles_resources();
static void signal_css_parsing_error(GtkCssProvider *provider, GtkCssSection *section, GError *error);
// actions
static void add_accelerator(MovieApplication *app, const char *action_name, const char *accel);
static void action_help(GSimpleAction *action, GVariant *parameter, gpointer user_data);


MovieApplication *movie_application_new(const char *application_id, const char *build_version, GApplicationFlags flags) {
    g_message(__func__);

    g_return_val_if_fail(g_application_id_is_valid(application_id), NULL);
    
    return g_object_new(movie_application_get_type(),
        "application-id", application_id,
        "build-version", build_version,
        "flags", flags,
    NULL);
}

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

    // set actions
    static GActionEntry actions[] = {
        {"help", action_help}
    };
    // https://developer.gnome.org/gio/stable/GActionMap.html#g-action-map-add-action
	g_action_map_add_action_entries(G_ACTION_MAP(app), actions, G_N_ELEMENTS(actions), app);

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

    // load settings file
    GKeyFile *settings;
    if((settings = application_get_settings_keyfile(app)) != NULL) {
        app->settings = settings;
    }

    // set window settings
    GtkSettings *gtksettings = gtk_settings_get_default();
    g_object_set(G_OBJECT(gtksettings),
        "gtk-theme-name", "Adwaita-dark",
        "gtk-application-prefer-dark-theme", true,
        // "gtk-font-name", "Lato 12",
        "gtk-recent-files-enabled", true,
        "gtk-shell-shows-menubar", false,
    NULL);

    // styling application
    GtkCssProvider *css_provider;
    if((css_provider = load_styles_resources())) {
        g_signal_connect(css_provider, "parsing-error", G_CALLBACK(signal_css_parsing_error), NULL);
    }

    // add accelerators
    add_accelerator(app, "app.help", "F1");
    add_accelerator(app, "win.open", "<Primary>O");
    add_accelerator(app, "win.save", "<Primary>S");
    add_accelerator(app, "win.save-as", "<Primary><Shift>S");
    add_accelerator(app, "win.newitem", "<Primary>N");
    // window
    add_accelerator(app, "win.close", "<Primary>W");
    add_accelerator(app, "win.fullscreen", "F11");
    // menu
    add_accelerator(app, "win.prefs", "<Primary>P");
}

static MovieWindow *application_new_window(MovieApplication *app) {

    MovieWindow *window = movie_window_new(app->settings);
    gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));

    g_signal_connect(window, "destroy", G_CALLBACK(signal_window_destroy), NULL);

    return window;
}

static void signal_activate(MovieApplication *app) {
    g_message(__func__);

    GtkWindow *window;
    if((window = gtk_application_get_active_window(GTK_APPLICATION(app))) == NULL) {
        // create if not exist
        window = GTK_WINDOW(application_new_window(app));
    }

    gtk_window_present(window);
}

static void signal_shutdown(MovieApplication *app) {
    g_message(__func__);

    // last window is gone... save settings and exit
    GKeyFile *settings;
    if((settings = app->settings) != NULL) {
        application_save_settings_keyfile(app, settings);
    }
}

static void signal_open(MovieApplication *app, GFile **files, int n_files, const gchar *hint) {
    g_message("app:%s", __func__);

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

    bool enabled = g_network_monitor_get_network_available(monitor); // todo
}

static void add_accelerator(MovieApplication *app, const char *action_name, const char *accel) {
    const char *vaccels[] = {accel, NULL};
    gtk_application_set_accels_for_action(GTK_APPLICATION(app), action_name, vaccels);
}

static void action_help(GSimpleAction *action, GVariant *parameter, gpointer user_data) {

    
    g_message("app:%s", __func__);
    // MovieApplication *app = MOVIE_APPLICATION(user_data);
}



static GtkCssProvider *load_styles_resources() {

    GtkCssProvider *css_provider = gtk_css_provider_new();

    #if PACKAGE_DEVELOPER_MODE
        gtk_css_provider_load_from_path(css_provider, "style.css", NULL);
    #else
        gtk_css_provider_load_from_resource(css_provider, "/shell/style.css");
    #endif

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

    g_print("%s:%s - GTK:%d.%d.%d \n",
        appid, app->build_version,
        gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version()
    );
}



static void signal_window_destroy(MovieWindow *window) {
    GtkApplication *gtkapp = gtk_window_get_application(GTK_WINDOW(window));
    g_application_quit(G_APPLICATION(gtkapp));
}




static GKeyFile *application_get_settings_keyfile(MovieApplication *app) {
    const char *appid = g_application_get_application_id(G_APPLICATION(app));
    const char *filename = g_build_filename(g_get_user_config_dir(), appid, "settings.ini", NULL);

    GError *error = NULL;
    GKeyFile *settings = g_key_file_new();
    if(!g_key_file_load_from_file(settings, filename, G_KEY_FILE_NONE, &error)) {
        g_warning("%s: %s", __func__, error->message);
        g_clear_error(&error);
    }

    g_free((char*) filename);
    return settings;
}

static bool application_save_settings_keyfile(MovieApplication *app, GKeyFile *settings) {
    const char *appid = g_application_get_application_id(G_APPLICATION(app));
    const char *filepath = g_build_filename(g_get_user_config_dir(), appid, NULL);
    const char *filename = g_build_filename(filepath, "settings.ini", NULL);

    // create save path if not set
    if(g_mkdir_with_parents(filepath, 0755) != 0) { // error=-1 exist=0
        return false;
    }

    GError *error = NULL;
    if(!g_key_file_save_to_file(settings, filename, &error)) {
        g_warning("%s: %s", __func__, error->message);
        g_clear_error(&error);
        return false;
    }

    g_free((char*) filepath);
    g_free((char*) filename);

    return true;
}


