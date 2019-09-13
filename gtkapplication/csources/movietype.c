#include "movietype.h"

// type definition
struct _MovieClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE(Movie, movie, G_TYPE_OBJECT);

// internals
static void movie_class_init(MovieClass *klass) {
    // GObjectClass *object_class = G_OBJECT_CLASS(klass);
    // GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
}

static void movie_init(Movie *self) {
    // g_message(__func__);
}

Movie *movie_new() {
    Movie *movie = g_object_new(movie_get_type(), NULL);
    return movie;
}

bool movie_is_visible(Movie *movie) {
    return movie->widget_visible;
}
void movie_set_visible(Movie *movie, bool visible) {
    movie->widget_visible = visible;
}

void movie_destroy(Movie *movie) {
    free(movie->title);
    //todo
}

