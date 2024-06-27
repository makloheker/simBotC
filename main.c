// C nih boss.. bahasa para leluhur

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

#define URL "https://api.simsimi.vn/v1/simtalk"
#define LC "id"

struct response_data {
    char *data;
    size_t size;
};

size_t write_callback(void *ptr, size_t size, size_t nmemb, struct response_data *data) {
    size_t total_size = size * nmemb;
    char *ptr_realloc = realloc(data->data, data->size + total_size + 1);
    if(ptr_realloc == NULL) {
        fprintf(stderr, "memory error!!1!1!...\n");
        return 0;
    }
    data->data = ptr_realloc;
    memcpy(&(data->data[data->size]), ptr, total_size);
    data->size += total_size;
    data->data[data->size] = '\0';
    return total_size;
}

char *send_request(const char *text) {
    CURL *curl;
    CURLcode res;
    struct response_data response = {0};

    curl = curl_easy_init();
    if(curl) {
        char post_fields[256];
        snprintf(post_fields, sizeof(post_fields), "text=%s&lc=%s", text, LC);

        curl_easy_setopt(curl, CURLOPT_URL, URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(NULL, "Content-Type: application/x-www-form-urlencoded"));

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(response.data);
            curl_easy_cleanup(curl);
            return NULL;
        }

        curl_easy_cleanup(curl);
    }

    return response.data;
}

char *parse_response(const char *response_data) {
    json_t *root;
    json_error_t error;
    root = json_loads(response_data, 0, &error);
    if(!root) {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return NULL;
    }

    json_t *message = json_object_get(root, "message");
    if(!json_is_string(message)) {
        fprintf(stderr, "error: message is not a string\n");
        json_decref(root);
        return NULL;
    }

    const char *message_text = json_string_value(message);
    char *result = strdup(message_text);
    json_decref(root);

    return result;
}

int main() {
    char input_text[256];

    while(1) {
        printf("you>: ");
        if(fgets(input_text, sizeof(input_text), stdin) == NULL) {
            break;
        }

        input_text[strcspn(input_text, "\n")] = '\0';

        if(strcmp(input_text, "exit") == 0 || strcmp(input_text, "quit") == 0 || strcmp(input_text, "keluar") == 0) {
            printf("quit bye dahh...\n");
            break;
        }

        char *response_data = send_request(input_text);
        if(response_data) {
            char *message = parse_response(response_data);
            if(message) {
                printf("bot>: %s\n", message);
                free(message);
            }
            free(response_data);
        } else {
            printf("error failed get response...\n");
        }
    }

    return 0;
}
