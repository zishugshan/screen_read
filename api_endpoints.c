#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

void save_history(const char *file_path) {
    CURL *curl;
    CURLcode res;
    curl_mime *form = NULL;
    curl_mimepart *field = NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        form = curl_mime_init(curl);

        // Add the file field
        field = curl_mime_addpart(form);
        curl_mime_name(field, "file");
        curl_mime_filedata(field, file_path);

        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:3000/history");
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

        printf("\nUploading history.txt...\n");
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        else
            printf("History uploaded successfully\n");

        curl_mime_free(form);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;
    fwrite(ptr, size, nmemb, stdout);  // Print response
    return total;
}

void get_history(const char *id) {
    CURL *curl;
    CURLcode res;
    char url[256];

    snprintf(url, sizeof(url), "http://localhost:3000/history/%s", id);

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        printf("\nFetching history for ID: %s\n", id);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        else
            printf("\nHistory fetched successfully\n");

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}


void upload_file(const char *filepath) {
    CURL *curl;
    CURLcode res;
    curl_mime *form = NULL;
    curl_mimepart *field = NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        form = curl_mime_init(curl);

        // Add the file field
        field = curl_mime_addpart(form);
        curl_mime_name(field, "file");
        curl_mime_filedata(field, filepath);

        // Set the URL and POST the form
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:3000/upload");
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

        printf("\nUploading file: %s\n", filepath);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
            fprintf(stderr, "Upload failed: %s\n", curl_easy_strerror(res));
        else
            printf("Upload successful\n");

        // Cleanup
        curl_mime_free(form);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

