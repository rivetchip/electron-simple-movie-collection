#include "sidebar.h"
#include "widgets.h"

// type definition
struct _WidgetSidebar {
    GtkBox parent_instance;

    GtkWidget *searchbox;
    GtkWidget *searchentry;
    GtkWidget *listbox;
};

enum {
    SIGNAL_SEARCH_KEYWORD,
    SIGNAL_LAST
};
static int signals[SIGNAL_LAST];

G_DEFINE_TYPE(WidgetSidebar, widget_sidebar, GTK_TYPE_BOX);

//events
static void signal_search_keyrelease(GtkEntry *entry, GdkEventKey *event, WidgetSidebar *sidebar);
static void signal_search_changed(GtkEntry *entry, WidgetSidebar *sidebar);
static void signal_listbox_selected(GtkListBox *listbox, GtkListBoxRow *listrow, WidgetSidebar *sidebar);
static GtkWidget *list_create_placeholder();
//items
// static GtkWidget *listbox_row_new(int movieId, char *title, bool is_favorite);


static void widget_sidebar_init(WidgetSidebar *self) {
    g_message(__func__);
}

static void widget_sidebar_class_init(WidgetSidebarClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    signals[SIGNAL_SEARCH_KEYWORD] = g_signal_new("search-keyword",
        G_OBJECT_CLASS_TYPE(object_class),
        G_SIGNAL_RUN_FIRST,
        0, // offset
        NULL, NULL, NULL, //accumulator, accu_data, c_marshaller
        G_TYPE_NONE, // return
        1, G_TYPE_STRING // params
    );
}

WidgetSidebar *widget_sidebar_new() {
    return g_object_new(widget_sidebar_get_type(),
        "orientation", GTK_ORIENTATION_VERTICAL,
    NULL);
}

WidgetSidebar *movie_application_new_sidebar() {

    WidgetSidebar *widget = widget_sidebar_new();
    widget_add_class(GTK_WIDGET(widget), "sidebar");

    // search
    GtkWidget *searchbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(searchbox, "searchbox");

    widget->searchbox = searchbox;

    GtkWidget *searchentry = gtk_entry_new();
    widget_add_class(searchentry, "searchbox-entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(searchentry), "Recherche");
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(searchentry), GTK_ENTRY_ICON_PRIMARY, "@edit-find");
    gtk_widget_set_hexpand(GTK_WIDGET(searchentry), false);

    g_signal_connect(searchentry, "key-release-event", G_CALLBACK(signal_search_keyrelease), widget);
    g_signal_connect(searchentry, "changed", G_CALLBACK(signal_search_changed), widget);

    widget->searchentry = searchentry;

    gtk_container_add(GTK_CONTAINER(searchbox), searchentry);

    // sidebar categories
    GtkWidget *scrolledframe = gtk_scrolled_window_new(NULL, NULL); // h,v adjustement
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledframe), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *listbox = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(listbox), GTK_SELECTION_SINGLE);
    widget_add_class(listbox, "categories");

    g_signal_connect(listbox, "row-selected", G_CALLBACK(signal_listbox_selected), widget);

    GtkWidget *placeholder = list_create_placeholder();
    gtk_list_box_set_placeholder(GTK_LIST_BOX(listbox), placeholder);

    widget->listbox = listbox;

    gtk_container_add(GTK_CONTAINER(scrolledframe), listbox);

    // Add all elements to sidebar
    gtk_box_pack_start(GTK_BOX(widget), searchbox, false, false, 0); // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(widget), scrolledframe, true, true, 0);

    return widget;
}

static void signal_search_keyrelease(GtkEntry *entry, GdkEventKey *event, WidgetSidebar *sidebar) {
    if(event->keyval == GDK_KEY_Escape) {
        gtk_entry_set_text(entry, ""); // empty
    }
}

static void signal_search_changed(GtkEntry *entry, WidgetSidebar *sidebar) {
    const char *keyword = gtk_entry_get_text(entry);
    g_signal_emit(sidebar, signals[SIGNAL_SEARCH_KEYWORD], 0, keyword);
}

static void signal_listbox_selected(GtkListBox *listbox, GtkListBoxRow *listrow, WidgetSidebar *sidebar) {

    int movieId = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(listrow), "movieId"));

    g_message("%s %d", __func__, movieId); //todo
}

static GtkWidget *list_create_placeholder() { //todo
    GtkWidget *label = gtk_label_new("placehold");
    gtk_widget_show(label);

    return label;
}


void widget_sidebar_listbox_bind_model(WidgetSidebar *sidebar, GListModel *model, GtkListBoxCreateWidgetFunc create_widget_func, gpointer user_data, GDestroyNotify user_data_free_func) {
    gtk_list_box_bind_model(GTK_LIST_BOX(sidebar->listbox), model, create_widget_func, user_data, user_data_free_func);
}

GtkWidget *widget_sidebar_listbox_widget(WidgetSidebar *sidebar, int movieId, char *title, bool is_favorite) {

    GtkWidget *widget = gtk_list_box_row_new();
    widget_add_class(widget, "category-item");

    g_object_set_data(G_OBJECT(widget), "movieId", GINT_TO_POINTER(movieId));

    // elements inside row
    GtkWidget *list_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *label = gtk_label_new(title);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    // gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    // align left, verticial center
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);

    GtkWidget *favorite_icon = gtk_image_new_from_icon_name("@emblem-favorite", GTK_ICON_SIZE_SMALL_TOOLBAR);

    gtk_box_pack_start(GTK_BOX(list_box), label, TRUE, TRUE, 0); // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(list_box), favorite_icon, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(widget), list_box);

    gtk_widget_show_all(widget);
    gtk_widget_set_visible(favorite_icon, is_favorite);

    return widget;
}



// GList *children, *iter;

// children = gtk_container_get_children(GTK_CONTAINER(container));
// for(iter = children; iter != NULL; iter = g_list_next(iter))
//   gtk_widget_destroy(GTK_WIDGET(iter->data));
// g_list_free(children);


//todo :add_item