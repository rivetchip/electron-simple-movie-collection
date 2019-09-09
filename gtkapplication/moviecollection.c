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



static char* strdup(const char *str) {
    if(!str) return NULL;

    size_t length = strlen(str) + 1;
    char *new_str = malloc(length * sizeof(char));
    memcpy(new_str, str, length);

    return new_str;
}
















static void signal_mainwindow_paned_move(GtkPaned *paned, GParamSpec *pspec, MovieApplication *app) {
    app->paned_position = gtk_paned_get_position(paned);
}








static void signal_searchentry_keyrelease(GtkEntry *entry, GdkEventKey *event, MovieApplication *app) {
    if(event->keyval == GDK_KEY_Escape) {
        gtk_entry_set_text(entry, ""); // empty
    }
}

static void signal_searchentry_changed(GtkEntry *entry, MovieApplication *app) {
    const char *keyword = gtk_entry_get_text(entry);

    #if PACKAGE_DEVELOPER_MODE
        g_message("%s %s", __func__, keyword);
    #endif

}







static void signal_sidebar_list_items_selected(GtkListBox *listbox, GtkListBoxRow *listrow, MovieApplication *app) {

    const char *item_id = g_object_get_data(
        G_OBJECT(GTK_WIDGET(listrow)), "itemId"
    );

    #if PACKAGE_DEVELOPER_MODE
        g_message("%s %s", __func__, item_id);
    #endif


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
    gtk_widget_set_hexpand(GTK_WIDGET(search_entry), false);

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
    gtk_box_pack_start(GTK_BOX(sidebar), search_box, false, false, 0); // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(sidebar), scrolled_frame, true, true, 0);

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
    // gtk_label_set_line_wrap(GTK_LABEL(label), true);
    // align left, verticial center
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);

    widget->label = label;

    GtkWidget *favorite_icon = gtk_image_new_from_icon_name("@emblem-favorite", GTK_ICON_SIZE_SMALL_TOOLBAR);
    widget->favorite_icon = favorite_icon;

    gtk_box_pack_start(GTK_BOX(list_box), label, true, true, 0); // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(list_box), favorite_icon, false, false, 0);

    gtk_container_add(GTK_CONTAINER(list_row), list_box);

    gtk_widget_show_all(list_row);
    gtk_widget_set_visible(favorite_icon, is_favorite);

    return widget;
}

static void widget_sidebar_add_item(struct WidgetSidebar *sidebar, struct WidgetSidebarItem *item) {
    // gtk_list_box_insert(GTK_LIST_BOX(sidebar->list_items), GTK_LIST_BOX_ROW(item->list_row), -1);
    gtk_container_add(GTK_CONTAINER(sidebar->list_items), GTK_WIDGET(item->list_row));
}






static struct WidgetPanels *widget_panels_new() {

    struct WidgetPanels *widget = malloc(sizeof(struct WidgetPanels));

    GtkWidget *panels = gtk_notebook_new();

    // show tabs only if dev mode
    #if !PACKAGE_DEVELOPER_MODE
        gtk_notebook_set_show_tabs(GTK_NOTEBOOK(panels), false);
    #endif
    gtk_notebook_set_show_border(GTK_NOTEBOOK(panels), false);

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
    widget->interactive = true;

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

        gtk_widget_set_visible(button, true);
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

    















// headerbar spacing 5