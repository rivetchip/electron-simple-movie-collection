#include "window.h"
#include "headerbar.h"
#include <stddef.h>

// type definition
struct _MovieWindow {
    GtkApplicationWindow parent_instance;

    // main app
    MovieApplication *movieapp;

    // window state
    int height;
    int width;
    bool is_maximized;
    bool is_fullscreen;
    int paned_position;

    // widgets
    WidgetHeaderbar *headerbar;
};

G_DEFINE_TYPE(MovieWindow, movie_window, GTK_TYPE_APPLICATION_WINDOW);

static bool signal_delete_event(MovieWindow *window, GdkEvent *event);
static void signal_destroy(MovieWindow *window);
static bool signal_state_event(MovieWindow *window, GdkEventWindowState *event);
static void signal_size_allocate(MovieWindow *window, GdkRectangle *allocation);
static void update_fullscreen(MovieWindow *window, bool is_fullscreen);
static void keyfile_restore_state(MovieWindow *window, GKeyFile *keyfile);
static void keyfile_store_sate(MovieWindow *window, GKeyFile *keyfile);


static void movie_window_init(MovieWindow *window) {
    g_message(__func__);
}

static void movie_window_class_init(MovieWindowClass *klass) {
	// GObjectClass *object_class = G_OBJECT_CLASS(klass);
	// GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
}

MovieWindow *movie_window_new(MovieApplication *application) {

    g_return_val_if_fail(GTK_IS_APPLICATION(application), NULL);

    return g_object_new(movie_window_get_type(),
        "application", application,
        "show-menubar", FALSE,
        "destroy-with-parent", TRUE,
    NULL);
}


MovieWindow *movie_appplication_new_window(MovieApplication *app, GdkScreen *screen) {
    const char *appid = g_application_get_application_id(G_APPLICATION(app));
    
    // initialize GTK+
    MovieWindow *window = movie_window_new(app);
    window->movieapp = app;

    // create an 800x600 window
    widget_add_class(GTK_WIDGET(window), "movie_window");
    gtk_window_set_icon_name(GTK_WINDOW(window), appid);
    gtk_window_set_title(GTK_WINDOW(window), "Movie Collection");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

	if(screen != NULL) {
		gtk_window_set_screen(GTK_WINDOW(window), screen);
	}

    // set window settings
    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(G_OBJECT(settings),
        "gtk-application-prefer-dark-theme", TRUE,
        // "gtk-font-name", "Lato 12",
    NULL);

    // get states
    GKeyFile *keyfile;
    if((keyfile = movie_application_get_keyfile_states(app)) != NULL) {
        keyfile_restore_state(window, keyfile);
        g_key_file_free(keyfile);
    }

    // restore previous state
    if(window->width > 0 && window->height > 0) {
        gtk_window_set_default_size(GTK_WINDOW(window), window->width, window->height);
    }
    if(window->is_maximized) {
        gtk_window_maximize(GTK_WINDOW(window));
    }
    if(window->is_fullscreen) {
        gtk_window_fullscreen(GTK_WINDOW(window));
    }

    // window events
    g_signal_connect(GTK_WINDOW(window), "window-state-event", G_CALLBACK(signal_state_event), NULL);
    g_signal_connect(GTK_WINDOW(window), "size-allocate", G_CALLBACK(signal_size_allocate), NULL);

    g_signal_connect(GTK_WINDOW(window), "delete-event", G_CALLBACK(signal_delete_event), NULL);
    g_signal_connect(GTK_WIDGET(window), "destroy", G_CALLBACK(signal_destroy), NULL);


    ////////// WINDOW DESIGN //////////

    WidgetHeaderbar *headerbar = movie_appplication_new_headerbar(window);
    window->headerbar = headerbar;

    // hide window decorations of main app and use our own
    gtk_window_set_titlebar(GTK_WINDOW(window), GTK_WIDGET(headerbar));


    // Window Main
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

/*
    
    


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








struct WidgetSidebarItem *xxx = widget_sidebar_item_new("ID_1", "XXX", FALSE);
widget_sidebar_add_item(widget_sidebar, xxx);

xxx = widget_sidebar_item_new("ID_2", "azertyuiopqsdfghjklmwxcvbnazertyuiopqsdfghjklmwxcvbnazertyuiopqsdfghjklmwxcvbn", FALSE);
widget_sidebar_add_item(widget_sidebar, xxx);

xxx = widget_sidebar_item_new("ID_3", "<>ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok", TRUE);
widget_sidebar_add_item(widget_sidebar, xxx);



*/


    // Make sure that when the browser area becomes visible, it will get mouse and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(main_box));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(GTK_WIDGET(window));




    return window;
}

static bool signal_delete_event(MovieWindow *window, GdkEvent *event) {
    g_message(__func__);

    // save states
    GKeyFile *keyfile = movie_application_new_keyfile(window->movieapp);
    keyfile_store_sate(window, keyfile);
    if(movie_application_set_keyfile_states(window->movieapp, keyfile)) {
        g_key_file_free(keyfile);
    }

    return GDK_EVENT_PROPAGATE;
}

static void signal_destroy(MovieWindow *window) {
    g_message(__func__);
    movie_application_quit(window->movieapp);
}

static bool signal_state_event(MovieWindow *window, GdkEventWindowState *event) {
    GdkWindowState state = event->new_window_state; // event->type=GDK_WINDOW_STATE

    window->is_maximized = (state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
    window->is_fullscreen = (state & GDK_WINDOW_STATE_FULLSCREEN) != 0;

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

static void keyfile_restore_state(MovieWindow *window, GKeyFile *keyfile) {
    int state;

    if((state = g_key_file_get_integer(keyfile, "WindowState", "height", NULL))) {
        window->height = state;
    }
    if((state = g_key_file_get_integer(keyfile, "WindowState", "width", NULL))) {
        window->width = state;
    }
    if((state = g_key_file_get_integer(keyfile, "WindowState", "maximized", NULL))) {
        window->is_maximized = state;
    }
    if((state = g_key_file_get_integer(keyfile, "WindowState", "fullscreen", NULL))) {
        window->is_fullscreen = state;
    }
    if((state = g_key_file_get_integer(keyfile, "WindowState", "paned_position", NULL))) {
        window->paned_position = state;
    }
}

static void keyfile_store_sate(MovieWindow *window, GKeyFile *keyfile) {
    int state;

    if((state = window->height)) {
        g_key_file_set_integer(keyfile, "WindowState", "height", state);
    }
    if((state = window->width)) {
        g_key_file_set_integer(keyfile, "WindowState", "width", state);
    }
    if((state = window->is_maximized)) {
        g_key_file_set_integer(keyfile, "WindowState", "maximized", state);
    }
    if((state = window->is_fullscreen)) {
        g_key_file_set_integer(keyfile, "WindowState", "fullscreen", state);
    }
    if((state = window->paned_position)) {
        g_key_file_set_integer(keyfile, "WindowState", "paned_position", state);
    }
}


