#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>


G_BEGIN_DECLS


// Type definition (or move it for main c file)
struct _MovieApplication {
    GtkApplication parent_instance;

    // window state
    int win_height;
    int win_width;
    bool is_maximized;
    bool is_fullscreen;
    int paned_position;
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

struct MovieCollectionItem {
    char *title;
    bool favorite;
    int rating; // /100

    char *tagline;
    char *originalTitle;
    int ratingPress;
    int duration; // minutes
    char *dateReleased;
    char *dateCreated;
    char *dateModified;
    char *poster;
    char *description;
    char *comment;
    char *director;
    char *countries; // array
    char *genres; // array
    char *actors; // array[2]
    char *serie; // array
    char *companies; // array
    char *keywords; // array
    char *source;
    int sourceId;
    char *webPage;
};

struct MovieCollection {
    int version;
    size_t size;
    GHashTable *movies; // array[MovieCollectionItem]
};

////////////////////

struct WidgetHeaderbar {
    GtkWidget *headerbar; // container
    GtkWidget *button_close;
    GtkWidget *button_minimize;
    GtkWidget *button_maximize;
};

static struct WidgetHeaderbar *widget_headerbar_new();

struct WidgetToolbar {
    GtkWidget *toolbar; // container
    GtkWidget *button_open;
    GtkWidget *button_save;
    GtkWidget *button_new;
    GtkWidget *providers;
};

static struct WidgetToolbar *widget_toolbar_new();

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
static void widget_sidebar_add_item(struct WidgetSidebar *sidebar, struct WidgetSidebarItem *item);

////////////////////

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
    struct WidgetStarRating *widget_starrating;

};

struct WidgetPanelEdition {
    GtkWidget *panel; // container
    struct WidgetStarRating *widget_starrating;

};

static struct WidgetPanels *widget_panels_new();
static struct WidgetPanelWelcome *widget_panel_welcome_new();
static struct WidgetPanelPreview *widget_panel_preview_new();
static struct WidgetPanelEdition *widget_panel_edition_new();

struct WidgetStatusbar {
    GtkWidget *statusbar; // container
    GtkWidget *label;
};

static struct WidgetStatusbar *widget_statusbar_new();
static void widget_statusbar_set_text(struct WidgetStatusbar *statusbar, const char* text);

////////////////////

struct WidgetStarRating {
    GtkWidget *starrating; // container
    bool interactive; // can be clickable
    int rating;
    GtkWidget *gtkstars[5]; //buttons
};

static struct WidgetStarRating *widget_starrating_new();
static int widget_starrating_get_rating(struct WidgetStarRating *stars);
static void widget_starrating_set_rating(struct WidgetStarRating *stars, int rating);
static void widget_starrating_set_interactive(struct WidgetStarRating *stars, bool interactive);
static void widget_starrating_signal_clicked(GtkButton *button, struct WidgetStarRating *stars);

////////////////////






G_END_DECLS
