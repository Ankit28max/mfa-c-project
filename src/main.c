// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/utils.h"

void menu() {
    puts("\n--- Secure MFA Authentication System (terminal) ---");
    puts("1. Create user");
    puts("2. Login");
    puts("3. Exit");
    printf("Choice: ");
}

/************* MFA LOGIN FUNCTION *************/
void login() {
    char username[USERNAME_LEN];
    char password[USERNAME_LEN];
    char otp_buf[16];
    int generatedOTP;

    printf("Enter username: ");
    safe_input(username, sizeof(username));

    User *u = find_user(username);

    if (u == NULL) {
        printf("❌ User does not exist.\n");
        return;
    }

    if (u->isLocked) {
        printf("🚫 Account is locked due to too many failures.\n");
        return;
    }

    printf("Enter password: ");
    safe_input(password, sizeof(password));

    if (!verify_password(u, password)) {
        printf("❌ Incorrect password.\n");
        return;
    }

    generatedOTP = rand() % 900000 + 100000;
    printf("\n📩 Your OTP is: %d\n", generatedOTP);

    int attempts = 3;

    while (attempts > 0) {
        printf("Enter OTP: ");
        safe_input(otp_buf, sizeof(otp_buf));

        if (atoi(otp_buf) == generatedOTP) {
            printf("✅ Login successful! Welcome, %s.\n", username);
            return;
        }

        attempts--;
        printf("❌ Wrong OTP. Attempts left: %d\n", attempts);
    }

    printf("🚫 OTP attempts exceeded. Account locked.\n");
    u->isLocked = 1;
}

/************* MAIN FUNCTION *************/
int main(void) {
    srand((unsigned)time(NULL));

    init_storage();

    char choice_buf[8];

    while (1) {
        menu();
        safe_input(choice_buf, sizeof(choice_buf));

        if (strcmp(choice_buf, "1") == 0) {
            create_user();
        } 
        else if (strcmp(choice_buf, "2") == 0) {
            login();
        } 
        else if (strcmp(choice_buf, "3") == 0) {
            printf("Exiting...\n");
            shutdown_storage();
            break;
        } 
        else {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}
