// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/utils.h"

void menu() {
    puts("\n--- Secure MFA Authentication System ---");
    puts("1. Create User");
    puts("2. Login (MFA)");
    puts("3. Exit");
    printf("Choice: ");
}

/************* MFA LOGIN (Commit 4 with Logging) *************/
void login() {
    char username[USERNAME_LEN];
    char password[USERNAME_LEN];
    char otp_buf[16];
    int otp;

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
        log_event("FAILED PASSWORD ATTEMPT", username);
        return;
    }

    // OTP creation
    otp = rand() % 900000 + 100000;
    printf("\n📩 Your OTP is: %d\n", otp);
    log_event("OTP GENERATED", username);

    int attempts = 3;

    while (attempts > 0) {
        printf("Enter OTP: ");
        safe_input(otp_buf, sizeof(otp_buf));

        if (atoi(otp_buf) == otp) {
            printf("✅ Login Successful! Welcome, %s.\n", username);
            log_event("LOGIN SUCCESS", username);
            return;
        }

        attempts--;
        printf("❌ Wrong OTP. Attempts left: %d\n", attempts);
        log_event("FAILED OTP ATTEMPT", username);
    }

    u->isLocked = 1;
    printf("🚫 Too many failed attempts. Account locked.\n");
    log_event("ACCOUNT LOCKED (FAILED OTP ATTEMPTS)", username);
}

/************* MAIN *************/
int main() {
    srand((unsigned)time(NULL));
    init_storage();

    char choice[8];

    while (1) {
        menu();
        safe_input(choice, sizeof(choice));

        if (strcmp(choice, "1") == 0)
            create_user();
        else if (strcmp(choice, "2") == 0)
            login();
        else if (strcmp(choice, "3") == 0) {
            printf("Goodbye!\n");
            shutdown_storage();
            break;
        } else
            printf("Invalid option.\n");
    }

    return 0;
}
