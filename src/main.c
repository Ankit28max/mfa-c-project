// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "../include/utils.h"

/* In-memory OTP cache */
static long last_generated_otp = -1;
static char last_otp_user[USERNAME_LEN] = "";

/* Safe numeric parsing for OTP */
static long parse_otp(const char *s) {
    if (!s || s[0] == '\0') return -1;
    char *endptr = NULL;
    errno = 0;
    long v = strtol(s, &endptr, 10);

    if (errno != 0) return -1;
    if (endptr == s || *endptr != '\0') return -1;
    if (v < 100000 || v > 999999) return -1;

    return v;
}

/* Menu UI */
void menu(void) {
    puts("\n--- Secure MFA Authentication System (Commit 6.1) ---");
    puts("1. Create User");
    puts("2. Login (MFA)");
    puts("3. Exit");
    puts("4. Request OTP (pre-login)");
    printf("Choice: ");
}

/* REQUEST OTP BEFORE LOGIN (Option 4) */
void request_otp(void) {
    char username[USERNAME_LEN];

    printf("Enter username to request OTP: ");
    safe_input(username, sizeof(username));

    User *u = find_user(username);
    if (!u) {
        printf("❌ User does not exist.\n");
        log_event("OTP REQUEST FAILED - USER NOT FOUND", username);
        return;
    }

    if (u->isLocked) {
        printf("🚫 Account is locked; OTP cannot be issued.\n");
        log_event("OTP REQUEST FAILED - ACCOUNT LOCKED", username);
        return;
    }

    /* Generate OTP */
    long otp = rand() % 900000 + 100000;

    /* Cache for next login */
    last_generated_otp = otp;
    strncpy(last_otp_user, username, USERNAME_LEN);
    last_otp_user[USERNAME_LEN - 1] = '\0';

    printf("📩 OTP for %s is: %ld\n", username, otp);

    log_event("OTP REQUEST SUCCESSFUL (OTP NOT LOGGED)", username);
}

/* MFA LOGIN */
void login(void) {
    char username[USERNAME_LEN];
    char password[USERNAME_LEN];
    char otp_buf[16];

    printf("Enter username: ");
    safe_input(username, sizeof(username));

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

    if (!verify_password(u, password)) {
        printf("❌ Wrong password.\n");
        record_failed_password(username);
        return;
    }

    /* Password correct → reset counters */
    reset_failed_counters(username);

    long otp;

    /* CASE 1 — OTP was requested earlier (Option 4) */
    if (strcmp(last_otp_user, username) == 0 && last_generated_otp != -1) {
        otp = last_generated_otp;
        printf("\n📩 OTP already sent earlier. Please enter the OTP.\n");
    }
    /* CASE 2 — No prior OTP request → generate new OTP */
    else {
        otp = rand() % 900000 + 100000;
        printf("\n📩 Your OTP is: %ld\n", otp);
    }

    log_event("OTP GENERATED OR REUSED", username);

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

            /* Clear OTP cache */
            last_generated_otp = -1;
            last_otp_user[0] = '\0';
            return;
        }

        attempts_left--;
        printf("❌ Wrong OTP. Attempts left: %d\n", attempts_left);
        record_failed_otp(username);
    }

    printf("🚫 Too many failed OTP attempts.\n");

    /* Clear OTP cache */
    last_generated_otp = -1;
    last_otp_user[0] = '\0';
}

/* MAIN LOOP */
int main(void) {
    srand((unsigned)(time(NULL) ^ (uintptr_t)&main));

    init_storage();

    char choice[8];

    while (1) {
        menu();
        safe_input(choice, sizeof(choice));

        if (strcmp(choice, "1") == 0) {
            create_user();
        }
        else if (strcmp(choice, "2") == 0) {
            login();
        }
        else if (strcmp(choice, "3") == 0) {
            printf("Goodbye.\n");
            shutdown_storage();
            break;
        }
        else if (strcmp(choice, "4") == 0) {
            request_otp();
        }
        else {
            printf("❌ Invalid option.\n");
        }
    }

    return 0;
}
