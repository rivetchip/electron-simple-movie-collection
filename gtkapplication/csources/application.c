
#include <config.h>
#include "application.h"

G_DEFINE_TYPE(MovieApplication, movie_application, GTK_TYPE_APPLICATION);

static void signal_startup(MovieApplication *app);
static void signal_activate(MovieApplication *app);
static void signal_shutdown(MovieApplication *app);
static void signal_open(MovieApplication *app);
static int signal_command_line(MovieApplication *app, GApplicationCommandLine *cmdline);
static int signal_handle_local_options(MovieApplication *app, GVariantDict *options);
static void signal_network_changed(GNetworkMonitor *monitor, bool available, MovieApplication *app);
static GtkCssProvider *load_styles_resources();
static void signal_css_parsing_error(GtkCssProvider *provider, GtkCssSection *section, GError *error);
static void commandline_print_version(MovieApplication *app);


static void movie_application_class_init(MovieApplicationClass *klass) {
	// GObjectClass *object_class = G_OBJECT_CLASS (klass);
	// GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

    // app_class->startup = demo_application_startup;
    // app_class->activate = demo_application_activate;
}

static void movie_application_init(MovieApplication *app) {
    g_message(__func__);

    // set network monitor
	app->monitor = g_network_monitor_get_default();
	g_signal_connect(app->monitor, "network-changed", G_CALLBACK(signal_network_changed), app);

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

MovieApplication *movie_application_new(const char *application_id, GApplicationFlags flags) {

    g_return_val_if_fail(g_application_id_is_valid(application_id), NULL); // normal comportment
    
    return g_object_new(movie_application_get_type(),
        "application-id", application_id,
        "flags", flags,
    NULL);
}

static void signal_startup(MovieApplication *app) {

    // set application main config variables

    // load previous window state, if any
    // mainwindow_load_state(app);


    // styling application
    GtkCssProvider *css_provider = load_styles_resources();
    g_signal_connect(css_provider, "parsing-error", G_CALLBACK(signal_css_parsing_error), NULL);

    // add_accelerator


}

static void signal_activate(MovieApplication *app) {
    g_message(__func__);

    // Check if window is already active
    // MovieWindow *window = gtk_application_get_active_window(GTK_APPLICATION(app));

    // if(window != NULL) {
    //     // create if not exist
    //     window = movie_appplication_create_window(app, NULL);
    //     gtk_widget_show(GTK_WIDGET(window));
    // }

    // gtk_window_present(GTK_WINDOW(window));
}

static void signal_shutdown(MovieApplication *app) {
    g_message(__func__);

    // save current window state
    // mainwindow_store_state(app);
}

static void signal_open(MovieApplication *app) {
    g_message(__func__);


}

static int signal_command_line(MovieApplication *app, GApplicationCommandLine *cmdline) {
    return 0; // exit todo
}

static int signal_handle_local_options(MovieApplication *app, GVariantDict *options) {
    // handle command lines locally

    if(g_variant_dict_lookup(options, "version", "b", NULL)) {
        commandline_print_version(app);
        return 0;
    }

    if(g_variant_dict_lookup(options, "inspect", "b", NULL)) {
        gtk_window_set_interactive_debugging(TRUE);
    }

    return -1; //let the default option processing continue
}

static void signal_network_changed(GNetworkMonitor *monitor, bool available, MovieApplication *app) {
    g_message(__func__);
    //@todo
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

    g_print("%s - GTK:%d.%d.%d \n", appid,
        gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version()
    );
}

