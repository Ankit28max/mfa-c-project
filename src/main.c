// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/utils.h"

/* Helper: parse OTP safely using strtol; returns -1 on parse error */
static long parse_otp(const char *s) {
    if (!s || s[0] == '\0') return -1;
    char *endptr = NULL;
    errno = 0;
    long v = strtol(s, &endptr, 10);
    if (errno != 0) return -1;
    if (endptr == s || *endptr != '\0') return -1; /* extra chars */
    if (v < 100000L || v > 999999L) return -1;
    return v;
}

void menu(void) {
    puts("\n--- Secure MFA Authentication System ---");
    puts("1. Create User");
    puts("2. Login (MFA)");
    puts("3. Exit");
    printf("Choice: ");
}

/* Improved login flow using storage's brute-force counters and safe parsing */
void login(void) {
    char username[USERNAME_LEN];
    char password[USERNAME_LEN];
    char otp_buf[16];

    printf("Enter username: ");
    safe_input(username, sizeof(username));
    if (username[0] == '\0') {
        printf("❌ Username cannot be empty.\n");
        return;
    }

    User *u = find_user(username);
    if (!u) {
        printf("❌ User does not exist.\n");
        log_event("FAILED LOGIN - USER NOT FOUND", username);
        return;
    }

    if (u->isLocked) {
        printf("🚫 Account is locked.\n");
        log_event("LOGIN ATTEMPT ON LOCKED ACCOUNT", username);
        return;
    }

    printf("Enter password: ");
    safe_input(password, sizeof(password));
    if (password[0] == '\0') {
        printf("❌ Password cannot be empty.\n");
        return;
    }

    if (!verify_password(u, password)) {
        printf("❌ Wrong password.\n");
        /* increment in-memory counter and possibly lock */
        record_failed_password(username);
        return;
    }

    /* OK password: reset counters and proceed to OTP */
    reset_failed_counters(username);

    /* generate OTP - non-cryptographic but varied seed; do NOT log OTP value */
    long otp = (long)(rand() % 900000 + 100000);
    printf("\n📩 Your OTP is: %ld\n", otp);
    log_event("OTP GENERATED", username);

    int attempts_left = 3;
    while (attempts_left > 0) {
        printf("Enter OTP: ");
        safe_input(otp_buf, sizeof(otp_buf));
        long parsed = parse_otp(otp_buf);
        if (parsed == -1) {
            printf("❌ Invalid OTP format. Attempts left: %d\n", attempts_left - 1);
            record_failed_otp(username);
            attempts_left--;
            continue;
        }

        if (parsed == otp) {
            printf("✅ Login successful! Welcome, %s.\n", username);
            log_event("LOGIN SUCCESS", username);
            reset_failed_counters(username);
            return;
        }

        attempts_left--;
        printf("❌ Wrong OTP. Attempts left: %d\n", attempts_left);
        record_failed_otp(username);
    }

    /* if loop exits -> locked or attempted threshold reached */
    if (u->isLocked) {
        printf("🚫 Too many failed attempts. Account locked.\n");
    } else {
        printf("🚫 OTP attempts exhausted. Account may be temporarily restricted.\n");
    }
}

int main(void) {
    /* seed random using time + address for more entropy */
    srand((unsigned)(time(NULL) ^ (uintptr_t)&main));

    init_storage();

    char choice[8];
    while (1) {
        menu();
        safe_input(choice, sizeof(choice));
        if (strcmp(choice, "1") == 0) {
            create_user();
        } else if (strcmp(choice, "2") == 0) {
            login();
        } else if (strcmp(choice, "3") == 0) {
            printf("Goodbye.\n");
            shutdown_storage();
            break;
        } else {
            printf("Invalid option.\n");
        }
    }

    return 0;
}
