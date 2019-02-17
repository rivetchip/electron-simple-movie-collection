#pragma once

#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>

G_BEGIN_DECLS


// Type definition (or move it for main c file)
struct _MovieApplication {
    GtkApplication parent_instance;

    // window state
    int win_height;
    int win_width;
    bool is_maximized;
    bool is_fullscreen;
};

typedef struct _MovieApplication MovieApplication;
typedef GtkApplicationClass MovieApplicationClass;

// Public API
GType movie_application_get_type(void) G_GNUC_CONST;

// Macro Functions G_DECLARE_FINAL_TYPE
static inline MovieApplication *MOVIE_APPLICATION(gpointer ptr) {
    return G_TYPE_CHECK_INSTANCE_CAST(ptr, movie_application_get_type(), MovieApplication);
}

// internal API
static MovieApplication *movie_application_new(const char *application_id, GApplicationFlags flags);
static void movie_application_init(MovieApplication *app);
static void movie_application_class_init(MovieApplicationClass *klass);

////////////////////

struct WidgetSidebar {
    GtkWidget *sidebar; // container
    GtkWidget *search_box;
    GtkWidget *search_entry;
    GtkWidget *list_items; // movies
};

struct WidgetSidebarItem {
    GtkWidget *list_row; // container
    GtkWidget *label;
    GtkWidget *favorite_icon;
};

static struct WidgetSidebar *widget_sidebar_new();
static struct WidgetSidebarItem *widget_sidebar_item_new(char *item_id, char *label_text, bool is_favorite);
static void widget_sidebar_items_add(struct WidgetSidebar *sidebar, struct WidgetSidebarItem *item);

struct WidgetPanels {
    GtkWidget *panels; // container
    GtkWidget *panel_welcome;
    GtkWidget *panel_preview;
    GtkWidget *panel_edition;
};

struct WidgetPanelWelcome {
    GtkWidget *panel; // container

};

struct WidgetPanelPreview {
    GtkWidget *panel; // container

};

struct WidgetPanelEdition {
    GtkWidget *panel; // container

};

static struct WidgetPanelWelcome *widget_panel_welcome_new();
static struct WidgetPanelPreview *widget_panel_preview_new();
static struct WidgetPanelEdition *widget_panel_edition_new();
static struct WidgetPanels *widget_panels_new();







////////////////////


struct MovieCollectionItem { //fixme: multiple flexible array+check overflow
    char *title;
    bool favorite;
    int rating;

    char *tagline;
    char *originalTitle;
    int ratingPress;
    int duration;
    char *dateReleased;
    char *dateCreated;
    char *dateModified;
    char *poster;
    char *description;
    char *comment;
    char *director;
    char *countries[255];
    char *genres[255];
    char *actors[255][2];
    char *serie;
    char *companies[255];
    char *keywords[255];
    char *source;
    int sourceId;
    char *webPage;

    struct WidgetSidebarItem *widget_item;
};

struct MovieCollection {
    int version;
    struct MovieCollectionItem movies[];

};



G_END_DECLS
