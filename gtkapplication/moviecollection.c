/*
sudo dnf install gtk3-devel gstreamer-devel clutter-devel webkitgtk3-devel libgda-devel gobject-introspection-devel
webkit2gtk3-devel
builddir > clear && ninja && GTK_DEBUG=interactive ./moviecollection --debug
gcc moviecollection.c -o main `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` && ./main
/usr/local
meson --buildtype
coredumpctl list => gdb / coredumpctl gdb
todo: check for :
Gdk-Message: 21:10:47.562: Error 71 (Protocol error) dispatching to Wayland display.
*/
// #define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

// __HAVE_DISTINCT_FLOAT128

#include <config.h> //build generated
#include "moviecollection.h"
#include "macros.h"
#include <time.h>



// todo: set custom windows with private properties
static char *storageFilename;
static char *storageFolder;
static char *storagePosters;
static struct MovieCollection *storageMovieCollection;
//default_location

G_DEFINE_TYPE(MovieApplication, movie_application, GTK_TYPE_APPLICATION);

static void movie_application_init(MovieApplication *app) {
    g_message(__func__);
}

static void movie_application_class_init(MovieApplicationClass *class) {
    // GApplicationClass *app_class = G_APPLICATION_CLASS(class);

    // app_class->startup = demo_application_startup;
    // app_class->activate = demo_application_activate;
}

static MovieApplication *movie_application_new(const char *application_id, GApplicationFlags flags) {

    g_return_val_if_fail(g_application_id_is_valid(application_id), NULL); // normal comportment
    
    return g_object_new(movie_application_get_type(),
        "application-id", application_id,
        "flags", flags,
    NULL);
}



static bool movie_collection_get(char *movieId, struct MovieCollectionItem *item) {

}

static bool movie_collection_add(char *movieId, struct MovieCollectionItem *item) {

}

static bool movie_collection_remove(char *movieId) {
    
}

static char *movie_collection_stringify() {

}






/**
 * Read up to (and including) a newline from *stream into *lineptr (and null-terminate it).
 * lineptr is a pointer returned from malloc (or NULL), pointing to *n characters of space.
 */
size_t getline(FILE *stream, char **lineptr, size_t *n) {
    // const char* endl[4] = {"\n", "\r", "\r\n", "\n"};

    if(*lineptr == NULL) {
        *lineptr = malloc(128); // default size
        if(*lineptr == NULL) {
            return -1;
        }

        *n = 128;
    }

    int c;
    size_t pos = 0;

    while((c = fgetc(stream)) != EOF) {

        if(pos + 1 >= *n) {
            size_t new_size = *n + (*n >> 2);

            if(new_size < 128) {
                new_size = 128;
            }

            char *new_ptr = realloc(*lineptr, new_size);
            if(new_ptr == NULL) {
                return -1;
            }

            *n = new_size;
            *lineptr = new_ptr;
        }

        ((unsigned char *)(*lineptr))[pos++] = c;
    
        if(c == '\n') {
            break;
        }
    }

    (*lineptr)[pos] = '\0';
    return pos;
}

static void widget_add_class(GtkWidget *widget, char *class_name) {
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), class_name);
}






static bool movie_collection_open(const char *filename, GError **error) {

    FILE *stream = fopen(filename, "rb");
    size_t line_size = 0;
    char *line = NULL;
    size_t read = 0;

    JsonParser *parser = json_parser_new();
    JsonObject *rootnode;

    unsigned int i = 0;
    while ((read = getline(stream, &line, &line_size)) > 0) {

        if(!json_parser_load_from_data(parser, line, strlen(line), error)) {
            free(line);
            fclose(stream);
            g_object_unref(parser);
            return FALSE;
        }

        rootnode = json_node_get_object(json_parser_get_root(parser));

        if(i > 0) { // movies
            
            const char *movieId = json_object_get_string_member(rootnode, "movieId");

            g_message("%s", movieId);
//todo

        } else { // first line : metadata


        }

        i++;
    }

    free(line);
    fclose(stream);
    g_object_unref(parser);

    return TRUE;
}

static bool movie_collection_save(const char *filename, GError **error) {

}












static void mainwindow_store_state(MovieApplication *mapp) {
    const char *appid = g_application_get_application_id(G_APPLICATION(mapp));

    GKeyFile *keyfile = g_key_file_new();

    g_key_file_set_integer(keyfile, "WindowState", "height", mapp->win_height);
    g_key_file_set_integer(keyfile, "WindowState", "width", mapp->win_width);
    g_key_file_set_boolean(keyfile, "WindowState", "maximized", mapp->is_maximized);
    g_key_file_set_boolean(keyfile, "WindowState", "fullscreen", mapp->is_fullscreen);
    g_key_file_set_integer(keyfile, "WindowState", "paned_position", mapp->paned_position);

    // save file under $XDG_CACHE_HOME
    char *state_path = g_build_filename(g_get_user_cache_dir(), appid, NULL);
    char *state_file = g_build_filename(state_path, "state.ini", NULL);

    // create save path if not set
    if(g_mkdir_with_parents(state_path, 0755) == 0) { // error=-1 exist=0

        GError *error_save = NULL;
        if(!g_key_file_save_to_file(keyfile, state_file, &error_save)) {
            g_warning("%s %s", __func__, error_save->message);
            g_clear_error(&error_save);
        }
    }

    g_key_file_free(keyfile);
    g_free(state_path);
    g_free(state_file);
}

static void mainwindow_load_state(MovieApplication *mapp) {
    const char *appid = g_application_get_application_id(G_APPLICATION(mapp));

    char *state_file = g_build_filename(g_get_user_cache_dir(), appid, "state.ini", NULL);

    GKeyFile *keyfile = g_key_file_new();

    if(g_key_file_load_from_file(keyfile, state_file, G_KEY_FILE_NONE, NULL)) {

        GError *error_read = NULL;

        int state_height = g_key_file_get_integer(keyfile, "WindowState", "height", &error_read);
        error_read == NULL ? (mapp->win_height = state_height) : g_clear_error(&error_read);

        int state_width = g_key_file_get_integer(keyfile, "WindowState", "width", &error_read);
        error_read == NULL ? (mapp->win_width = state_width) : g_clear_error(&error_read);

        bool state_maximized = g_key_file_get_boolean(keyfile, "WindowState", "maximized", &error_read);
        error_read == NULL ? (mapp->is_maximized = state_maximized) : g_clear_error(&error_read);

        int state_fullscreen = g_key_file_get_boolean(keyfile, "WindowState", "fullscreen", &error_read);
        error_read == NULL ? (mapp->is_fullscreen = state_fullscreen) : g_clear_error(&error_read);

        int paned_position = g_key_file_get_integer(keyfile, "WindowState", "paned_position", &error_read);
        error_read == NULL ? (mapp->paned_position = paned_position) : g_clear_error(&error_read);
    }

    g_key_file_free(keyfile);
    g_free(state_file);
}

static bool signal_mainwindow_state_event(GtkWidget *window, GdkEventWindowState *event, MovieApplication *mapp) {
    GdkWindowState window_state = event->new_window_state; // event->type=GDK_WINDOW_STATE

    mapp->is_maximized = (window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;

    mapp->is_fullscreen = (window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;

    return GDK_EVENT_PROPAGATE;
}

static void signal_mainwindow_size_allocate(GtkWidget *window, GdkRectangle *allocation, MovieApplication *mapp) {
    // save the window geometry only if we are not maximized of fullscreen
    if(!(mapp->is_maximized || mapp->is_fullscreen)) {
        // use gtk_ ; Using the allocation directly can lead to growing windows with client-side decorations
        gtk_window_get_size(GTK_WINDOW(window),
            &mapp->win_width,
            &mapp->win_height
        );
    }
}

static void signal_mainwindow_paned_move(GtkPaned *paned, GParamSpec *pspec, MovieApplication *mapp) {
    mapp->paned_position = gtk_paned_get_position(paned);
}


static void signal_mainwindow_destroy(GtkWidget *window, MovieApplication *mapp) {
    GtkApplication *gtkapp = gtk_window_get_application(GTK_WINDOW(window));
    g_application_quit(G_APPLICATION(gtkapp));
}

static void signal_css_provider_parsing_error(GtkCssProvider *provider, GtkCssSection *section, GError *error) {

    g_warning("Theme parsing error: %u:%u %s",
        gtk_css_section_get_end_line (section) + 1,
        gtk_css_section_get_end_position (section),
        error->message
    );
}

static void signal_headerbar_close(GtkButton *button) {
    GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if(GTK_IS_WINDOW(toplevel)) {
        GtkApplication *gtk_app = gtk_window_get_application(GTK_WINDOW(toplevel));
        g_application_quit(G_APPLICATION(gtk_app));
    }
}

static void signal_headerbar_minimize(GtkButton *button) {
    GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if(GTK_IS_WINDOW(toplevel)) {
        gtk_window_iconify(GTK_WINDOW(toplevel));
    }
}

static void signal_headerbar_maximize(GtkButton *button) {
    GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(button));

    if(GTK_IS_WINDOW(toplevel)) {
        GtkWindow *gtk_window = GTK_WINDOW(toplevel);
        gtk_window_is_maximized(gtk_window) ? gtk_window_unmaximize(gtk_window) : gtk_window_maximize(gtk_window);
    }
}

static struct WidgetHeaderbar *widget_headerbar_new() {

    struct WidgetHeaderbar *widget = malloc(sizeof(struct WidgetHeaderbar));

    // Set GTK CSD HeaderBar
    GtkWidget *headerbar = gtk_header_bar_new();
    widget_add_class(headerbar, "headerbar");

    widget->headerbar = headerbar;

    // hide window decorationq of header bar
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(headerbar), FALSE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(headerbar), "Movie Collection");
    gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(headerbar), FALSE);
    gtk_widget_set_size_request(headerbar, -1, 45); // width height

    // add buttons and callback on click (override gtk-decoration-layout property)

    GtkWidget *button_close = gtk_button_new_from_icon_name("@window-close", GTK_ICON_SIZE_BUTTON);
    widget_add_class(button_close, "headerbutton");
    g_signal_connect(button_close, "clicked", G_CALLBACK(signal_headerbar_close), NULL);

    widget->button_close = button_close;

    GtkWidget *button_minimize = gtk_button_new_from_icon_name("@window-minimize", GTK_ICON_SIZE_BUTTON);
    widget_add_class(button_minimize, "headerbutton");
    g_signal_connect(button_minimize, "clicked", G_CALLBACK(signal_headerbar_minimize), NULL);

    widget->button_minimize = button_minimize;

    GtkWidget *button_maximize = gtk_button_new_from_icon_name("@window-maximize", GTK_ICON_SIZE_BUTTON);
    widget_add_class(button_maximize, "headerbutton");
    g_signal_connect(button_maximize, "clicked", G_CALLBACK(signal_headerbar_maximize), NULL);

    widget->button_maximize = button_maximize;

    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), button_close);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), button_minimize);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), button_maximize);

    // options buttons
    GtkWidget *button_menu = gtk_menu_button_new();
    widget_add_class(button_menu, "headerbutton");

    GtkWidget *popover = gtk_popover_new(button_menu);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(button_menu), popover);



    // gtk_container_add(GTK_CONTAINER(popover), xxx);


    // gtk_widget_show_all(xxx);

    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerbar), button_menu);


    return widget;
}

static void signal_app_startup(MovieApplication *mapp) {

    // set application main config variables

    // load previous window state, if any
    mainwindow_load_state(mapp);
}


static void signal_searchentry_keyrelease(GtkEntry *entry, GdkEventKey *event, MovieApplication *mapp) {
    if(event->keyval == GDK_KEY_Escape) {
        gtk_entry_set_text(entry, ""); // empty
    }
}

static void signal_searchentry_changed(GtkEntry *entry, MovieApplication *mapp) {
    const char *keyword = gtk_entry_get_text(entry);

    #if PACKAGE_DEVELOPER_MODE
        g_message("%s %s", __func__, keyword);
    #endif

}






static void signal_sidebar_list_items_selected(GtkListBox *listbox, GtkListBoxRow *listrow, MovieApplication *mapp) {

    const char *item_id = g_object_get_data(
        G_OBJECT(GTK_WIDGET(listrow)), "itemId"
    );

    #if PACKAGE_DEVELOPER_MODE
        g_message("%s %s", __func__, item_id);
    #endif


}

static void signal_toolbar_provider_change(GtkToggleButton *togglebutton, char *provider_name) {
    bool is_active = gtk_toggle_button_get_active(togglebutton);
    
    #if PACKAGE_DEVELOPER_MODE
        g_message("%s %s %s", __func__, provider_name, (is_active?"on":"off"));
    #endif



}

static struct WidgetToolbar *widget_toolbar_new() {

    struct WidgetToolbar *widget = malloc(sizeof(struct WidgetToolbar));

    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    widget_add_class(toolbar, "toolbar");
    gtk_widget_set_size_request(toolbar, -1, 45); // width height

    widget->toolbar = toolbar;

    // main buttons

    GtkWidget *button_open = gtk_button_new_from_icon_name(
        "@toolbar-open", GTK_ICON_SIZE_LARGE_TOOLBAR
    );
    gtk_button_set_label(GTK_BUTTON(button_open), "Ouvrir");
    widget_add_class(button_open, "toolbar-button");
    gtk_button_set_always_show_image(GTK_BUTTON(button_open), TRUE);

    widget->button_open = button_open;

    GtkWidget *button_save = gtk_button_new_from_icon_name(
        "@toolbar-save", GTK_ICON_SIZE_LARGE_TOOLBAR
    );
    gtk_button_set_label(GTK_BUTTON(button_save), "Enregistrer");
    widget_add_class(button_save, "toolbar-button");
    gtk_button_set_always_show_image(GTK_BUTTON(button_save), TRUE);

    widget->button_save = button_save;

    GtkWidget *button_new = gtk_button_new_from_icon_name(
        "@toolbar-new", GTK_ICON_SIZE_LARGE_TOOLBAR
    );
    gtk_button_set_label(GTK_BUTTON(button_new), "Ajouter un film");
    widget_add_class(button_new, "toolbar-button");
    gtk_button_set_always_show_image(GTK_BUTTON(button_new), TRUE);

    widget->button_new = button_new;

    gtk_box_pack_start(GTK_BOX(toolbar), button_open, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), button_save, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), button_new, FALSE, FALSE, 0);

    // add movie provider selection todo

    GtkWidget *providers = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    widget->providers = providers;

    GtkWidget *tmdben = gtk_radio_button_new_with_label(NULL, "TMDb EN");
    widget_add_class(tmdben, "toolbar-button");
    widget_add_class(tmdben, "toolbar-provider");
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(tmdben), FALSE);
    g_signal_connect(tmdben, "toggled", G_CALLBACK(signal_toolbar_provider_change), "tmdb-en");

    GtkWidget *tmdbfr = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(tmdben), "TMDb FR");
    widget_add_class(tmdbfr, "toolbar-button");
    widget_add_class(tmdbfr, "toolbar-provider");
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(tmdbfr), FALSE);
    g_signal_connect(tmdbfr, "toggled", G_CALLBACK(signal_toolbar_provider_change), "tmdb-fr");

    gtk_container_add(GTK_CONTAINER(providers), tmdben);
    gtk_container_add(GTK_CONTAINER(providers), tmdbfr);

    gtk_box_pack_end(GTK_BOX(toolbar), providers, FALSE, FALSE, 0);

    return widget;
}

static struct WidgetSidebar *widget_sidebar_new() {

    struct WidgetSidebar *widget = malloc(sizeof(struct WidgetSidebar));

    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(sidebar, "sidebar");

    widget->sidebar = sidebar;

    // Sidebar search
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(search_box, "searchbar");

    widget->search_box = search_box;

    GtkWidget *search_entry = gtk_entry_new();
    widget_add_class(search_entry, "searchbar-entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Recherche");
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(search_entry), GTK_ENTRY_ICON_PRIMARY, "@edit-find");
    gtk_widget_set_hexpand(GTK_WIDGET(search_entry), FALSE);

    widget->search_entry = search_entry;

    gtk_container_add(GTK_CONTAINER(search_box), search_entry);

    // sidebar categories
    GtkWidget *scrolled_frame = gtk_scrolled_window_new(NULL, NULL); // horiz, vertical adjustement
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_frame), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *list_items = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list_items), GTK_SELECTION_SINGLE);
    widget_add_class(list_items, "categories");
    // todo
    // gtk_list_box_set_placeholder(GTK_LIST_BOX(list_items), gtk_label_new("qsdqsd"))


    widget->list_items = list_items;

    gtk_container_add(GTK_CONTAINER(scrolled_frame), list_items);

    // Add all elements to sidebar
    gtk_box_pack_start(GTK_BOX(sidebar), search_box, FALSE, FALSE, 0); // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(sidebar), scrolled_frame, TRUE, TRUE, 0);

    return widget;
}

static struct WidgetSidebarItem *widget_sidebar_item_new(char *item_id, char *label_text, bool is_favorite) {

    struct WidgetSidebarItem *widget = malloc(sizeof(struct WidgetSidebarItem));

    // list row
    GtkWidget *list_row = gtk_list_box_row_new();
    widget_add_class(list_row, "category-item");
    
    g_object_set_data_full(G_OBJECT(list_row),
        "itemId", item_id, NULL //(GDestroyNotify) g_free
    );

    widget->list_row = list_row;

    // elements inside row
    GtkWidget *list_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *label = gtk_label_new(label_text);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    // gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    // align left, verticial center
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);

    widget->label = label;

    GtkWidget *favorite_icon = gtk_image_new_from_icon_name("@emblem-favorite", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_visible(favorite_icon, (is_favorite));

    widget->favorite_icon = favorite_icon;

    gtk_box_pack_start(GTK_BOX(list_box), label, TRUE, TRUE, 0); // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(list_box), favorite_icon, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(list_row), list_box);

    // fixme
    gtk_widget_show_all(list_row);

    return widget;
}

static void widget_sidebar_add_item(struct WidgetSidebar *sidebar, struct WidgetSidebarItem *item) {
    // gtk_list_box_insert(GTK_LIST_BOX(sidebar->list_items), GTK_LIST_BOX_ROW(item->list_row), -1);
    gtk_container_add(GTK_CONTAINER(sidebar->list_items), GTK_WIDGET(item->list_row));
}



static struct WidgetStatusbar *widget_statusbar_new() {

    struct WidgetStatusbar *widget = malloc(sizeof(struct WidgetStatusbar));

    GtkWidget *statusbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    widget_add_class(statusbar, "statusbar");

    widget->statusbar = statusbar;

    GtkWidget *label = gtk_label_new("");
    widget_add_class(label, "statusbar-message");
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);

    widget->label = label;

    // align left, verticial center
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);

    gtk_widget_set_size_request(statusbar, -1, 30); // width height

    gtk_container_add(GTK_CONTAINER(statusbar), label);

    return widget;
}

static void widget_statusbar_set_text(struct WidgetStatusbar *statusbar, const char *text) {
    gtk_label_set_text(GTK_LABEL(statusbar->label), text);
}




static struct WidgetPanels *widget_panels_new() {

    struct WidgetPanels *widget = malloc(sizeof(struct WidgetPanels));

    GtkWidget *panels = gtk_notebook_new();

    // show tabs only if dev mode
    #if !PACKAGE_DEVELOPER_MODE
        gtk_notebook_set_show_tabs(GTK_NOTEBOOK(panels), FALSE);
    #endif
    gtk_notebook_set_show_border(GTK_NOTEBOOK(panels), FALSE);

    widget->panels = panels;

    // create panels

    struct WidgetPanelWelcome *panel_welcome = widget_panel_welcome_new();
    // widget->panel_welcome = panel_welcome; todo

    struct WidgetPanelPreview *panel_preview = widget_panel_preview_new();
    // widget->panel_preview = panel_preview;

    struct WidgetPanelEdition *panel_edition = widget_panel_edition_new();
    // widget->panel_edition = panel_edition;

    gtk_notebook_append_page(GTK_NOTEBOOK(panels), panel_welcome->panel, gtk_label_new("welcome"));
    gtk_notebook_append_page(GTK_NOTEBOOK(panels), panel_preview->panel, gtk_label_new("preview"));
    gtk_notebook_append_page(GTK_NOTEBOOK(panels), panel_edition->panel, gtk_label_new("edition"));

    return widget;
}

static struct WidgetPanelWelcome *widget_panel_welcome_new() {

    struct WidgetPanelWelcome *widget = malloc(sizeof(struct WidgetPanelWelcome));

    GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(panel, "panel");
    widget_add_class(panel, "panel-welcome");
    widget->panel = panel;


    gtk_container_add(GTK_CONTAINER(panel), gtk_label_new("HELLO!!"));


    return widget;
}

static struct WidgetPanelPreview *widget_panel_preview_new() {

    struct WidgetPanelPreview *widget = malloc(sizeof(struct WidgetPanelPreview));

    GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(panel, "panel");
    widget_add_class(panel, "panel-preview");
    widget->panel = panel;


    struct WidgetStarRating *widget_starrating = widget_starrating_new();
    widget->widget_starrating = widget_starrating;

    GtkWidget *starrating = widget_starrating->starrating;


    // widget_starrating_set_rating(widget_starrating, 20);




    gtk_container_add(GTK_CONTAINER(panel), starrating);


    return widget;
}

static struct WidgetPanelEdition *widget_panel_edition_new() {

    struct WidgetPanelEdition *widget = malloc(sizeof(struct WidgetPanelEdition));

    GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(panel, "panel");
    widget_add_class(panel, "panel-edition");
    widget->panel = panel;










    return widget;
}




static struct WidgetStarRating *widget_starrating_new() {

    struct WidgetStarRating *widget = malloc(sizeof(struct WidgetStarRating));
    widget->interactive = TRUE;

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    widget_add_class(box, "starrating");
    widget->starrating = box;

    const int rate_to_star[] = {20, 40, 60, 80, 100};

    int i;
    for(i = 0; i < 5; i++) {

        GtkWidget *button = gtk_button_new();
        GtkWidget *image = gtk_image_new_from_icon_name("@emblem-staroff", GTK_ICON_SIZE_BUTTON);

        gtk_container_add(GTK_CONTAINER(button), image);

        g_signal_connect(button, "clicked",
            G_CALLBACK(widget_starrating_signal_clicked), widget
        );

        g_object_set_data_full(G_OBJECT(button),
            "rating", GINT_TO_POINTER(rate_to_star[i]), (GDestroyNotify) g_free
        );

        gtk_widget_set_sensitive(button, widget->interactive);
        gtk_style_context_add_class(gtk_widget_get_style_context(button), "starrating-star");
        
        widget->gtkstars[i] = button;

        gtk_widget_set_visible(button, TRUE);
        gtk_container_add(GTK_CONTAINER(box), button);
    }

    return widget;
}

static void widget_starrating_signal_clicked(GtkButton *button, struct WidgetStarRating *stars) {

    if(!stars->interactive) {
        return;
    }

    int rating = GPOINTER_TO_INT(g_object_get_data(
        G_OBJECT(button), "rating")
    );

    widget_starrating_set_rating(stars, rating);
}

static int widget_starrating_get_rating(struct WidgetStarRating *stars) {
    return stars->rating;
}

static void widget_starrating_set_rating(struct WidgetStarRating *stars, int rating) {
    stars->rating = rating;
    
    // add fudge factor so we can actually get 5 stars in reality, and round up to nearest power of 10
    double fudge_rating = rating + 10;
    fudge_rating = 10 * ceil(fudge_rating / 10);

    int i;
    for(i = 0; i < 5; i++) { //todo

        GtkWidget *button = stars->gtkstars[i];

        int btn_rating = GPOINTER_TO_INT(g_object_get_data(
            G_OBJECT(button), "rating")
        );

        GtkWidget *image = gtk_bin_get_child(GTK_BIN(button));

        gtk_image_set_from_icon_name(GTK_IMAGE(image),
            (fudge_rating >= btn_rating ? "@emblem-star" : "@emblem-staroff"), GTK_ICON_SIZE_BUTTON
        );

        // add or remove class if selected
        GtkStyleContext *style = gtk_widget_get_style_context(button);

        gtk_style_context_remove_class(style, fudge_rating >= btn_rating ? "star-disabled" : "star-enabled");
        gtk_style_context_add_class(style, fudge_rating >= btn_rating ? "star-enabled" : "star-disabled");
    }
}

static void widget_starrating_set_interactive(struct WidgetStarRating *stars, bool interactive) {
    stars->interactive = interactive;

    int i;
    for(i = 0; i < 5; i++) { 
        GtkWidget *button = stars->gtkstars[i];
        gtk_widget_set_sensitive(button, interactive);
    }
}

    


// simple message dialog

static void dialog_message(GtkWindow *window, char *message, char *message2) {

    GtkWidget *dialog = gtk_message_dialog_new(window,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        message
    );

    if(message2 != NULL) {
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", message2);
    }

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static char *dialog_file_chooser(char *existing_filename) {
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    char *selected_filename = NULL;

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Ouvrir un fichier", NULL, action, // title
        "Annuler", GTK_RESPONSE_CANCEL,
        "Ouvrir", GTK_RESPONSE_ACCEPT,
        NULL
    );
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);

    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_mime_type(filter, "application/ndjson");
    gtk_file_filter_add_mime_type(filter, "application/json");
    gtk_file_filter_add_mime_type(filter, "text/plain");
    gtk_file_chooser_set_filter(chooser, filter);

    if(existing_filename != NULL) { // file already exist
        gtk_file_chooser_set_filename(chooser, existing_filename);
    }

    int status = gtk_dialog_run(GTK_DIALOG(dialog));

    if(status == GTK_RESPONSE_ACCEPT) {
        selected_filename = gtk_file_chooser_get_filename(chooser);
    }

    gtk_widget_destroy(dialog);

    return selected_filename;
}

static char *show_save_dialog(char *existing_filename) {
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    char *selected_filename = NULL;

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Ouvrir un fichier", NULL, action, // title
        "Annuler", GTK_RESPONSE_CANCEL,
        "Enregistrer", GTK_RESPONSE_ACCEPT,
        NULL
    );

    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_mime_type(filter, "application/ndjson");
    gtk_file_filter_add_mime_type(filter, "application/json");
    gtk_file_filter_add_mime_type(filter, "text/plain");
    gtk_file_chooser_set_filter(chooser, filter);

    if(existing_filename != NULL) { // file already exist
        gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
        gtk_file_chooser_set_filename(chooser, existing_filename);
    } else {
        gtk_file_chooser_set_current_name(chooser, "MyCollection.ndjson");
    }

    int status = gtk_dialog_run(GTK_DIALOG(dialog));

    if(status == GTK_RESPONSE_ACCEPT) {
        selected_filename = gtk_file_chooser_get_filename(chooser);
    }

    gtk_widget_destroy(dialog);

    return selected_filename;
}











static void signal_toolbar_open(GtkButton *button, gpointer user_data) {

    const char *filename = dialog_file_chooser(NULL);

    if(filename == NULL) {
        return;
    }

    GError *error = NULL;

    if(!movie_collection_open(filename, &error)) {

        #if PACKAGE_DEVELOPER_MODE
            g_message("%s %s", __func__, error->message);
        #endif
        
        GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(button));
        dialog_message(GTK_WINDOW(toplevel), "Could not open file", error->message);
    
        g_clear_error(&error); //todo check
    }

    g_free((char*) filename);
}

static void signal_toolbar_save(GtkButton *button, gpointer user_data) {

    const char *filename = show_save_dialog(NULL);

    if(filename == NULL) {
        return;
    }

    GError *error = NULL;
    bool save = movie_collection_save(filename, &error);


//todo if already set

g_message("filename %s", filename);
}

static void signal_toolbar_new(GtkButton *button, gpointer user_data) {

//todo
}












static void show_interactive_dialog(MovieApplication *mapp) {
    const char *appid = g_application_get_application_id(G_APPLICATION(mapp));

    // Initialize GTK+
    GtkWidget *main_window = gtk_application_window_new(GTK_APPLICATION(mapp));
    widget_add_class(main_window, "main_window");

    // Create an 800x600 window that will contain the browser instance
    gtk_window_set_icon_name(GTK_WINDOW(main_window), appid);
    gtk_window_set_title(GTK_WINDOW(main_window), "Movie Collection");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(main_window), 0);
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(main_window), TRUE);

    // Set window settings
    GtkSettings *window_settings = gtk_settings_get_default();
    g_object_set(G_OBJECT(window_settings),
        "gtk-application-prefer-dark-theme", TRUE,
        // "gtk-font-name", "Lato 12",
    NULL);

    g_signal_connect(GTK_WIDGET(main_window), "destroy",
        G_CALLBACK(signal_mainwindow_destroy), mapp
    );

    g_signal_connect(GTK_WINDOW(main_window), "window-state-event",
        G_CALLBACK(signal_mainwindow_state_event), mapp
    );

    g_signal_connect(GTK_WINDOW(main_window), "size-allocate",
        G_CALLBACK(signal_mainwindow_size_allocate), mapp
    );

    // Load window previous state, if any
    if(mapp->win_height > 0 && mapp->win_width > 0) {
        gtk_window_set_default_size(GTK_WINDOW(main_window),
            mapp->win_width,
            mapp->win_height
        );
    }

    if(mapp->is_maximized) {
        gtk_window_maximize(GTK_WINDOW(main_window));
    }

    if(mapp->is_fullscreen) {
        gtk_window_fullscreen(GTK_WINDOW(main_window));
    }

    // Styling application (if file available)
    GtkCssProvider *css_provider = gtk_css_provider_new();

    g_signal_connect(css_provider, "parsing-error",
        G_CALLBACK(signal_css_provider_parsing_error), NULL
    );
    
    gtk_css_provider_load_from_resource(css_provider, "/shell/style.css");

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    gtk_icon_theme_add_resource_path(gtk_icon_theme_get_default(), "/icons");


    ////////// WINDOW DESIGN //////////

    // hide window decorations of main app and use our own
    struct WidgetHeaderbar *widget_headerbar = widget_headerbar_new();

    GtkWidget *headerbar = widget_headerbar->headerbar;
    gtk_window_set_titlebar(GTK_WINDOW(main_window), headerbar);

    // Window Main
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Toolbar
    struct WidgetToolbar *widget_toolbar = widget_toolbar_new();

    GtkWidget *toolbar = widget_toolbar->toolbar;

    g_signal_connect(widget_toolbar->button_open, "clicked",
        G_CALLBACK(signal_toolbar_open), mapp
    );

    g_signal_connect(widget_toolbar->button_save, "clicked",
        G_CALLBACK(signal_toolbar_save), mapp
    );

    g_signal_connect(widget_toolbar->button_new, "clicked",
        G_CALLBACK(signal_toolbar_new), mapp
    );

    // Panel between sidebar and content
    GtkWidget *layout_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_position(GTK_PANED(layout_paned), 300);

    g_signal_connect(GTK_PANED(layout_paned), "notify::position",
        G_CALLBACK(signal_mainwindow_paned_move), mapp
    );

    if(mapp->paned_position > 0) {
        gtk_paned_set_position(GTK_PANED(layout_paned), mapp->paned_position);
    }

    // Sidebar
    struct WidgetSidebar *widget_sidebar = widget_sidebar_new();

    GtkWidget *sidebar = widget_sidebar->sidebar;

    g_signal_connect(widget_sidebar->search_entry, "key-release-event",
        G_CALLBACK(signal_searchentry_keyrelease), mapp
    );

    g_signal_connect(widget_sidebar->search_entry, "changed",
        G_CALLBACK(signal_searchentry_changed), mapp
    );

    g_signal_connect(widget_sidebar->list_items, "row-selected", // categories
        G_CALLBACK(signal_sidebar_list_items_selected), mapp
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








struct WidgetSidebarItem *xxx = widget_sidebar_item_new("ID_1", "XXX", FALSE);
widget_sidebar_add_item(widget_sidebar, xxx);

xxx = widget_sidebar_item_new("ID_2", "azertyuiopqsdfghjklmwxcvbnazertyuiopqsdfghjklmwxcvbnazertyuiopqsdfghjklmwxcvbn", FALSE);
widget_sidebar_add_item(widget_sidebar, xxx);

xxx = widget_sidebar_item_new("ID_3", "<>ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok ok", FALSE);
widget_sidebar_add_item(widget_sidebar, xxx);






    // Put the content area into the main window
    gtk_container_add(GTK_CONTAINER(main_window), main_box);

    // Make sure that when the browser area becomes visible, it will get mouse and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(main_box));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(main_window);
}

static void signal_app_activate(MovieApplication *mapp) {

    // Check if window is already active
    GtkWindow *window = gtk_application_get_active_window(GTK_APPLICATION(mapp));

    if(window != NULL) {
        gtk_window_present(window);
        return;
    }

    // Else, we show the main window
    show_interactive_dialog(mapp);
}

static void signal_app_shutdown(MovieApplication *mapp) {

    // save current window state
    mainwindow_store_state(mapp);
}

static void signal_app_open(MovieApplication *mapp) {
    // todo
}

static int signal_app_command_line(MovieApplication *mapp, GApplicationCommandLine *cmdline) {
    return 0; // exit
}

static void app_commandline_print_version(MovieApplication *mapp) {
    const char *appid = g_application_get_application_id(G_APPLICATION(mapp));

    g_print("%s - GTK:%d.%d.%d \n", appid,
        gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version()
    );
}

static int signal_app_handle_local_options(MovieApplication *mapp, GVariantDict *options) {
    // handle command lines locally

    if(g_variant_dict_lookup(options, "version", "b", NULL)) {
        app_commandline_print_version(mapp);
        return 0;
    }

    if(g_variant_dict_lookup(options, "inspect", "b", NULL)) {
        gtk_window_set_interactive_debugging(TRUE);
    }

    return -1; //let the default option processing continue
}



int main(int argc, char *argv[]) {

    #if PACKAGE_DEVELOPER_MODE
        g_message("Dev mode");
        // inspector debug
        putenv("GTK_DEBUG=fatal-warnings");
        putenv("GOBJECT_DEBUG=instance-count");
        // putenv("G_ENABLE_DIAGNOSTIC=1");
    #endif

    int status;

    // Instantiate the main app
    MovieApplication *mapp = movie_application_new(
        PACKAGE_APPLICATION_ID,
        G_APPLICATION_FLAGS_NONE //| G_APPLICATION_HANDLES_COMMAND_LINE
    );

    // Add aplication main events flow (start with "_init")
    g_signal_connect(mapp, "startup", G_CALLBACK(signal_app_startup), NULL);
    g_signal_connect(mapp, "activate", G_CALLBACK(signal_app_activate), NULL);
    g_signal_connect(mapp, "shutdown", G_CALLBACK(signal_app_shutdown), NULL);
    g_signal_connect(mapp, "open", G_CALLBACK(signal_app_open), NULL);

    // Add app main arguments
    GOptionEntry entries[] = {
        {"version", 'v', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL, "Show program version", NULL},
        {"inspect", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL, "Open the interactive debugger", NULL},
        {NULL}
    };
    g_application_add_main_option_entries(G_APPLICATION(mapp), entries);

    g_signal_connect(mapp, "handle-local-options", G_CALLBACK(signal_app_handle_local_options), NULL);
    g_signal_connect(mapp, "command-line", G_CALLBACK(signal_app_command_line), NULL); // received from remote

    // Run the app and get its exit status
    status = g_application_run(G_APPLICATION(mapp), argc, argv);

    g_object_unref(mapp);

    return status;
}



// headerbar spacing 5