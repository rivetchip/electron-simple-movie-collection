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
    //
}

Movie *movie_new() {
    return g_object_new(movie_get_type(), NULL);
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

