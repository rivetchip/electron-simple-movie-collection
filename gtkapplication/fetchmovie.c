#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>


static struct CurlMemoryStruct {
    char *stream;
    size_t size;
};

static size_t curl_writer_callback(void *chunk, size_t size, size_t size_chunk, struct CurlMemoryStruct *mem) {
    size_t realsize = size * size_chunk;
 
    char *ptr = realloc(mem->stream, (mem->size + realsize + 1));
    if(ptr == NULL) {
        //out of memory!
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
 
    mem->stream = ptr;
    memcpy(&(mem->stream[mem->size]), chunk, realsize);
    mem->size += realsize;
    mem->stream[mem->size] = 0;
 
    return realsize;
}

static char *fetch(const char *method, const char *uri, char **error_msg) {

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, uri);

    struct CurlMemoryStruct *response = malloc(sizeof(struct CurlMemoryStruct));
    response->stream = malloc(1); // will be grown as needed by the realloc above
    response->size = 0; // no data at this point

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer_callback);

    // Add Headers
    struct curl_slist *req_headers = NULL;
    req_headers = curl_slist_append(req_headers, "User-Agent: Mozilla/5.0");
    req_headers = curl_slist_append(req_headers, "Accept: application/json");
    req_headers = curl_slist_append(req_headers, "Accept-Language: fr");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, req_headers);

    // include headers
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L); // if >4XX
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);

    // curl_easy_setopt(curl, CURLOPT_POST, 1L);
    // curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    // curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, sizeof long);

    // allow redirect to http/https
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);

    // verbose output
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    CURLcode curl_code = curl_easy_perform(curl);

    curl_slist_free_all(req_headers);
    curl_easy_cleanup(curl);

    if(curl_code != CURLE_OK) {
        const char *curl_error = curl_easy_strerror(curl_code);
        *error_msg = (char *) malloc(sizeof(curl_error));
        strcpy(*error_msg, curl_error);

        return NULL;
    }

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    long header_size;
    curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &header_size);

    char *response_headers = malloc(header_size);
    strncpy(response_headers, response->stream, header_size); // only first

    long body_size = response->size - header_size;
    char *response_body = malloc(body_size);
    strncpy(response_body, response->stream + header_size, body_size); // only first


    free(response_headers);

    return response_body;
}



char *fetchmovie(char *source, char *action, char *keyword, char *lang) {

}

