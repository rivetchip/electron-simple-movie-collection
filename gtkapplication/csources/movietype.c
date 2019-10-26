#include "movietype.h"

// type definition
struct _MovieClass {
    GObjectClass parent_class;
};

enum {
    SIGNAL_VISIBILITY,
    SIGNAL_LAST
};
static int signals[SIGNAL_LAST];

G_DEFINE_TYPE(Movie, movie, G_TYPE_OBJECT);


Movie *movie_new() {
    return g_object_new(movie_get_type(), NULL);
}

static void movie_class_init(MovieClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    signals[SIGNAL_VISIBILITY] = g_signal_new("visibility",
        G_OBJECT_CLASS_TYPE(object_class),
        G_SIGNAL_RUN_FIRST,
        0, // offset
        NULL, NULL, NULL, //accumulator, accu_data, c_marshaller
        G_TYPE_NONE, // return
        1, G_TYPE_BOOLEAN // params
    );
}

static void movie_init(Movie *movie) {
    //
}

void movie_notify_visible(Movie *movie, bool visible) {
    g_signal_emit(movie, signals[SIGNAL_VISIBILITY], 0, visible);
}

void movie_destroy(Movie *movie) {
    free(movie->title);
    //todo
}

