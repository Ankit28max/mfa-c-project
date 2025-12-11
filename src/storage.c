// src/storage.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/utils.h"

#define MAX_USERS 20

static User users[MAX_USERS];
static int userCount = 0;

/************* SAFE HASHING (djb2) *************/
void secure_hash(const char *input, char *out) {
    unsigned long hash = 5381;
    int c;
    while ((c = *input++))
        hash = ((hash << 5) + hash) + c;

    snprintf(out, PASSWORD_HASH_LEN, "%lu", hash);
}

/************* SAFE INPUT *************/
void safe_input(char *buf, size_t len) {
    if (!fgets(buf, (int)len, stdin)) {
        buf[0] = '\0';
        return;
    }
    size_t L = strnlen(buf, len);
    if (L > 0 && buf[L - 1] == '\n')
        buf[L - 1] = '\0';
    else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
    }
}

/************* STORAGE LOAD/SAVE *************/
void init_storage() {
    FILE *f = fopen("users.db", "rb");
    if (!f) {
        f = fopen("users.db", "wb");
        if (f) fclose(f);
        return;
    }

    fread(&userCount, sizeof(int), 1, f);
    fread(users, sizeof(User), MAX_USERS, f);
    fclose(f);
}

void save_storage() {
    FILE *f = fopen("users.db", "wb");
    if (!f) return;
    fwrite(&userCount, sizeof(int), 1, f);
    fwrite(users, sizeof(User), MAX_USERS, f);
    fclose(f);
}

void shutdown_storage() {
    save_storage();
}

/************* CREATE USER *************/
void create_user() {
    if (userCount >= MAX_USERS) {
        printf("❌ Maximum user limit reached.\n");
        return;
    }

    User u;
    char password[USERNAME_LEN];

    printf("Enter new username: ");
    safe_input(u.username, sizeof(u.username));

    // Check duplicate
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, u.username) == 0) {
            printf("❌ User already exists.\n");
            return;
        }
    }

    printf("Enter password: ");
    safe_input(password, sizeof(password));

    secure_hash(password, u.passwordHash);
    u.isLocked = 0;

    users[userCount++] = u;
    save_storage();

    printf("✅ User '%s' created successfully.\n", u.username);
}

/************* ACCESSORS *************/
User* find_user(const char *name) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, name) == 0)
            return &users[i];
    }
    return NULL;
}

int verify_password(User *u, const char *password) {
    char temp[PASSWORD_HASH_LEN];
    secure_hash(password, temp);
    return strcmp(temp, u->passwordHash) == 0;
}

/************* SECURITY LOGGING (Commit 4) *************/
void log_event(const char *event, const char *username) {
    FILE *f = fopen("security.log", "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(f,
        "[%02d-%02d-%04d %02d:%02d:%02d] USER: %s | EVENT: %s\n",
        t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
        t->tm_hour, t->tm_min, t->tm_sec,
        username ? username : "UNKNOWN",
        event
    );

    fclose(f);
}
