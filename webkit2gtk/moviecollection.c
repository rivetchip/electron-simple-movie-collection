/*
sudo dnf install gtk3-devel gstreamer-devel clutter-devel webkitgtk3-devel libgda-devel gobject-introspection-devel
webkit2gtk3-devel

gcc moviecollection.c -o main `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` && ./main
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <JavaScriptCore/JavaScript.h>


static void destroyWindowCallback(
    GtkWidget* widget,
    GtkWidget* window
);
static gboolean closeWebViewCallback(
    WebKitWebView* webView,
    GtkWidget* window
);

static void external_message_received(
    WebKitUserContentManager *userContentManager,
    WebKitJavascriptResult *javascriptResult,
    gpointer arg
);


int main(int argc, char* argv[]) {

    bool is_debug = argv[1] != NULL && strcmp(argv[1], "--debug") == 0;

    // Initialize GTK+
    gtk_init(&argc, &argv);

    // Create an 800x600 window that will contain the browser instance
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);

    // Set GTK window options
    gtk_window_set_title(GTK_WINDOW(main_window), "Movie Collection");
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(main_window), true);

    // Create a browser instance
    WebKitWebView *webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // create user content
    WebKitUserContentManager *webKitUserContentManager = webkit_user_content_manager_new();
    webkit_user_content_manager_register_script_message_handler(webKitUserContentManager, "external");
    g_signal_connect(webKitUserContentManager, "script-message-received::external", G_CALLBACK(external_message_received), NULL);

    // Put the browser area into the main window
    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(webView));

    // Set up callbacks so that if either the main window or the browser instance is
    // closed, the program will exit
    g_signal_connect(main_window, "destroy", G_CALLBACK(destroyWindowCallback), NULL);
    g_signal_connect(webView, "close", G_CALLBACK(closeWebViewCallback), main_window);

    // Webview settings (debug)
    WebKitSettings *webSettings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(webView));
    webkit_settings_set_default_charset(webSettings, "utf8");

    if(is_debug) {
        webkit_settings_set_enable_write_console_messages_to_stdout(webSettings, true);
        webkit_settings_set_enable_developer_extras(webSettings, true);
    }

    // Load a web page into the browser instance
    char cwd[256];
    getcwd(cwd, sizeof(cwd));

    char webView_page[256];
    sprintf(webView_page, "file://%s/bundle/index.html", cwd);

    webkit_web_view_load_uri(webView, webView_page);

    // Make sure that when the browser area becomes visible, it will get mouse and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(webView));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(main_window);

    // Run the main GTK+ event loop
    gtk_main();

    return 0;
}

static void destroyWindowCallback(GtkWidget* widget, GtkWidget* window) {
    gtk_main_quit();
}

static gboolean closeWebViewCallback(WebKitWebView* webView, GtkWidget* window) {
    gtk_widget_destroy(window);
    return TRUE;
}


static void external_message_received(WebKitUserContentManager *userContentManager, WebKitJavascriptResult *javascriptResult, gpointer arg) {


}


