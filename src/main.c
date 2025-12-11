// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/utils.h"

#define USERNAME_LEN 48
#define PASSWORD_LEN 48

void menu() {
    puts("\n--- Secure MFA Authentication System (terminal) ---");
    puts("1. Create user");
    puts("2. Login");
    puts("3. Exit");
    printf("Choice: ");
}

/* safe_input: read at most (len-1) chars, strip newline */
void safe_input(char *buf, size_t len) {
    if (!fgets(buf, (int)len, stdin)) {
        buf[0] = '\0';
        return;
    }
    size_t L = strnlen(buf, len);
    if (L > 0 && buf[L-1] == '\n') buf[L-1] = '\0';
    else { /* if there's leftover input, flush it */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) { }
    }
}

int main(void) {
    srand((unsigned)time(NULL));
    char choice_buf[8];

    // initialize modules (stubs for now)
    init_storage();   // declared in utils.h / utils.c (we'll implement later)
    while (1) {
        menu();
        safe_input(choice_buf, sizeof(choice_buf));
        if (strlen(choice_buf) == 0) continue;

        if (strcmp(choice_buf, "1") == 0) {
            puts("[stub] Create user - will be implemented in commit 2.");
            // create_user();  <-- implement in commit 2
        } else if (strcmp(choice_buf, "2") == 0) {
            puts("[stub] Login - will be implemented in commit 3.");
            // login();  <-- implement in commit 3
        } else if (strcmp(choice_buf, "3") == 0) {
            puts("Exiting, bye.");
            break;
        } else {
            puts("Invalid choice.");
        }
    }

    shutdown_storage(); // cleanup if needed
    return 0;
}
