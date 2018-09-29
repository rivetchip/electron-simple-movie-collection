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


static void destroy_window_callback(GtkWidget* widget, GtkWidget* window) {
    gtk_main_quit();
}

static gboolean close_webview_callback(WebKitWebView* Webview, GtkWidget* window) {
    gtk_widget_destroy(window);
    return true;
}

static void initialize_web_extensions(WebKitWebContext *webkit_context, GVariant *user_data) {
    int unique_id;
    char *webextension_dir;

    g_variant_get(user_data, "(is)", &unique_id, &webextension_dir);

    webkit_web_context_set_web_extensions_directory(webkit_context, webextension_dir);
    webkit_web_context_set_web_extensions_initialization_user_data(webkit_context, user_data);
}


int main(int argc, char* argv[]) {

    bool is_debug = argv[1] != NULL && strcmp(argv[1], "--debug") == 0;

    if(is_debug) {
        g_message("app:is_debug");
    }

    // get current application path
    char launcher_dir[200];
    getcwd(launcher_dir, sizeof(launcher_dir));

    if(is_debug) {
        g_message("app:launcher_dir %s", launcher_dir);
    }

    //get a different ID for each Web Process
    static int unique_id = 0;

    char webextension_dir[256];
    sprintf(webextension_dir, "%s/", launcher_dir);

    if(is_debug) {
        g_message("app:webextension_dir %s", webextension_dir);
    }

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










    // Set GTK CSD HeaderBar
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_widget_set_name(header_bar, "header_bar");

    gtk_window_set_titlebar(GTK_WINDOW(main_window), header_bar);

    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Movie Collection");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), true);



    // GtkWidget *close_icon = gtk_image_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_MENU);
    // GtkWidget *close_btn = gtk_button_new();

    // gtk_style_context_add_class(gtk_widget_get_style_context(close_btn), "titlebutton");

    // gtk_container_add(GTK_CONTAINER(close_btn), close_icon);

    // gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), close_btn);

















    // styling application
    GtkCssProvider *window_css_provider = gtk_css_provider_get_default();

    char window_style_dir[256];
    sprintf(window_style_dir, "%s/style.css", launcher_dir);

    if(is_debug) {
        g_message("app:window_style_dir %s", window_style_dir);
    }

    GError **css_error = NULL;
    gtk_css_provider_load_from_path(window_css_provider, window_style_dir, css_error);

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(window_css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    // Callback when the main window is closed
    g_signal_connect(main_window, "destroy", G_CALLBACK(destroy_window_callback), NULL);


    // Webview settings & create the webview
    WebKitSettings *websettings = webkit_settings_new_with_settings(
        "default-charset", "utf8",
        "enable-javascript", true,
        "auto-load-images", false,
        "allow-file-access-from-file-urls", true, // allow xhr request
        "enable-write-console-messages-to-stdout", is_debug, // debug settings
        "enable-developer-extras", is_debug,
    NULL);

    // arguments passed to proxy extension

    WebKitWebContext *webkit_context = webkit_web_context_get_default();// todo webkit_web_view_get_context(webview);
    
    GVariant *webextension_data = g_variant_new(
        "(is)", unique_id++, webextension_dir
    );

    // Callback when initialize extensions
    g_signal_connect(webkit_context, "initialize-web-extensions", G_CALLBACK(initialize_web_extensions), webextension_data);

    WebKitWebView *webview = WEBKIT_WEB_VIEW(webkit_web_view_new_with_settings(websettings));

    // Callback when browser instance is closed, the program will also exit
    g_signal_connect(webview, "close", G_CALLBACK(close_webview_callback), main_window);





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

