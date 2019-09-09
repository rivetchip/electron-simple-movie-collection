#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>

#include "macros.h"
#include "vector.h"

#ifdef  __cplusplus
    extern "C" {
#endif




////////////////////



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
