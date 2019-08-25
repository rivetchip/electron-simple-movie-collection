#include "collection.h"
#include "vector.h"
#include <json-glib/json-glib.h>

// type definition
struct _MoviesTable {
    int version;
    char *created;
    char *imported;
    char *source;

    vector *movies; // array[Movie]
};

// internals
static size_t getline(FILE *stream, char **lineptr, size_t *n);
static bool json_metadata_parse(JsonObject *object, MoviesTable *movies_table);
static bool json_node_parse(JsonObject *object, struct Movie *movie);


MoviesTable *movie_collection_new() {
    MoviesTable *table = malloc(sizeof(*table));
    table->movies = vector_new();

    return table;
}

MoviesTable *movie_collection_new_from_stream(FILE *stream, GError **error) {

    size_t line_size = 0;
    char *line = NULL;
    size_t read = 0;

    JsonParser *parser = json_parser_new();
    JsonObject *jsonnode;

    MoviesTable *table = movie_collection_new();

    unsigned int i = 0;
    while((read = getline(stream, &line, &line_size)) > 0) {

        if(!json_parser_load_from_data(parser, line, strlen(line), error)) {
            free(line);
            g_object_unref(parser);
            return NULL;
        }

        jsonnode = json_node_get_object(json_parser_get_root(parser));

        switch(i) {
            case 0: { // first line: metadata
                if(!json_metadata_parse(jsonnode, table)) {
                    continue;
                }
            }
            break;

            default: { // movies
                struct Movie *movie = malloc(sizeof(*movie));
                if(!json_node_parse(jsonnode, movie)) {
                    continue;
                }

                if(!movie_collection_add(table, movie->movieId, movie)) {
                    continue;
                }
            }
            break;
        }

        i++;
    }

    free(line);
    g_object_unref(parser);

    return table;
}

void movie_collection_foreach(MoviesTable *table, void (*foreach)(const char *movieId, struct Movie *movie, void *user_data), void *user_data) {
    int total = vector_total(table->movies);

    for(unsigned int index = 0; index < total; index++) {
        struct Movie *movie;

        if((movie = vector_get(table->movies, index)) != NULL) {
            const char *movieId = movie->movieId;

            (*foreach)(movieId, movie, user_data);
        }
    }
}

struct Movie *movie_collection_get(MoviesTable *table, char *movieId) {
    int total = vector_total(table->movies);

    struct Movie *movie;
    for(unsigned int index = 0; index < total; index++) {
        if((movie = vector_get(table->movies, index)) != NULL) {
            return movie;
        }
    }
    return NULL;
}

bool movie_collection_add(MoviesTable *table, const char *movieId, struct Movie *movie) {

    // todo if index=0 success ??

    int key = vector_add(table->movies, movie);

    return TRUE;
}

bool movie_collection_remove(MoviesTable *table, char *movieId) {
    int total = vector_total(table->movies);

    for(unsigned int index = 0; index < total; index++) {
        if((vector_delete(table->movies, index))) {
            return TRUE;
        }
    }
    return FALSE;
}

char *movie_collection_stringify(MoviesTable *table) {
    char *retval;

    // JsonGenerator *generator = json_generator_new();
    // json_generator_set_pretty(generator, TRUE); // todo
    // json_generator_set_root(generator, node);

        // retval = json_generator_to_data(generator, NULL);
        // g_object_unref(generator);

    return retval;
}

static bool json_metadata_parse(JsonObject *object, MoviesTable *metadata) {
    JsonNode *node;

    if((node = json_object_get_member(object, "version")) != NULL) {
        metadata->version = json_node_get_int(node);
    }
    if((node = json_object_get_member(object, "source")) != NULL) {
        metadata->source = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "created")) != NULL) {
        metadata->created = json_node_dup_string(node);
    }
    if((node = json_object_get_member(object, "imported")) != NULL) {
        metadata->imported = json_node_dup_string(node);
    }

    return TRUE;
}

static bool json_node_parse(JsonObject *object, struct Movie *movie) {
    JsonNode *node; JsonArray *array;

    if((node = json_object_get_member(object, "movieId")) != NULL) {
        movie->movieId = json_node_dup_string(node);
    }
    if(movie->movieId == NULL) {
        return FALSE;
    }

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

    return TRUE;
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
