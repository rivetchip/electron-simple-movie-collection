#include "collection.h"
#include <json-glib/json-glib.h>
#include <string.h>
#include <ctype.h>

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

    GSequence *movies;
    // cache
    bool access_prohibited;
    GSequenceIter *last_iter;
    unsigned int last_position;
};

// macros
static char *stristr(const char *haystack, const char *needle) {
    do {
        const char* h = haystack;
        const char* n = needle;
        while(*n && tolower(*h) == tolower(*n)) {
            h++;
            n++;
        }
        if(*n == 0) {
            return (char *) haystack;
        }
    } while(*haystack++);
    return NULL;
}

#define str_empty(str) (!(str) || !*(str))
#define str_equal(str1, str2) (strcmp(str1, str2) == 0)
#define str_contains(str1, str2) (stristr(str1, str2) != NULL)
// internals
static void movies_list_iface_init(GListModelInterface *iface);
static GType list_iface_get_item_type(GListModel *list);
static guint list_iface_get_n_items(GListModel *list);
static gpointer list_iface_get_item(GListModel *list, guint position);
static void list_iface_dispose(GObject *object);
// events
static bool check_seq_access(MoviesList *list);
static void list_items_changed(MoviesList *list, unsigned int position, unsigned int removed, unsigned int added);
// json parsing
static bool json_metadata_parse(JsonObject *object, MoviesList *list);
static bool json_node_parse(JsonObject *object, Movie *movie);
static size_t getline(FILE *stream, char **lineptr, size_t *n);

G_DEFINE_TYPE_EXTENDED(MoviesList, movies_list, G_TYPE_OBJECT, 0,
    G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL, movies_list_iface_init)
);
// G_DEFINE_TYPE_WITH_CODE(MoviesList, movies_list, G_TYPE_OBJECT,
//     G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL, movies_list_iface_init)
// );

static void movies_list_iface_init(GListModelInterface *iface) {
    iface->get_item_type = list_iface_get_item_type;
    iface->get_n_items = list_iface_get_n_items;
    iface->get_item = list_iface_get_item;
}

static guint list_iface_get_n_items(GListModel *glist) {
    MoviesList *list = MOVIES_LIST(glist);
    return g_sequence_get_length(list->movies);
}

static GType list_iface_get_item_type(GListModel *glist) {
    return G_TYPE_POINTER;
}

static gpointer list_iface_get_item(GListModel *glist, guint position) {
    // g_message("%s %i", __func__, position);

    MoviesList *list = MOVIES_LIST(glist);
    GSequenceIter *iter = NULL;

    if(list->last_iter) {
        if(position < G_MAXUINT && list->last_position == position + 1) {
            iter = g_sequence_iter_prev(list->last_iter);
        }
        else if(position > 0 && list->last_position == position - 1) {
            iter = g_sequence_iter_next(list->last_iter);
        }
        else if(list->last_position == position) {
            iter = list->last_iter;
        }
    } else {
        iter = g_sequence_get_iter_at_pos(list->movies, position);
    }

    list->last_iter = iter;
    list->last_position = position;

    if(!g_sequence_iter_is_end(iter)) {
        return (iter);
        // return g_object_ref(g_sequence_get(iter));
    }
    return NULL;
}

static void list_iface_dispose(GObject *object) {
    MoviesList *list = MOVIES_LIST(object);
    g_clear_pointer(&list->movies, g_sequence_free);
}

static void movies_list_class_init(MoviesListClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = list_iface_dispose;
}

static void movies_list_init(MoviesList *list) {
    list->movies = g_sequence_new(g_object_unref);
    list->last_position = 0;
    list->access_prohibited = false;
}

static bool check_seq_access(MoviesList *list) {
    if(G_UNLIKELY(list->access_prohibited)) {
        g_warning("Accessing a sequence while it is being sorted or searched is not allowed");
        return false;
    }
    return true;
}

static void list_items_changed(MoviesList *list, unsigned int position, unsigned int removed, unsigned int added) {
    // check if the iter cache may have been invalidated
    if(position <= list->last_position) {
        list->last_iter = NULL;
        list->last_position = 0;
    }

    g_list_model_items_changed(G_LIST_MODEL(list), position, removed, added);
}

MoviesList *movies_list_new() {
    g_message(__func__);

    return g_object_new(movies_list_get_type(), NULL);
}

GSequenceIter *movies_list_append(MoviesList *list, Movie *movie) {
    GSequenceIter *iter = g_sequence_append(list->movies, g_object_ref(movie));
    int n_items = g_sequence_get_length(list->movies);

    list_items_changed(list, (n_items-1), 0, 1);
    return iter;
}

GSequenceIter *movies_list_insert(MoviesList *list, Movie *movie, unsigned int position) {
    g_return_val_if_fail(position <= g_sequence_get_length(list->movies), NULL);

    GSequenceIter *iter = g_sequence_get_iter_at_pos(list->movies, position);
    g_sequence_insert_before(iter, g_object_ref(movie));

    list_items_changed(list, position, 0, 1);
    return iter;
}

GSequenceIter *movies_list_append_sorted(MoviesList *list, Movie *movie, GCompareDataFunc compare_func, gpointer user_data) {

    GSequenceIter *iter = g_sequence_insert_sorted(list->movies, g_object_ref(movie),
        compare_func, user_data
    );
    unsigned int position = g_sequence_iter_get_position(iter);

    list_items_changed(list, position, 0, 1);
    return iter;
}

bool movies_list_remove(MoviesList *list, GSequenceIter *iter) {
    g_return_val_if_fail(!g_sequence_iter_is_end(iter), false);

    unsigned int position = g_sequence_iter_get_position(iter);
    g_sequence_remove(iter);

    list_items_changed(list, position, 1, 0);
    return true;
}

bool movies_list_remove_all(MoviesList *list) {
    int n_items;
    if((n_items = g_sequence_get_length(list->movies)) > 0) {
        g_sequence_remove_range(
            g_sequence_get_begin_iter(list->movies),
            g_sequence_get_end_iter(list->movies)
        );

        list_items_changed(list, 0, n_items, 0);
    }
    return true;
}

bool movies_list_sort(MoviesList *list, GCompareDataFunc compare_func, gpointer user_data) {
    g_sequence_sort(list->movies, compare_func, user_data);

    int n_items = g_sequence_get_length(list->movies);

    list_items_changed(list, 0, n_items, n_items);
    return true;
}

static inline bool search_keyword_match(Movie *movie, const char *keyword) {
    return (keyword == NULL || (movie->title != NULL && str_contains(movie->title, keyword)));
}

bool movies_list_search_keyword(MoviesList *list, const char *keyword) {
    if(!check_seq_access(list)) {
        return false;
    }

    GSequenceIter *begin = g_sequence_get_begin_iter(list->movies);
    GSequenceIter *end = g_sequence_get_end_iter(list->movies);

    g_return_val_if_fail(begin != NULL, false);
    g_return_val_if_fail(end != NULL, false);

    list->access_prohibited = true;

    GSequenceIter *iter = begin;
    unsigned int position;

    while(iter != end) {
        GSequenceIter *next = g_sequence_iter_next(iter);

        Movie *movie = g_sequence_get(iter);
        position = g_sequence_iter_get_position(iter);

        bool visible = search_keyword_match(movie, keyword);
        movie_notify_visible(movie, visible);

        // g_message(">> %s / %s / %s // %d", (visible?"X":"-"), keyword, movie->title, position);

        iter = next;
    }

    list->access_prohibited = false;

    // list_items_changed(list, 0, n_items, n_items);
    return true;
}






bool movies_list_stream(MoviesList *list, FILE *stream, GError **error) {
    size_t line_size = 0;
    char *line = NULL;
    size_t read = 0;

    JsonParser *parser = json_parser_new();

    size_t line_number = 0;
    while((read = getline(stream, &line, &line_size)) > 0) {

        if(!json_parser_load_from_data(parser, line, strlen(line), error)) {
            free(line);
            g_object_unref(parser);
            return false;
        }

        JsonNode *jsonnode;
        if((jsonnode = json_parser_get_root(parser)) == NULL) {
            // todo return error
            g_warning("# %s: Json node is invalid @ %zu", __func__, line_number);
            return false;
        }

        JsonObject *jsonobject = json_node_get_object(jsonnode);

        switch(line_number) {
            case 0: { // first: metadata
                if(!json_metadata_parse(jsonobject, list)) {
                    g_warning("# %s: Could not metada @ %zu", __func__, line_number);
                    continue;
                }
            }
            break;

            default: { // others: movie
                Movie *movie = movie_new();
                if(!json_node_parse(jsonobject, movie)) {
                    g_warning("# %s: Could not parse @ %zu", __func__, line_number);
                    continue;
                }

                GSequenceIter *iter;
                if((iter = movies_list_append(list, movie)) == NULL) {
                    g_warning("# %s: Could not add @ %zu", __func__, line_number);
                    continue;
                }
                // g_message(">> ADD %s %d", movie->title, g_sequence_iter_get_position(iter));
            }
            break;
        }

        line_number++;
    }

    free(line);
    g_object_unref(parser);

    return true;
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

