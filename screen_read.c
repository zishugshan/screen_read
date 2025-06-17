#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"
#include "api_endpoints.h"
#include <ctype.h>
void read_screenshot();
void upload_file(const char *filepath);
void copy_to_clipboard();
void copy_to_clipboard_and_append();
void save_history(const char *file_path);

int main() {
    const char *screenshot_path = "screenshot.png";
    char command[256];

    printf("📸 Capturing screenshot...\n");
    int result;
    #ifdef __APPLE__
    result = system("screencapture -i screenshot.png");
    #else
    result = system("scrot screenshot.png");
    #endif

    if (result != 0) {
        fprintf(stderr, "Screenshot capture failed.\n");
        return 1;
    }

    // upload_file(screenshot_path);
    
    // Step: OCR with Tesseract
    printf("🧠 Running OCR on %s...\n", screenshot_path);

    // Step 1: OCR with Tesseract
    snprintf(command, sizeof(command), "tesseract \"%s\" screenshot -l eng --psm 6", screenshot_path);
    result = system(command);
    if (result != 0) {
        fprintf(stderr, "Tesseract failed to run\n");
        return 1;
    }

    // Step 3: Read screenshot.txt file
    read_screenshot();
    copy_to_clipboard();
    copy_to_clipboard_and_append();
    // save_history("history.txt");
    return 0;
}

