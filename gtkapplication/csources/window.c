#include "window.h"

#include "widgets.h"
#include "dialogs.h"
#include "headerbar.h"
#include "toolbar.h"
#include "statusbar.h"
#include "sidebar.h"

#include "collection.h"
#include "movietype.h"


#include <stddef.h>
#include <stdbool.h>

// type definition
struct _MovieWindow {
    GtkApplicationWindow parent_instance;

    // main app
    MovieApplication *movieapp;
    MoviesList *movies_list;

    // window state
    int height;
    int width;
    bool is_maximized;
    bool is_fullscreen;
    int paned_position;

    // widgets
    WidgetHeaderbar *headerbar;
    WidgetToolbar *toolbar;
    WidgetStatusbar *statusbar;
    WidgetSidebar *sidebar;

};

G_DEFINE_TYPE(MovieWindow, movie_window, GTK_TYPE_APPLICATION_WINDOW);

// window
static bool signal_delete_event(MovieWindow *window, GdkEvent *event);
static void signal_destroy(MovieWindow *window);
static bool signal_state_event(MovieWindow *window, GdkEventWindowState *event);
static void signal_size_allocate(MovieWindow *window, GdkRectangle *allocation);
static void update_fullscreen(MovieWindow *window, bool is_fullscreen);
static void keyfile_restore_state(MovieWindow *window, GKeyFile *keyfile);
static void keyfile_store_sate(MovieWindow *window, GKeyFile *keyfile);
// toolbar
static void signal_toolbar_open(WidgetToolbar *toolbar, MovieWindow *window);
static void signal_toolbar_save(WidgetToolbar *toolbar, MovieWindow *window);
static void signal_toolbar_new(WidgetToolbar *toolbar, MovieWindow *window);
static void signal_toolbar_source(WidgetToolbar *toolbar, const char *source_name, MovieWindow *window);
// sidebar
static void signal_paned_move(GtkPaned *paned, GParamSpec *pspec, MovieWindow *window);
static void signal_sidebar_search(WidgetSidebar *sidebar, const char *keyword, MovieWindow *window);
static void signal_sidebar_selected(WidgetSidebar *sidebar, GSequenceIter *iter, MovieWindow *window);






static void movie_window_class_init(MovieWindowClass *klass) {
    // GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
}

static void movie_window_init(MovieWindow *window) {
    //
}

MovieWindow *movie_window_new(MovieApplication *application) {
    g_message(__func__);

    g_return_val_if_fail(GTK_IS_APPLICATION(application), NULL);

    return g_object_new(movie_window_get_type(),
        "application", application,
        "show-menubar", false,
        "destroy-with-parent", true,
    NULL);
}


	// object_class->dispose = gedit_window_dispose;
	// object_class->finalize = gedit_window_finalize;
	// object_class->get_property = gedit_window_get_property;
//todo: pass settings from application?









MovieWindow *movie_application_new_window(MovieApplication *app, GdkScreen *screen) {

    // initialize GTK+
    MovieWindow *window = movie_window_new(app);
    window->movieapp = app;//todo remove

    // Create Movies collection
    MoviesList *movies_list = movies_list_new();
    window->movies_list = movies_list;

    // create an 800x600 window
    widget_add_class(GTK_WIDGET(window), "movie_window");
    gtk_window_set_title(GTK_WINDOW(window), "Movie Collection");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window), true);

	if(screen != NULL) {
		gtk_window_set_screen(GTK_WINDOW(window), screen);
	}

    // get states
    GKeyFile *keyfile;
    if((keyfile = movie_application_get_keyfile(app, "states")) != NULL) {
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
    g_signal_connect(window, "window-state-event", G_CALLBACK(signal_state_event), NULL);
    g_signal_connect(window, "size-allocate", G_CALLBACK(signal_size_allocate), NULL);

    g_signal_connect(window, "delete-event", G_CALLBACK(signal_delete_event), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(signal_destroy), NULL);


    ////////// WINDOW DESIGN //////////

    // hide window decorations of main app and use our own
    WidgetHeaderbar *headerbar = movie_application_new_headerbar();
    gtk_window_set_titlebar(GTK_WINDOW(window), GTK_WIDGET(headerbar));

    window->headerbar = headerbar;

    // window inner content
    GtkWidget *mainbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // toolbar with main buttons optons
    WidgetToolbar *toolbar = movie_application_new_toolbar();

    g_signal_connect(toolbar, "open", G_CALLBACK(signal_toolbar_open), window);
    g_signal_connect(toolbar, "save", G_CALLBACK(signal_toolbar_save), window);
    g_signal_connect(toolbar, "new", G_CALLBACK(signal_toolbar_new), window);
    g_signal_connect(toolbar, "source", G_CALLBACK(signal_toolbar_source), window);

    // status bar on the bottom
    WidgetStatusbar *statusbar = movie_application_new_statusbar();
    window->statusbar = statusbar;


    // panel between sidebar and content
    GtkWidget *panedbox = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_position(GTK_PANED(panedbox), 300);

    g_signal_connect(GTK_PANED(panedbox), "notify::position", G_CALLBACK(signal_paned_move), window);

    if(window->paned_position > 0) {
        gtk_paned_set_position(GTK_PANED(panedbox), window->paned_position);
    }

    // sidebar with categories list and searchbar
    WidgetSidebar *sidebar = movie_application_new_sidebar();
    window->sidebar = sidebar;

    g_signal_connect(sidebar, "search", G_CALLBACK(signal_sidebar_search), window);
    g_signal_connect(sidebar, "selected", G_CALLBACK(signal_sidebar_selected), window);

    // bind collection to model
    widget_sidebar_listbox_bind_model(sidebar, G_LIST_MODEL(movies_list));







/*
    
    struct WidgetPanels *widget_panels = widget_panels_new();

    GtkWidget *panels = widget_panels->panels;




    struct WidgetStatusbar *widget_statusbar = widget_statusbar_new();

    GtkWidget *statusbar = widget_statusbar->statusbar;

    // Add all elements to main
    gtk_box_pack_start(GTK_BOX(main_box), toolbar, false, false, 0);
    gtk_box_pack_start(GTK_BOX(main_box), layout_paned, true, true, 0);
    gtk_box_pack_start(GTK_BOX(main_box), statusbar, false, false, 0);

    // Put the content area into the main window
    gtk_container_add(GTK_CONTAINER(window), main_box);








struct WidgetSidebarItem *xxx = widget_sidebar_item_new("ID_1", "XXX", false);
widget_sidebar_add_item(widget_sidebar, xxx);

xxx = widget_sidebar_item_new("ID_2", "azertyuiopqsdfghjklmwxcvbnazertyuiopqsdfghjklmwxcvbnazertyuiopqsdfghjklmwxcvbn", false);
widget_sidebar_add_item(widget_sidebar, xxx);

xxx = widget_sidebar_item_new("ID_3", "<>ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok", true);
widget_sidebar_add_item(widget_sidebar, xxx);



*/


    // Put the content area into the main window

    gtk_paned_pack1(GTK_PANED(panedbox), GTK_WIDGET(sidebar), true, false); // resize, shrink
    gtk_paned_pack2(GTK_PANED(panedbox), gtk_label_new("test"), true, false);

    gtk_box_pack_start(GTK_BOX(mainbox), GTK_WIDGET(toolbar), false, false, 0);
    gtk_box_pack_start(GTK_BOX(mainbox), panedbox, true, true, 0);
    gtk_box_pack_start(GTK_BOX(mainbox), GTK_WIDGET(statusbar), false, false, 0);

    gtk_container_add(GTK_CONTAINER(window), mainbox);

    // Make sure that when the widnow becomes visible, it will get mouse and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(mainbox));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(GTK_WIDGET(window));

    return window;
}

static bool signal_delete_event(MovieWindow *window, GdkEvent *event) {
    g_message(__func__);

    // save states
    GKeyFile *keyfile = g_key_file_new();
    keyfile_store_sate(window, keyfile);
    if(movie_application_set_keyfile(window->movieapp, "states", keyfile)) {
        g_key_file_free(keyfile);
    }

    return GDK_EVENT_PROPAGATE;
}

static void signal_destroy(MovieWindow *window) {
    GtkApplication *gtkapp = gtk_window_get_application(GTK_WINDOW(window));
    g_application_quit(G_APPLICATION(gtkapp));
}

static bool signal_state_event(MovieWindow *window, GdkEventWindowState *event) {

    window->is_maximized = (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
    window->is_fullscreen = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;

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

static void signal_paned_move(GtkPaned *paned, GParamSpec *pspec, MovieWindow *window) {
    window->paned_position = gtk_paned_get_position(paned);
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


static void update_fullscreen(MovieWindow *window, bool is_fullscreen) {
    // gtk_widget_hide (window->statusbar);
}

static void signal_toolbar_open(WidgetToolbar *toolbar, MovieWindow *window) {
    g_message(__func__);

    const char *filename;
    if(!(filename = dialog_file_chooser(GTK_WINDOW(window), NULL))) {
        return;
    }

    GError *error = NULL;
    FILE *stream = fopen(filename, "rb");

    MoviesList *movies_list = window->movies_list;
    // todo: while reading

    // destroy previous colection if any
    movies_list_remove_all(movies_list);

    if(!(movies_list_stream(movies_list, stream, &error))) {
        g_warning("# %s %s", __func__, error->message);

        dialog_message(GTK_WINDOW(window),
            "Erreur lors de l'ouverture du fichier", error->message
        );
        g_clear_error(&error);

        // destroy previous colection if any
        movies_list_remove_all(movies_list);
    }

    fclose(stream);
    g_free((char*) filename);
}

static void signal_toolbar_save(WidgetToolbar *toolbar, MovieWindow *window) {
    g_message(__func__);

    
}

static void signal_toolbar_new(WidgetToolbar *toolbar, MovieWindow *window) {
    g_message(__func__);

    
}

static void signal_toolbar_source(WidgetToolbar *toolbar, const char *source_name, MovieWindow *window) {
    g_message("%s %s", __func__, source_name);

    
}




///// SIDEBAR

static void signal_sidebar_search(WidgetSidebar *sidebar, const char *keyword, MovieWindow *window) {
    g_message("%s %s", __func__, keyword);


}

static void signal_sidebar_selected(WidgetSidebar *sidebar, GSequenceIter *iter, MovieWindow *window) {
    g_message("%s", __func__);

    Movie *movie = g_sequence_get(iter);

    g_message("%s", movie->title);

}
