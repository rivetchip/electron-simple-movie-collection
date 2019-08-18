#pragma once

#include <stdbool.h>

#include <gtk/gtk.h>

#ifdef  __cplusplus
    extern "C" {
#endif


struct WidgetHeaderbar {
    GtkWidget *headerbar; // container
    GtkWidget *button_close;
    GtkWidget *button_minimize;
    GtkWidget *button_maximize;
};

struct WidgetToolbar {
    GtkWidget *toolbar; // container
    GtkWidget *button_open;
    GtkWidget *button_save;
    GtkWidget *button_new;
    GtkWidget *providers;
};

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

////////////////////

struct WidgetStatusbar {
    GtkWidget *statusbar; // container
    GtkWidget *label;
};

////////////////////

struct WidgetStarRating {
    GtkWidget *starrating; // container
    bool interactive; // can be clickable
    int rating;
    GtkWidget *gtkstars[5]; //buttons
};



#ifdef  __cplusplus
    } // extern c
#endif
