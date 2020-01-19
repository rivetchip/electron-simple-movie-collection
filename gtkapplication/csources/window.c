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

enum window_tab {
    WINDOW_TAB_WELCOME = 0,
    WINDOW_TAB_PREVIEW,
    WINDOW_TAB_EDITION
};

struct _MovieWindow {
    GtkApplicationWindow parent_instance;

    // main app
    GKeyFile *settings;
    MoviesList *movies_list;
    enum window_tab current_tab;

    // window state
    int height;
    int width;
    bool is_maximized;
    bool is_fullscreen;
    int paned_position;

    // widgets
    GtkWidget *panedbox;
    WidgetHeaderbar *headerbar;
    WidgetToolbar *toolbar;
    WidgetSidebar *sidebar;
    WidgetStatusbar *statusbar;

    // GtkMenuButton  *gear_button;
};

G_DEFINE_TYPE(MovieWindow, movie_window, GTK_TYPE_APPLICATION_WINDOW);


// window
static bool signal_delete_event(MovieWindow *window, GdkEvent *event);
static bool signal_state_event(MovieWindow *window, GdkEventWindowState *event);
static void signal_size_allocate(MovieWindow *window, GdkRectangle *allocation);
static void update_fullscreen(MovieWindow *window, bool is_fullscreen);
static void window_apply_settings(MovieWindow *window, GKeyFile *settings);
static void settings_restore_states(MovieWindow *window, GKeyFile *settings);
static void settings_store_states(MovieWindow *window, GKeyFile *settings);
// window actions
static void action_close(GSimpleAction *action, GVariant *parameter, gpointer window);
static void action_minimize(GSimpleAction *action, GVariant *parameter, gpointer window);
static void action_maximize(GSimpleAction *action, GVariant *parameter, gpointer window);
static void action_fullscreen(GSimpleAction *action, GVariant *parameter, gpointer window);
// main actions
static void action_open(GSimpleAction *action, GVariant *parameter, gpointer window);
static void action_save(GSimpleAction *action, GVariant *parameter, gpointer window);
static void action_save_as(GSimpleAction *action, GVariant *parameter, gpointer window);
static void action_newitem(GSimpleAction *action, GVariant *parameter, gpointer window);
static void action_source_changed(WidgetToolbar *toolbar, const char *source_name, MovieWindow *window);
// others actions
static void action_preferences(GSimpleAction *action, GVariant *parameter, gpointer window);
static void action_shortcuts(GSimpleAction *action, GVariant *parameter, gpointer window);
static void action_about(GSimpleAction *action, GVariant *parameter, gpointer window);
// sidebar
static void signal_movieslist_changed(GListModel *list, unsigned int position, unsigned int removed, unsigned int added, MovieWindow *window);
static void signal_paned_move(GtkPaned *paned, GParamSpec *pspec, MovieWindow *window);
static void signal_sidebar_search(WidgetSidebar *sidebar, const char *keyword, MovieWindow *window);
static void signal_sidebar_selected(WidgetSidebar *sidebar, GSequenceIter *iter, MovieWindow *window);

// set actions
static GActionEntry win_actions[] = {
    // main actions
    {"open", action_open},
    {"save", action_save},
    {"save-as", action_save_as},
    {"newitem", action_newitem},
    // window
    {"close", action_close},
    {"minimize", action_minimize},
    {"maximize", action_maximize},
    {"fullscreen", action_fullscreen},
    // menu
    {"prefs", action_preferences},
    {"shortcuts", action_shortcuts},
    {"about", action_about},
};

MovieWindow *movie_window_new(GKeyFile *settings) {
    g_message(__func__);

    MovieWindow *window = g_object_new(movie_window_get_type(),
        "show-menubar", false,
    NULL);

    // we must restore the settings after the object has been constructed
    if((window->settings = settings) != NULL) {
        window_apply_settings(window, settings);
    }

    // when the widnow becomes visible, it will get mouse and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(window));
    // the main window and all its contents are visible
    gtk_widget_show_all(GTK_WIDGET(window));

    return window;
}

static void movie_window_class_init(MovieWindowClass *klass) {
    g_message(__func__);

    // GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
}

static void movie_window_init(MovieWindow *window) {
    g_message(__func__);
    // initialize GTK+

    // create Movies collection list
    MoviesList *movies_list = movies_list_new();
    window->movies_list = movies_list;

    g_signal_connect(G_LIST_MODEL(movies_list), "items-changed", G_CALLBACK(signal_movieslist_changed), window);

    // create an 800x600 window
    widget_add_class(GTK_WIDGET(window), "movie_window");
    gtk_window_set_title(GTK_WINDOW(window), "Movie Collection");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window), true);

    // window events
    g_signal_connect(window, "window-state-event", G_CALLBACK(signal_state_event), NULL);
    g_signal_connect(window, "size-allocate", G_CALLBACK(signal_size_allocate), NULL);
    g_signal_connect(window, "delete-event", G_CALLBACK(signal_delete_event), NULL);

    // set actions
    g_action_map_add_action_entries(G_ACTION_MAP(window), win_actions, G_N_ELEMENTS(win_actions), window);


    ////////// WINDOW DESIGN //////////

    // hide window decorations of main app and use our own
    WidgetHeaderbar *headerbar = widget_headerbar_new();
    gtk_window_set_titlebar(GTK_WINDOW(window), GTK_WIDGET(headerbar));
    // todo signals here!

    window->headerbar = headerbar;

    // window inner content
    GtkWidget *mainbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // toolbar with main buttons optons
    WidgetToolbar *toolbar = widget_toolbar_new();
    g_signal_connect(toolbar, "source", G_CALLBACK(action_source_changed), window);

    // status bar on the bottom
    WidgetStatusbar *statusbar = widget_statusbar_new();
    window->statusbar = statusbar;


    // panel between sidebar and content
    GtkWidget *panedbox = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    window->panedbox = panedbox;

    gtk_paned_set_position(GTK_PANED(panedbox), 300);
    g_signal_connect(GTK_PANED(panedbox), "notify::position", G_CALLBACK(signal_paned_move), window);


    // sidebar with categories list and searchbar
    WidgetSidebar *sidebar = widget_sidebar_new();
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
}

static void window_apply_settings(MovieWindow *window, GKeyFile *settings) {

    // restore previous state
    settings_restore_states(window, settings);

    if(window->width > 0 && window->height > 0) {
        gtk_window_set_default_size(GTK_WINDOW(window), window->width, window->height);
    }
    if(window->is_maximized) {
        gtk_window_maximize(GTK_WINDOW(window));
    }
    if(window->is_fullscreen) {
        gtk_window_fullscreen(GTK_WINDOW(window));
    }

    if(window->paned_position > 0) {
        gtk_paned_set_position(GTK_PANED(window->panedbox), window->paned_position);
    }
}




static bool signal_delete_event(MovieWindow *window, GdkEvent *event) {
    g_message(__func__);

    // save states
    GKeyFile *settings;
    if((settings = window->settings) != NULL) {
        settings_store_states(window, settings);
    }

    return GDK_EVENT_PROPAGATE;
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

static void update_fullscreen(MovieWindow *window, bool is_fullscreen) {
    g_message("%s/%d", __func__, is_fullscreen);

    // gtk_widget_hide(window->statusbar);
}

static void signal_paned_move(GtkPaned *paned, GParamSpec *pspec, MovieWindow *window) {
    window->paned_position = gtk_paned_get_position(paned);
}

static void settings_restore_states(MovieWindow *window, GKeyFile *settings) {
    int state;

    if((state = g_key_file_get_integer(settings, "WindowState", "height", NULL))) {
        window->height = state;
    }
    if((state = g_key_file_get_integer(settings, "WindowState", "width", NULL))) {
        window->width = state;
    }
    if((state = g_key_file_get_integer(settings, "WindowState", "maximized", NULL))) {
        window->is_maximized = state;
    }
    if((state = g_key_file_get_integer(settings, "WindowState", "fullscreen", NULL))) {
        window->is_fullscreen = state;
    }
    if((state = g_key_file_get_integer(settings, "WindowState", "paned_position", NULL))) {
        window->paned_position = state;
    }
}

static void settings_store_states(MovieWindow *window, GKeyFile *settings) {
    int state;

    if((state = window->height)) {
        g_key_file_set_integer(settings, "WindowState", "height", state);
    }
    if((state = window->width)) {
        g_key_file_set_integer(settings, "WindowState", "width", state);
    }
    g_key_file_set_integer(settings, "WindowState", "maximized", window->is_maximized);
    g_key_file_set_integer(settings, "WindowState", "fullscreen", window->is_fullscreen);

    if((state = window->paned_position)) {
        g_key_file_set_integer(settings, "WindowState", "paned_position", state);
    }
}


// window actions

static void action_close(GSimpleAction *action, GVariant *parameter, gpointer window) {
    g_message(__func__);
    gtk_window_close(GTK_WINDOW(window));
}

static void action_minimize(GSimpleAction *action, GVariant *parameter, gpointer window) {
    g_message(__func__);
    gtk_window_iconify(GTK_WINDOW(window));
    
}

static void action_maximize(GSimpleAction *action, GVariant *parameter, gpointer window) {
    g_message(__func__);
    gtk_window_is_maximized(GTK_WINDOW(window)) ? gtk_window_unmaximize(GTK_WINDOW(window)) : gtk_window_maximize(GTK_WINDOW(window));
}

static void action_fullscreen(GSimpleAction *action, GVariant *parameter, gpointer window) {
    g_message(__func__);

    MovieWindow *mwindow = MOVIE_WINDOW(window);
    mwindow->is_fullscreen ? gtk_window_unfullscreen(GTK_WINDOW(window)) : gtk_window_fullscreen(GTK_WINDOW(window));
}

// toolbar actions

static void action_open(GSimpleAction *action, GVariant *parameter, gpointer window) {
    g_message(__func__);
    
}

static void action_save(GSimpleAction *action, GVariant *parameter, gpointer window) {
    g_message(__func__);
    
}

static void action_save_as(GSimpleAction *action, GVariant *parameter, gpointer window) {
    g_message(__func__);
    
}

static void action_newitem(GSimpleAction *action, GVariant *parameter, gpointer window) {
    g_message(__func__);
    
}

static void action_source_changed(WidgetToolbar *toolbar, const char *source_name, MovieWindow *window) {
    g_message(__func__);
    
    g_message("%s %s", __func__, source_name);
}

// others actions

static void action_preferences(GSimpleAction *action, GVariant *parameter, gpointer window) {
    g_message(__func__);
}

static void action_shortcuts(GSimpleAction *action, GVariant *parameter, gpointer window) {
    g_message(__func__);
}

static void action_about(GSimpleAction *action, GVariant *parameter, gpointer window) {
    g_message(__func__);
}












/*

static void signal_toolbar_open(WidgetToolbar *toolbar, MovieWindow *window) {
    g_message(__func__);

    const char *filename;
    if((filename = dialog_file_chooser(GTK_WINDOW(window), NULL)) == NULL) {
        return;
    }

    GError *error = NULL;
    FILE *stream = fopen(filename, "rb");

    MoviesList *movies_list = window->movies_list;

    // destroy previous colection if any
    movies_list_remove_all(movies_list);

    // todo: while reading : optimisation, widget_set_busy ?
    if(!(movies_list_stream(movies_list, stream, &error))) {

        dialog_message(GTK_WINDOW(window),
            "Erreur lors de l'ouverture du fichier",
            (error != NULL ? error->message : NULL)
        );
        g_clear_error(&error);

        // destroy previous colection if any
        movies_list_remove_all(movies_list);
    }

    fclose(stream);
    g_free((char*) filename);
}

*/



///// SIDEBAR

static void signal_movieslist_changed(GListModel *list, unsigned int position, unsigned int removed, unsigned int added, MovieWindow *window) {
    g_message("%s pos:%u del:%u add:%u", __func__, position, removed, added);

    unsigned int n_items = g_list_model_get_n_items(list); //todo
    widget_statusbar_set_text(window->statusbar, g_strdup_printf("%d films", n_items));
}

static void signal_sidebar_search(WidgetSidebar *sidebar, const char *keyword, MovieWindow *window) {
    g_message("%s %s", __func__, keyword);

    bool success;
    MoviesList *movies_list = window->movies_list;
    if(!(success = movies_list_search_keyword(movies_list, keyword))) {
        g_warning("Cannot search through movies list sequences");
    }
}

static void signal_sidebar_selected(WidgetSidebar *sidebar, GSequenceIter *iter, MovieWindow *window) {
    g_message("%s", __func__);

    g_return_if_fail(iter != NULL);


    Movie *movie = g_sequence_get(iter);

    g_message("%s", movie->title);




}
