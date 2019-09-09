#include "collection.h"
#include <json-glib/json-glib.h>

// type definition
struct _MoviesListClass {
    GObjectClass parent_class;
};
struct _MoviesList {
    GObject parent_instance;

    int meta_version;
    char *meta_created;
    char *meta_imported;
    char *meta_source;

    unsigned int capacity;
    unsigned int total;
    Movie **movies;
};

// internals
static void movies_list_iface_init(GListModelInterface *iface);
static GType list_iface_get_item_type(GListModel *list);
static guint list_iface_get_n_items(GListModel *list);
static gpointer list_iface_get_item(GListModel *list, guint position);
static void list_items_changed(MoviesList *list, unsigned int position, unsigned int removed, unsigned int added);

G_DEFINE_TYPE_EXTENDED(MoviesList, movies_list, G_TYPE_OBJECT, 0,
    G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL, movies_list_iface_init)
);
// G_DEFINE_TYPE_WITH_CODE(MoviesList, movies_list, G_TYPE_OBJECT,
//     G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL, movies_list_iface_init)
// );

static bool list_resize(MoviesList *list, unsigned int capacity);
static bool json_metadata_parse(JsonObject *object, MoviesList *list);
static bool json_node_parse(JsonObject *object, Movie *movie);
static size_t getline(FILE *stream, char **lineptr, size_t *n);
// static bool str_equal(const char *string1, const char *string2);

static void movies_list_iface_init(GListModelInterface *iface) {
    iface->get_item_type = list_iface_get_item_type;
    iface->get_n_items = list_iface_get_n_items;
    iface->get_item = list_iface_get_item;
}

static guint list_iface_get_n_items(GListModel *glist) {
    MoviesList *list = MOVIES_LIST(glist);

    g_message("%s %i", __func__, movies_list_total(list));

    return movies_list_total(list);
}

static GType list_iface_get_item_type(GListModel *glist) {
    return G_TYPE_POINTER;
}
static gpointer list_iface_get_item(GListModel *glist, guint position) {
    g_message("%s %i", __func__, position);

    MoviesList *list = MOVIES_LIST(glist);
    Movie *movie = movies_list_get(list, position);
    return movie;
}

static void movies_list_class_init(MoviesListClass *klass) {
    // GObjectClass *object_class = G_OBJECT_CLASS(klass);
}

static void movies_list_init(MoviesList *list) {
    list->capacity = 20;
    list->total = 0;
    list->movies = malloc(sizeof(Movie*) * list->capacity);
}

static void list_items_changed(MoviesList *list, unsigned int position, unsigned int removed, unsigned int added) {
    g_list_model_items_changed(G_LIST_MODEL(list), position, removed, added);
}


MoviesList *movies_list_new() {
    return g_object_new(movies_list_get_type(), NULL);
}

unsigned int movies_list_total(MoviesList *list) {
    return list->total;
}

static bool list_resize(MoviesList *list, unsigned int capacity) {
    printf("list resize: %u to %u\n", list->capacity, capacity);

    Movie **movies;
    if((movies = realloc(list->movies, sizeof(Movie*) * capacity)) != NULL) {
        list->movies = movies;
        list->capacity = capacity;
        return true;
    }
    return false;
}

bool movies_list_add(MoviesList *list, Movie *movie, unsigned int *index) {
    if(list->capacity == list->total) {
        if(!list_resize(list, list->capacity * 2)) {
            return false;
        }
    }
    list->movies[list->total++] = movie;

    *index = (list->total - 1); // return last key

    list_items_changed(list, *index, 0, 1);
    return true;
}

bool movies_list_set(MoviesList *list, unsigned int index, Movie *movie) {
    if(index >= 0 && index < list->total) {
        list->movies[index] = movie;

        list_items_changed(list, index, 0, 1);
        return true;
    }
    return false;
}

Movie *movies_list_get(MoviesList *list, unsigned int index) {
    // g_message("%s %d", __func__, index);

    if(index >= 0 && index < list->total) {
        return list->movies[index];
    }
    return NULL;
}

bool movies_list_remove(MoviesList *list, unsigned int index) {
    if(index < 0 || index >= list->total) {
        return false;
    }

    // we dont shift the collection, to preserve keys (todo)
    if(list->movies[index] != NULL) {
        list->movies[index] = NULL;
        list_items_changed(list, index, 1, 0);
    }

    // if quarter full the contents is reallocated to a vector of half the current size
    if(list->total > 0 && list->total == list->capacity / 4) {
        if(!list_resize(list, list->capacity / 2)) {
            return false;
        }
    }
    return false;
}

bool movies_list_remove_all(MoviesList *list) {
    unsigned int removed;
    if((removed = movies_list_total(list)) > 0) {
        Movie *movie;
        for(unsigned int i = 0; i < removed - 1; i++) {
            if((movie = movies_list_get(list, i)) != NULL) {
                movies_list_remove(list, i);
                movie_destroy(movie);
                g_message(">>>>DESTR %s %d", __func__, i);
            }
        }

        list->total = 0; // reinit
        // list_items_changed(list, 0, removed, 0);
    }

    return true;
}

void movies_list_destroy(MoviesList *list) {
    movies_list_remove_all(list);
    free(list->movies);
}

MoviesList *movies_list_from_stream(MoviesList *list, FILE *stream, GError **error) {
    size_t line_size = 0;
    char *line = NULL;
    size_t read = 0;

    JsonParser *parser = json_parser_new();
    JsonObject *jsonnode;

    size_t line_number = 0;
    while((read = getline(stream, &line, &line_size)) > 0) {

        if(!json_parser_load_from_data(parser, line, strlen(line), error)) {
            free(line);
            g_object_unref(parser);
            return NULL;
        }

        jsonnode = json_node_get_object(json_parser_get_root(parser));

        switch(line_number) {
            case 0: { // first: metadata
                if(!json_metadata_parse(jsonnode, list)) {
                    g_warning("%s: Could not metada @ %zu", __func__, line_number);
                    continue;
                }
            }
            break;

            default: { // others: movie
                Movie *movie = movie_new();
                if(!json_node_parse(jsonnode, movie)) {
                    g_warning("%s: Could not parse @ %zu", __func__, line_number);
                    continue;
                }

                unsigned int index;
                if(!movies_list_add(list, movie, &index)) {
                    g_warning("%s: Could not add @ %zu", __func__, line_number);
                    continue;
                }
                g_message(">> ADD %s %d", movie->title, index);
            }
            break;
        }

        line_number++;
    }

    free(line);
    g_object_unref(parser);

    return list;
}

bool movies_list_foreach(MoviesList *list, unsigned int *index, Movie **movie) {
    if((*index > 0 && *index > list->total) || list->total == 0) {
        return false; // out of bounds
    }
    // loop from index to end, find the next found item
    Movie *temp;
    for(unsigned int i = *index; i < list->total - 1; i++) {
        (*index)++; // next
        if((temp = movies_list_get(list, i)) != NULL) {
            *movie = temp;
            return true;
        }
    }
    return false;
}









// char *movies_list_stringify(MoviesList *list) {
//     char *retval;

//     // JsonGenerator *generator = json_generator_new();
//     // json_generator_set_pretty(generator, true); // todo
//     // json_generator_set_root(generator, node);

//         // retval = json_generator_to_data(generator, NULL);
//         // g_object_unref(generator);

//     return retval;
// }


static bool json_metadata_parse(JsonObject *object, MoviesList *metadata) {
    JsonNode *node;

    if((node = json_object_get_member(object, "version")) != NULL) {
        metadata->meta_version = json_node_get_int(node);
    }
    if((node = json_object_get_member(object, "source")) != NULL) {
        metadata->meta_source = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "created")) != NULL) {
        metadata->meta_created = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "imported")) != NULL) {
        metadata->meta_imported = json_node_dup_string(node);
    }

    return true;
}

static bool json_node_parse(JsonObject *object, Movie *movie) {
    JsonNode *node; JsonArray *array;

    // if((node = json_object_get_member(object, "movieId")) != NULL) {
    //     movie->movieId = json_node_dup_string(node);
    // }

    if((node = json_object_get_member(object, "title")) != NULL) {
        movie->title = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "favorite")) != NULL) {
        movie->favorite = json_node_get_boolean(node);
    }
    if((node = json_object_get_member(object, "rating")) != NULL) {
        movie->rating = json_node_get_int(node);
    }
    if((node = json_object_get_member(object, "tagline")) != NULL) {
        movie->tagline = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "originalTitle")) != NULL) {
        movie->originalTitle = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "ratingPress")) != NULL) {
        movie->ratingPress = json_node_get_int(node);
    }
    if((node = json_object_get_member(object, "duration")) != NULL) {
        movie->duration = json_node_get_int(node);
    }
    if((node = json_object_get_member(object, "dateReleased")) != NULL) {
        movie->dateReleased = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "dateCreated")) != NULL) {
        movie->dateCreated = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "dateModified")) != NULL) {
        movie->dateModified = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "poster")) != NULL) {
        movie->poster = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "description")) != NULL) {
        movie->description = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "comment")) != NULL) {
        movie->comment = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "director")) != NULL) {
        movie->director = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "countries")) != NULL && JSON_NODE_HOLDS_ARRAY(node)) {
        array = json_node_get_array(node);

        int i; int length;
        if((length = json_array_get_length(array)) > 0) {
            char **values = malloc((length + 1) * sizeof(*values)); // +sentinel

            for (i = 0; i < length; i++) {
                node = json_array_get_element(array, i);
                values[i] = json_node_dup_string(node);
            }
            values[i++] = NULL;

            movie->countries = values;
        }
    }
    if((node = json_object_get_member(object, "genres")) != NULL && JSON_NODE_HOLDS_ARRAY(node)) {
        array = json_node_get_array(node);

    }
    if((node = json_object_get_member(object, "actors")) != NULL && JSON_NODE_HOLDS_ARRAY(node)) {
        array = json_node_get_array(node);

    }
    if((node = json_object_get_member(object, "serie")) != NULL) {
        movie->serie = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "companies")) != NULL && JSON_NODE_HOLDS_ARRAY(node)) {

    }
    if((node = json_object_get_member(object, "keywords")) != NULL && JSON_NODE_HOLDS_ARRAY(node)) {
        // todo cehck movie->keyword if nothing defined


    }
    if((node = json_object_get_member(object, "source")) != NULL) {
        movie->source = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "sourceId")) != NULL) {
        movie->sourceId = json_node_get_int(node);
    }
    if((node = json_object_get_member(object, "webPage")) != NULL) {
        movie->webPage = json_node_dup_string(node);
    }

    return true;
}



static size_t getline(FILE *stream, char **lineptr, size_t *n) {
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

// static bool str_equal(const char *string1, const char *string2) {
//     return strcmp (string1, string2) == 0;
// }
