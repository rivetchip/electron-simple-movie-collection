#include "collection.h"
#include "vector.h"
#include <json-glib/json-glib.h>
#include <string.h>

// type definition
struct _MoviesList {
    int meta_version;
    char *meta_created;
    char *meta_imported;
    char *meta_source;

    unsigned int capacity;
    unsigned int total;
    struct Movie **movies;
};

// internals
static bool list_resize(MoviesList *l, int capacity);
static bool json_metadata_parse(JsonObject *object, MoviesList *list);
static bool json_node_parse(JsonObject *object, struct Movie *movie);
static size_t getline(FILE *stream, char **lineptr, size_t *n);
static bool str_equal(const char *string1, const char *string2);


MoviesList *movies_list_new() {
    MoviesList *l = malloc(sizeof(*l));

    l->capacity = 20;
    l->total = 0;
    l->movies = malloc(sizeof(struct Movie*) * l->capacity);

    return l;
}

int movies_list_total(MoviesList *l) {
    return l->total;
}

static bool list_resize(MoviesList *l, int capacity) {
    printf("list_resize: %d to %d\n", l->capacity, capacity);

    struct Movie **movies;
    if((movies = realloc(l->movies, sizeof(struct Movie*) * capacity)) != NULL) {
        l->movies = movies;
        l->capacity = capacity;
        return true;
    }
    return false;
}

bool movies_list_add(MoviesList *l, struct Movie *movie, int *index) {
    if(l->capacity == l->total) {
        if(!list_resize(l, l->capacity * 2)) {
            return false;
        }
    }
    l->movies[l->total++] = movie;

    *index = (l->total - 1); // return last key
    return true;
}

bool movies_list_set(MoviesList *l, int index, struct Movie *movie) {
    if(index >= 0 && index < l->total) {
        l->movies[index] = movie;
        return true;
    }
    return false;
}

struct Movie *movies_list_get(MoviesList *l, int index) {
    if(index >= 0 && index < l->total) {
        return l->movies[index];
    }
    return NULL;
}

bool movies_list_delete(MoviesList *l, int index) {
    if(index >= 0 && index < l->total) {
        l->movies[index] = NULL;
        return true;
    }
    // we dont resize the collection here, to preserve keys (todo)
    return false;
}

void movies_list_free(MoviesList *l) {
    free(l->movies);
    free(l);

    // while(l->total > 0){
    //     free(l->movies[--l->size]);
    // }
}

MoviesList *movies_list_new_from_stream(FILE *stream, GError **error) {
    size_t line_size = 0;
    char *line = NULL;
    size_t read = 0;

    JsonParser *parser = json_parser_new();
    JsonObject *jsonnode;

    MoviesList *list = movies_list_new();

    unsigned int line_number = 0;
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
                    g_warning("%s: Could not metada @ %i", __func__, line_number);
                    continue;
                }
            }
            break;

            default: { // others: movie
                struct Movie *movie = malloc(sizeof(*movie));
                if(!json_node_parse(jsonnode, movie)) {
                    g_warning("%s: Could not parse @ %i", __func__, line_number);
                    continue;
                }
                
                int movieId;
                if(!movies_list_add(list, movie, &movieId)) {
                    g_warning("%s: Could not add @ %i", __func__, line_number);
                    continue;
                }
                // g_message("%s %d", movie->title, movieId);
            }
            break;
        }

        line_number++;
    }

    free(line);
    g_object_unref(parser);

    return list;
}

// void movie_collection_foreach(MoviesTable *table, void (*foreach)(const char *movieId, struct Movie *movie, void *user_data), void *user_data) {
//     int total = vector_total(table->movies);

//     for(unsigned int index = 0; index < total; index++) {
//         struct Movie *movie;

//         if((movie = vector_get(table->movies, index)) != NULL) {
//             const char *movieId = movie->movieId;

//             (*foreach)(movieId, movie, user_data);
//         }
//     }
// }

char *movie_collection_stringify(MoviesList *l) {
    char *retval;

    // JsonGenerator *generator = json_generator_new();
    // json_generator_set_pretty(generator, true); // todo
    // json_generator_set_root(generator, node);

        // retval = json_generator_to_data(generator, NULL);
        // g_object_unref(generator);

    return retval;
}

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

static bool json_node_parse(JsonObject *object, struct Movie *movie) {
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

static bool str_equal(const char *string1, const char *string2) {
    return strcmp (string1, string2) == 0;
}

