/*
sudo dnf install gtk3-devel gstreamer-devel clutter-devel webkitgtk3-devel libgda-devel gobject-introspection-devel
webkit2gtk3-devel

gcc moviecollection.c -o main `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` && ./main

meson
*/

#include <stdlib.h>
#include <glib.h>

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>


static void destroy_window_callback(
    GtkWidget* widget,
    GtkWidget* window
);

static gboolean close_webview_callback(
    WebKitWebView *webView,
    GtkWidget* window
);


int main(int argc, char* argv[]) {

    bool is_debug = argv[1] != NULL && strcmp(argv[1], "--debug") == 0;

    if(is_debug) {
        g_message("app:is_debug");
    }

    // get current application path
    char *cwd = getcwd(NULL, 0);

    // Initialize GTK+
    gtk_init(&argc, &argv);

    // Create an 800x600 window that will contain the browser instance
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    // Link proxy extension to provide bridge javascript/Native API
    static guint32 unique_id = 0; //get a different ID for each Web Process
    
    char webextension_dir[256];
    sprintf(webextension_dir, "%s/", cwd);

    WebKitWebContext *webkit_context = webkit_web_context_get_default();
    webkit_web_context_set_web_extensions_directory(webkit_context, webextension_dir);

    // arguments passed to proxy extension
    GVariant *user_data = g_variant_new(
        "(i)", unique_id++
    );

    webkit_web_context_set_web_extensions_initialization_user_data(webkit_context, user_data);

    // Create a browser instance
    WebKitWebView *webview = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // Set GTK window options
    gtk_window_set_title(GTK_WINDOW(main_window), "Movie Collection");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(main_window), true);

    // Put the browser area into the main window
    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(webview));

    // Set up callbacks so that if either the main window or the browser instance is
    // closed, the program will exit
    g_signal_connect(main_window, "destroy", G_CALLBACK(destroy_window_callback), NULL);
    g_signal_connect(webview, "close", G_CALLBACK(close_webview_callback), main_window);

    // Webview settings (debug)
    WebKitSettings *websettings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(webview));
    webkit_settings_set_default_charset(websettings, "utf8");
    webkit_settings_set_enable_javascript(websettings, true);

    if(is_debug) {
        webkit_settings_set_enable_write_console_messages_to_stdout(websettings, true);
        webkit_settings_set_enable_developer_extras(websettings, true);
    }

    // Load a web page into the browser instance

    char webview_page[256];
    sprintf(webview_page, "file://%s/bundle/index.html", cwd);

    webkit_web_view_load_uri(webview, webview_page);

    // Make sure that when the browser area becomes visible, it will get mouse and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(webview));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(main_window);

    // Run the main GTK+ event loop
    gtk_main();

    return 0;
}

static void destroy_window_callback(GtkWidget* widget, GtkWidget* window) {
    gtk_main_quit();
}

static gboolean close_webview_callback(WebKitWebView* Webview, GtkWidget* window) {
    gtk_widget_destroy(window);
    return true;
}

