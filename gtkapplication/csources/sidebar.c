#include "sidebar.h"
#include "widgets.h"
#include "movietype.h"
#include <string.h>

// type definition
struct _WidgetSidebar {
    GtkBox parent_instance;

    GtkWidget *searchbox;
    GtkWidget *searchentry;

    GtkWidget *listbox;
    GListModel *listmodel;
};

enum {
    SIGNAL_SEARCH,
    SIGNAL_SELECTED,
    SIGNAL_LAST
};
static int signals[SIGNAL_LAST];

G_DEFINE_TYPE(WidgetSidebar, widget_sidebar, GTK_TYPE_BOX);

// internals
static void sidebar_finalize(GObject *obj);
//events
static bool signal_search_keypress(GtkSearchEntry *entry, GdkEventKey *event, WidgetSidebar *sidebar);
static void signal_search_changed(GtkSearchEntry *entry, WidgetSidebar *sidebar);
static void signal_listbox_selected(GtkListBox *listbox, GtkListBoxRow *listrow, WidgetSidebar *sidebar);
static GtkWidget *listbox_create_placeholder();
// list box
static GtkWidget *listbox_create_widget(GSequenceIter *iter, char *title, bool is_favorite);
static void listbox_model_changed(GListModel *list, unsigned int position, unsigned int removed, unsigned int added, WidgetSidebar *sidebar);
static void listbox_model_notify_visibility(Movie *movie, bool is_visible, GtkWidget *listrow);


WidgetSidebar *widget_sidebar_new() {
    g_message(__func__);

    return g_object_new(widget_sidebar_get_type(),
        "orientation", GTK_ORIENTATION_VERTICAL,
    NULL);
}

static void widget_sidebar_class_init(WidgetSidebarClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->finalize = sidebar_finalize;

    signals[SIGNAL_SEARCH] = g_signal_new("search",
        G_OBJECT_CLASS_TYPE(object_class),
        G_SIGNAL_RUN_FIRST,
        0, // offset
        NULL, NULL, NULL, //accumulator, accu_data, c_marshaller
        G_TYPE_NONE, // return
        1, G_TYPE_STRING // params
    );

    signals[SIGNAL_SELECTED] = g_signal_new("selected",
        G_OBJECT_CLASS_TYPE(object_class),
        G_SIGNAL_RUN_FIRST,
        0, // offset
        NULL, NULL, NULL, //accumulator, accu_data, c_marshaller
        G_TYPE_NONE, // return
        1, G_TYPE_POINTER // params
    );
}

static void widget_sidebar_init(WidgetSidebar *widget) {

    widget_add_class(GTK_WIDGET(widget), "sidebar");

    // search
    GtkWidget *searchbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    widget_add_class(searchbox, "searchbox");
    widget->searchbox = searchbox;

    GtkWidget *searchentry = gtk_search_entry_new(); //todo: white icon
    widget_add_class(searchentry, "searchbox-entry");
    widget->searchentry = searchentry;

    gtk_entry_set_placeholder_text(GTK_ENTRY(searchentry), "Recherche");
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(searchentry), GTK_ENTRY_ICON_PRIMARY, "@edit-find");
    gtk_widget_set_hexpand(GTK_WIDGET(searchentry), false);

    g_signal_connect(searchentry, "key-press-event", G_CALLBACK(signal_search_keypress), widget);
    g_signal_connect(searchentry, "search-changed", G_CALLBACK(signal_search_changed), widget);

    gtk_container_add(GTK_CONTAINER(searchbox), searchentry);

    // sidebar categories
    GtkWidget *scrolledframe = gtk_scrolled_window_new(NULL, NULL); // h,v adjustement
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledframe), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *listbox = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(listbox), GTK_SELECTION_SINGLE);
    widget_add_class(listbox, "categories");
    widget->listbox = listbox;

    g_signal_connect(listbox, "row-selected", G_CALLBACK(signal_listbox_selected), widget);

    GtkWidget *placeholder = listbox_create_placeholder();
    gtk_list_box_set_placeholder(GTK_LIST_BOX(listbox), placeholder);


    gtk_container_add(GTK_CONTAINER(scrolledframe), listbox);

    // Add all elements to sidebar
    gtk_box_pack_start(GTK_BOX(widget), searchbox, false, false, 0); // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(widget), scrolledframe, true, true, 0);
}

static void sidebar_finalize(GObject *obj) {
    WidgetSidebar *sidebar = WIDGET_SIDEBAR(obj);

    if(sidebar->listmodel) {
        g_signal_handlers_disconnect_by_func(sidebar->listmodel, listbox_model_changed, obj);
        g_clear_object(&sidebar->listmodel);
    }
}





static bool signal_search_keypress(GtkSearchEntry *entry, GdkEventKey *event, WidgetSidebar *sidebar) {
    if(event->keyval == GDK_KEY_Escape) {
        gtk_entry_set_text(GTK_ENTRY(entry), ""); // empty
        g_signal_emit(sidebar, signals[SIGNAL_SEARCH], 0, NULL);
    }
    return GDK_EVENT_PROPAGATE;
}

static void signal_search_changed(GtkSearchEntry *entry, WidgetSidebar *sidebar) {
    const char *keyword = gtk_entry_get_text(GTK_ENTRY(entry));
    g_signal_emit(sidebar, signals[SIGNAL_SEARCH], 0, (strlen(keyword) > 0 ? keyword : NULL));
}

static void signal_listbox_selected(GtkListBox *listbox, GtkListBoxRow *listrow, WidgetSidebar *sidebar) {
    g_return_if_fail(G_IS_OBJECT(listrow));

    gpointer iter = g_object_get_data(G_OBJECT(listrow), "iter");
    g_return_if_fail(iter != NULL);

    g_signal_emit(sidebar, signals[SIGNAL_SELECTED], 0, iter);
}

static GtkWidget *listbox_create_placeholder() {

    GtkWidget *widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    widget_add_class(widget, "categories-placeholder");

    gtk_widget_set_can_focus(widget, false);
    gtk_widget_set_halign(widget, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(widget, GTK_ALIGN_CENTER);

    GtkWidget *icon = gtk_image_new_from_icon_name("@edit-find", GTK_ICON_SIZE_DND);
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 64);

    GtkWidget *label1 = gtk_label_new("Aucun résultat trouvé");
    PangoAttrList *list1 = pango_attr_list_new();
    pango_attr_list_insert(list1, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    pango_attr_list_insert(list1, pango_attr_scale_new(1.5));
    gtk_label_set_attributes(GTK_LABEL(label1), list1);

    GtkWidget *label2 = gtk_label_new("Essayez une autre recherche");

    gtk_container_add(GTK_CONTAINER(widget), icon);
    gtk_container_add(GTK_CONTAINER(widget), label1);
    gtk_container_add(GTK_CONTAINER(widget), label2);

    gtk_widget_show_all(widget);

    return widget;
}


void widget_sidebar_listbox_bind_model(WidgetSidebar *sidebar, GListModel *model) {
    g_return_if_fail(model == NULL || G_IS_LIST_MODEL(model));

    // delete previous
    gtk_container_foreach(GTK_CONTAINER(sidebar->listbox), (GtkCallback) gtk_widget_destroy, NULL);

    GListModel *listmodel = g_object_ref(model);
    sidebar->listmodel = listmodel;

    // append all existing
    g_signal_connect(listmodel, "items-changed", G_CALLBACK(listbox_model_changed), sidebar);

    unsigned int n_items;
    if((n_items = g_list_model_get_n_items(listmodel)) > 0) {
        listbox_model_changed(listmodel, 0, 0, n_items, sidebar);
    }
}

static void listbox_model_notify_visibility(Movie *movie, bool is_visible, GtkWidget *listrow) {
    gtk_widget_set_visible(listrow, is_visible);
}

static void listbox_model_changed(GListModel *list, unsigned int position, unsigned int removed, unsigned int added, WidgetSidebar *sidebar) {
    GtkWidget *listbox = sidebar->listbox;

    while(removed--) {
        GtkListBoxRow *row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(listbox), position);
        gtk_container_remove(GTK_CONTAINER(listbox), GTK_WIDGET(row));
    }

    for(unsigned int i = 0; i < added; i++) {
        GSequenceIter *iter = g_list_model_get_item(list, position + i);
        Movie *movie = g_sequence_get(iter);

        GtkWidget *widget = listbox_create_widget(
            iter, movie->title, movie->favorite
        );

        g_signal_connect(movie, "visibility", G_CALLBACK(listbox_model_notify_visibility), widget);

        // allow to either return a full reference or a floating reference
        // floating => then turn it into a full reference now
        if(g_object_is_floating(widget)) {
            g_object_ref_sink(widget);
        }

        gtk_widget_show(widget);

        gtk_list_box_insert(GTK_LIST_BOX(listbox), widget, position + i);
        g_object_unref(widget);
    }
}


static GtkWidget *listbox_create_widget(GSequenceIter *iter, char *title, bool is_favorite) {

    GtkWidget *widget = gtk_list_box_row_new();
    widget_add_class(widget, "category-item");

    // set iterator from movies list
    g_object_set_data(G_OBJECT(widget), "iter", iter);

    // elements inside row
    GtkWidget *listbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_visible(listbox, true);

    GtkWidget *label = gtk_label_new(title);
    gtk_widget_set_visible(label, true);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    // gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);

    // align left, verticial center
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.5);

    GtkWidget *icon_fav = gtk_image_new_from_icon_name("@emblem-favorite", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_visible(icon_fav, is_favorite);

    gtk_box_pack_start(GTK_BOX(listbox), label, true, true, 0); // expand, fill, padding
    gtk_box_pack_start(GTK_BOX(listbox), icon_fav, false, false, 0);

    gtk_container_add(GTK_CONTAINER(widget), listbox);

    return widget;
}
