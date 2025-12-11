// src/storage.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/utils.h"

#define MAX_USERS 20

static User users[MAX_USERS];
static int userCount = 0;

/************* SAFE HASHING (simple djb2 hash) *************/
void secure_hash(const char *input, char *out) {
    unsigned long hash = 5381;
    int c;
    while ((c = *input++)) {
        hash = ((hash << 5) + hash) + c;
    }
    snprintf(out, PASSWORD_HASH_LEN, "%lu", hash);
}

/************* SAFE INPUT HELPER *************/
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
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }
}

/************* FILE HANDLING *************/
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

/************* USER CREATION *************/
void create_user() {
    if (userCount >= MAX_USERS) {
        printf("❌ Max user limit reached.\n");
        return;
    }

    User u;
    char password[USERNAME_LEN];

    printf("Enter new username: ");
    safe_input(u.username, sizeof(u.username));

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

/************* ACCESSOR FUNCTIONS *************/
User* find_user(const char *name) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, name) == 0) {
            return &users[i];
        }
    }
    return NULL;
}

int verify_password(User *u, const char *password) {
    char hash[PASSWORD_HASH_LEN];
    secure_hash(password, hash);
    return strcmp(hash, u->passwordHash) == 0;
}
