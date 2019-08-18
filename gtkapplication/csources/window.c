
#include <config.h>
#include "window.h"

G_DEFINE_TYPE(MovieWindow, movie_window, GTK_TYPE_APPLICATION_WINDOW);

static void signal_destroy(MovieWindow *window, MovieApplication *app);
static bool signal_state_event(MovieWindow *window, GdkEventWindowState *event);
static void signal_size_allocate(MovieWindow *window, GdkRectangle *allocation);
static void update_fullscreen(MovieWindow *window, bool is_fullscreen);
static void load_state(MovieApplication *app, MovieWindow *window);



static void movie_window_init(MovieWindow *window) {
    g_message(__func__);
}

static void movie_window_class_init(MovieWindowClass *klass) {
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);



  // virtual function overrides go here
  // property and signal definitions go here
}

MovieWindow *movie_window_new(MovieApplication *application) {

    g_return_val_if_fail(GTK_IS_APPLICATION(application), NULL);

    return g_object_new(movie_window_get_type(),
        "application", application,
        "show-menubar", FALSE,
    NULL);
}


MovieWindow *movie_appplication_create_window(MovieApplication *app, GdkScreen *screen) {
    const char *appid = g_application_get_application_id(G_APPLICATION(app));

    // initialize GTK+
    MovieWindow *window = movie_window_new(app);
    // widget_add_class(GTK_WIDGET(window), "movie_window");

    // create an 800x600 window
    gtk_window_set_icon_name(GTK_WINDOW(window), appid);
    gtk_window_set_title(GTK_WINDOW(window), "Movie Collection");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    // set window settings
    GtkSettings *window_settings = gtk_settings_get_default();
    g_object_set(G_OBJECT(window_settings),
        "gtk-application-prefer-dark-theme", TRUE,
        // "gtk-font-name", "Lato 12",
    NULL);

/*
    g_signal_connect(GTK_WIDGET(window), "destroy", G_CALLBACK(signal_destroy), NULL);
    g_signal_connect(GTK_WINDOW(window), "window-state-event", G_CALLBACK(signal_state_event), NULL);
    g_signal_connect(GTK_WINDOW(window), "size-allocate", G_CALLBACK(signal_size_allocate), NULL);

    // restore previous state
    if(window->win_height > 0 && window->win_width > 0) {
        gtk_window_set_default_size(GTK_WINDOW(window),
            app->win_width,
            app->win_height
        );
    }
    if(window->is_maximized) {
        gtk_window_maximize(GTK_WINDOW(window));
    }
    if(window->is_fullscreen) {
        gtk_window_fullscreen(GTK_WINDOW(window));
    }

    ////////// WINDOW DESIGN //////////

    // hide window decorations of main app and use our own
    struct WidgetHeaderbar *widget_headerbar = widget_headerbar_new();

    GtkWidget *headerbar = widget_headerbar->headerbar;
    gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);

    // Window Main
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Toolbar
    struct WidgetToolbar *widget_toolbar = widget_toolbar_new();

    GtkWidget *toolbar = widget_toolbar->toolbar;

    g_signal_connect(widget_toolbar->button_open, "clicked",
        G_CALLBACK(signal_toolbar_open), app
    );

    g_signal_connect(widget_toolbar->button_save, "clicked",
        G_CALLBACK(signal_toolbar_save), app
    );

    g_signal_connect(widget_toolbar->button_new, "clicked",
        G_CALLBACK(signal_toolbar_new), app
    );

    // Panel between sidebar and content
    GtkWidget *layout_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_position(GTK_PANED(layout_paned), 300);

    g_signal_connect(GTK_PANED(layout_paned), "notify::position",
        G_CALLBACK(signal_mainwindow_paned_move), app
    );

    if(app->paned_position > 0) {
        gtk_paned_set_position(GTK_PANED(layout_paned), app->paned_position);
    }

    // Sidebar
    struct WidgetSidebar *widget_sidebar = widget_sidebar_new();

    GtkWidget *sidebar = widget_sidebar->sidebar;

    g_signal_connect(widget_sidebar->search_entry, "key-release-event",
        G_CALLBACK(signal_searchentry_keyrelease), app
    );

    g_signal_connect(widget_sidebar->search_entry, "changed",
        G_CALLBACK(signal_searchentry_changed), app
    );

    g_signal_connect(widget_sidebar->list_items, "row-selected", // categories
        G_CALLBACK(signal_sidebar_list_items_selected), app
    );

    struct WidgetPanels *widget_panels = widget_panels_new();

    GtkWidget *panels = widget_panels->panels;

    gtk_paned_pack1(GTK_PANED(layout_paned), sidebar, TRUE, FALSE); // resize, shrink
    gtk_paned_pack2(GTK_PANED(layout_paned), panels, TRUE, FALSE);


    struct WidgetStatusbar *widget_statusbar = widget_statusbar_new();

    GtkWidget *statusbar = widget_statusbar->statusbar;

    // Add all elements to main
    gtk_box_pack_start(GTK_BOX(main_box), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), layout_paned, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), statusbar, FALSE, FALSE, 0);

    // Put the content area into the main window
    gtk_container_add(GTK_CONTAINER(window), main_box);

    // Make sure that when the browser area becomes visible, it will get mouse and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(main_box));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(window);






struct WidgetSidebarItem *xxx = widget_sidebar_item_new("ID_1", "XXX", FALSE);
widget_sidebar_add_item(widget_sidebar, xxx);

xxx = widget_sidebar_item_new("ID_2", "azertyuiopqsdfghjklmwxcvbnazertyuiopqsdfghjklmwxcvbnazertyuiopqsdfghjklmwxcvbn", FALSE);
widget_sidebar_add_item(widget_sidebar, xxx);

xxx = widget_sidebar_item_new("ID_3", "<>ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok", TRUE);
widget_sidebar_add_item(widget_sidebar, xxx);



*/


}

static void signal_destroy(MovieWindow *window, MovieApplication *app) {
    GtkApplication *gtkapp = gtk_window_get_application(GTK_WINDOW(window));
    g_application_quit(G_APPLICATION(gtkapp));
}

static bool signal_state_event(MovieWindow *window, GdkEventWindowState *event) {
    GdkWindowState window_state = event->new_window_state; // event->type=GDK_WINDOW_STATE

    window->is_maximized = (window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
    window->is_fullscreen = (window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;

	if((event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN) != 0) {
		update_fullscreen(window, window->is_fullscreen);
	}

    return GDK_EVENT_PROPAGATE;
}

static void signal_size_allocate(MovieWindow *window, GdkRectangle *allocation) {
    // save the window geometry only if we are not maximized of fullscreen
    if(!(window->is_maximized || window->is_fullscreen)) {
        // using the allocation directly can lead to growing windows with client-side decorations
        gtk_window_get_size(GTK_WINDOW(window),
            &window->width,
            &window->height
        );
    }
}


static void update_fullscreen(MovieWindow *window, bool is_fullscreen) {
    // gtk_widget_hide (window->statusbar);
}


static void load_state(MovieApplication *app, MovieWindow *window) {
    const char *appid = g_application_get_application_id(G_APPLICATION(app));

    char *state_file = g_build_filename(g_get_user_cache_dir(), appid, "state.ini", NULL);

    GKeyFile *keyfile = g_key_file_new();
/*
    if(g_key_file_load_from_file(keyfile, state_file, G_KEY_FILE_NONE, NULL)) {

        GError *error_read = NULL;

        int state_height = g_key_file_get_integer(keyfile, "WindowState", "height", &error_read);
        error_read == NULL ? (app->height = state_height) : g_clear_error(&error_read);

        int state_width = g_key_file_get_integer(keyfile, "WindowState", "width", &error_read);
        error_read == NULL ? (app->width = state_width) : g_clear_error(&error_read);

        bool state_maximized = g_key_file_get_boolean(keyfile, "WindowState", "maximized", &error_read);
        error_read == NULL ? (app->is_maximized = state_maximized) : g_clear_error(&error_read);

        int state_fullscreen = g_key_file_get_boolean(keyfile, "WindowState", "fullscreen", &error_read);
        error_read == NULL ? (app->is_fullscreen = state_fullscreen) : g_clear_error(&error_read);

        int paned_position = g_key_file_get_integer(keyfile, "WindowState", "paned_position", &error_read);
        error_read == NULL ? (app->paned_position = paned_position) : g_clear_error(&error_read);
    }
*/
    g_key_file_free(keyfile);
    g_free(state_file);
}


