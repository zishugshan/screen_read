#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include <curl/curl.h>

#define MAX_LINES 200
#define MAX_LINE 1024
#define MAX_LINE_LEN 256
#define DATE_PATTERN "^[0-9]{1,2} [A-Za-z]+ [0-9]{4}$"
#define AMOUNT_PATTERN "₹[0-9]+"

// find total amount (sum of transaction) todo
void calculate_total_amount(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("ACannot open file");
        return;
    }

    char line[256];
    int total = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Amount:", 7) == 0) {
            // Find the ₹ symbol and extract the number
            char *rupee = strstr(line, "Amount");
            if (rupee) {
                int amount = atoi(rupee + 3); // Skip "₹" and possible space
                total += amount;
            }
        }
    }

    fclose(fp);
    printf("💰 Total Amount Spent: ₹%d\n", total);
}

char* time_stamp(){
  // Get current time
    static char timestamp[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d  %H:%M:%S\n", t);
    return timestamp;
}

void remove_empty_lines(const char *input, char *output) {
    const char *line_start = input;
    char *out_ptr = output;

    while (*line_start) {
        // Find the end of the current line
        const char *line_end = strchr(line_start, '\n');
        if (!line_end) line_end = line_start + strlen(line_start);

        // Skip empty or whitespace-only lines
        int non_empty = 0;
        for (const char *p = line_start; p < line_end; p++) {
            if (!isspace(*p)) {
                non_empty = 1;
                break;
            }
        }

        if (non_empty) {
            memcpy(out_ptr, line_start, line_end - line_start);
            out_ptr += line_end - line_start;
            *out_ptr++ = '\n';
        }

        // Move to the next line
        if (*line_end == '\n')
            line_start = line_end + 1;
        else
            break;
    }

    *out_ptr = '\0';
}

void copy_to_clipboard_and_append() {
    const char *input_file = "screenshot.txt";
    const char *history_file = "history.txt";

    FILE *fp_in = fopen(input_file, "r");
    if (!fp_in) {
        perror("Failed to open screenshot.txt");
        return;
    }

    // Read input text
    char *raw_text = malloc(10000);
    fread(raw_text, 1, 9999, fp_in);
    raw_text[9999] = '\0';
    fclose(fp_in);

    // Clean text by removing empty lines
    char *cleaned_text = malloc(10000);
    remove_empty_lines(raw_text, cleaned_text);

    // Copy to clipboard
    FILE *pipe = popen("pbcopy", "w");
    if (pipe) {
        fwrite(cleaned_text, 1, strlen(cleaned_text), pipe);
        pclose(pipe);
    }

    FILE *fp_hist = fopen(history_file, "a");
    if (!fp_hist) {
        perror("Failed to open history.txt");
        free(raw_text);
        free(cleaned_text);
        return;
    }
    // Add two line breaks
    fputs("\n\n", fp_hist);


    // Write timestamp
    fputs(time_stamp(), fp_hist);
    fputs("--------------------\n", fp_hist);


    fputs(cleaned_text, fp_hist);
    fclose(fp_in);
    fclose(fp_hist);
    pclose(pipe);

    printf("Text copied to clipboard and saved to history.txt \n");
}

void copy_to_clipboard() {
    const char *filename = "screenshot.txt";
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open screenshot.txt");
        return;
    }

    FILE *pipe = popen("pbcopy", "w");
    if (!pipe) {
        perror("Failed to run pbcopy");
        fclose(fp);
        return;
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp)) {
        fputs(buffer, pipe);
    }

    fclose(fp);
    pclose(pipe);
    printf("Text from screenshot.txt copied to clipboard!\n");
}

void read_screenshot() {
    FILE *file = fopen("screenshot.txt", "r");
    if (!file) {
        perror("Error opening screenshot.txt");
        return;
    }

    printf("\nOCR Text Output:\n");
    printf("--------------------------------------------------\n");

    char line[4096];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }

    printf("--------------------------------------------------\n");

    fclose(file);
}

// Helper to extract amount (last number in a line)
int extract_amount(const char *line) {
    int amount = 0;
    const char *ptr = line + strlen(line) - 1;

    // Go backwards to find digits
    while (ptr >= line && !isdigit(*ptr)) ptr--;
    const char *end = ptr;

    while (ptr >= line && isdigit(*ptr)) ptr--;
    ptr++;

    if (end >= ptr) {
        char num[16] = {0};
        strncpy(num, ptr, end - ptr + 1); 
        amount = atoi(num);
    }   

    return amount;
}

// Extract date (first date-like pattern)
void extract_date(const char *line, char *date) {
    const char *months[] = { 
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };  
    for (int i = 0; i < 12; i++) {
        char *pos = strstr(line, months[i]);
        if (pos && pos - line >= 3) {
            strncpy(date, pos - 3, 11);
            date[11] = '\0';
            return;
        }   
    }   
    strcpy(date, "Unknown");
}


void extract_transactions(const char *text) {
    char *lines[MAX_LINES];
    int count = 0;

    // Duplicate and split text into lines
    char *text_copy = strdup(text);
    char *line = strtok(text_copy, "\n");

    while (line && count < MAX_LINES) {
        lines[count++] = strdup(line);
        line = strtok(NULL, "\n");
    }

    regex_t date_regex, amount_regex;
    regcomp(&date_regex, DATE_PATTERN, REG_EXTENDED);
    regcomp(&amount_regex, AMOUNT_PATTERN, REG_EXTENDED);

    for (int i = 1; i < count - 1; i++) {
        if (regexec(&date_regex, lines[i], 0, NULL, 0) == 0) {
            char *name = lines[i - 1];
            char *date = lines[i];

            // Search for ₹ amount in line below
            char *amount_line = lines[i + 1];
            regmatch_t match;
            if (regexec(&amount_regex, amount_line, 1, &match, 0) == 0) {
                int len = match.rm_eo - match.rm_so;
                char amount[16];
                strncpy(amount, amount_line + match.rm_so, len);
                amount[len] = '\0';
                printf("%s .. %s .. %s\n", name, date, amount);
            }
        }
    }

    regfree(&date_regex);
    regfree(&amount_regex);
    free(text_copy);
}

void clean_row_text() {
    FILE *file = fopen("transactions.txt", "r");
    if (!file) {
        perror("Error opening OCR output file");
        return;
    }

    char line[MAX_LINE];
    char next_line[MAX_LINE];
    char name[256];
    char date[64];
    int total_amount = 0;
    while (fgets(line, sizeof(line), file)) {
        // Skip lines with these keywords
        if (strstr(line, "Debited from") ||
            strstr(line, "Payment") ||
            strstr(line, "Paid to") ||
            strstr(line, "Transfer to") ||
            strstr(line, "History")) {
            continue;
        }

        // Extract amount
        int amount = extract_amount(line);
        total_amount += amount;
        if (amount <= 0) continue;

        // Extract name from non-digit characters
        memset(name, 0, sizeof(name));
        int len = strlen(line);
        for (int i = 0, j = 0; i < len && j < sizeof(name) - 1; i++) {
            if (!isdigit((unsigned char)line[i])) {
                name[j++] = line[i];
            }
        }

        // Trim trailing newline, %, spaces
        size_t namelen = strlen(name);
        while (namelen > 0 && (isspace(name[namelen - 1]) || name[namelen - 1] == '%' || name[namelen - 1] == '\n')) {
            name[--namelen] = '\0';
        }

        // Read next line (expecting date)
        if (!fgets(next_line, sizeof(next_line), file)) break;

        memset(date, 0, sizeof(date));
        extract_date(next_line, date);

        if (strlen(name) > 2 && strlen(date) > 5) {
            printf("Name  : %s\n", name);
            printf("Date  : %s\n", date);
            printf("Amount: ₹%d\n\n", amount);
        }
    }
    printf("Total Amount is : %d\n",total_amount);
    fclose(file);
}
