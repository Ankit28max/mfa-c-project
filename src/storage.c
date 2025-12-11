// src/storage.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/utils.h"

#define MAX_USERS 20
#define PW_MIN_LEN 4
#define PW_MAX_LEN (USERNAME_LEN-1)
#define PASSWORD_FAIL_THRESHOLD 3
#define OTP_FAIL_THRESHOLD 3

static User users[MAX_USERS];
static int userCount = 0;

/* In-memory brute-force counters keyed by user index; non-persistent */
static int failed_passwords[MAX_USERS];
static int failed_otps[MAX_USERS];

static int find_index_by_name(const char *name) {
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(users[i].username, name) == 0) return i;
    }
    return -1;
}

/************* SAFE HASHING (djb2) *************/
void secure_hash(const char *input, char *out) {
    unsigned long hash = 5381;
    int c;
    while ((c = *input++)) hash = ((hash << 5) + hash) + c;
    /* store as decimal string */
    snprintf(out, PASSWORD_HASH_LEN, "%lu", hash);
}

/************* CONSTANT-TIME COMPARE *************/
static int constant_time_cmp(const char *a, const char *b) {
    /* returns 1 if equal, 0 otherwise */
    size_t la = strlen(a);
    size_t lb = strlen(b);
    if (la != lb) return 0;
    unsigned char result = 0;
    for (size_t i = 0; i < la; ++i) result |= (unsigned char)(a[i] ^ b[i]);
    return result == 0;
}

/************* SAFE INPUT: trims and bounds checks *************/
void safe_input(char *buf, size_t len) {
    if (!buf || len == 0) return;
    if (!fgets(buf, (int)len, stdin)) {
        buf[0] = '\0';
        return;
    }

    /* remove trailing newline if present */
    size_t L = strnlen(buf, len);
    if (L > 0 && buf[L - 1] == '\n') {
        buf[L - 1] = '\0';
        L--;
    } else {
        /* if input longer than buffer, flush remainder */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
    }

    /* trim leading spaces */
    size_t start = 0;
    while (buf[start] && (buf[start] == ' ' || buf[start] == '\t' || buf[start] == '\r')) start++;

    /* trim trailing spaces */
    size_t end = (L == 0 ? 0 : L - 1);
    while (end > start && (buf[end] == ' ' || buf[end] == '\t' || buf[end] == '\r')) end--;

    /* shift contents left */
    size_t outlen = 0;
    for (size_t i = start; i <= end && outlen+1 < len; ++i) {
        buf[outlen++] = buf[i];
    }
    buf[outlen] = '\0';
}

/************* STORAGE LOAD/SAVE *************/
void init_storage() {
    /* initialize counters */
    for (int i = 0; i < MAX_USERS; ++i) {
        failed_passwords[i] = 0;
        failed_otps[i] = 0;
    }

    FILE *f = fopen("users.db", "rb");
    if (!f) {
        /* create blank db */
        f = fopen("users.db", "wb");
        if (f) fclose(f);
        userCount = 0;
        return;
    }

    /* read userCount then user array */
    if (fread(&userCount, sizeof(int), 1, f) != 1) {
        /* empty or corrupted file */
        userCount = 0;
        fclose(f);
        return;
    }
    if (userCount < 0 || userCount > MAX_USERS) userCount = 0;

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

/************* USER CREATION WITH VALIDATION *************/
static int is_username_valid(const char *name) {
    if (!name || name[0] == '\0') return 0;
    /* no spaces allowed; minimal length 1 */
    size_t L = strlen(name);
    if (L == 0 || L >= USERNAME_LEN) return 0;
    for (size_t i = 0; i < L; ++i) {
        if (name[i] == ' ' || name[i] == '\t' || name[i] == '\r' || name[i] == '\n')
            return 0;
    }
    return 1;
}

void create_user() {
    if (userCount >= MAX_USERS) {
        printf("❌ Maximum user limit reached.\n");
        return;
    }

    User u;
    char password[PW_MAX_LEN + 1];

    printf("Enter new username (no spaces, max %d chars): ", USERNAME_LEN - 1);
    safe_input(u.username, sizeof(u.username));
    if (!is_username_valid(u.username)) {
        printf("❌ Invalid username. No spaces, non-empty, under %d chars.\n", USERNAME_LEN);
        return;
    }

    /* ensure user not present */
    if (find_index_by_name(u.username) != -1) {
        printf("❌ User already exists.\n");
        return;
    }

    printf("Enter password (min %d chars): ", PW_MIN_LEN);
    safe_input(password, sizeof(password));
    size_t plen = strlen(password);
    if (plen < PW_MIN_LEN) {
        printf("❌ Password too short. Must be at least %d characters.\n", PW_MIN_LEN);
        return;
    }
    if (plen > PW_MAX_LEN) {
        printf("❌ Password too long.\n");
        return;
    }

    /* hash & store */
    secure_hash(password, u.passwordHash);
    u.isLocked = 0;

    users[userCount++] = u;
    save_storage();

    /* initialize counters for this index */
    int idx = find_index_by_name(u.username);
    if (idx >= 0 && idx < MAX_USERS) {
        failed_passwords[idx] = 0;
        failed_otps[idx] = 0;
    }

    printf("✅ User '%s' created successfully.\n", u.username);
}

/************* ACCESSORS *************/
User* find_user(const char *name) {
    int idx = find_index_by_name(name);
    if (idx >= 0) return &users[idx];
    return NULL;
}

int verify_password(User *u, const char *password) {
    if (!u || !password) return 0;
    char computed[PASSWORD_HASH_LEN];
    secure_hash(password, computed);
    return constant_time_cmp(computed, u->passwordHash);
}

/************* IN-MEMORY BRUTE-FORCE COUNTERS *************/
void record_failed_password(const char *username) {
    int idx = find_index_by_name(username);
    if (idx < 0) return;
    if (++failed_passwords[idx] >= PASSWORD_FAIL_THRESHOLD) {
        users[idx].isLocked = 1;
        log_event("ACCOUNT LOCKED (FAILED PASSWORD THRESHOLD)", username);
        save_storage();
    } else {
        log_event("FAILED PASSWORD ATTEMPT", username);
    }
}

void record_failed_otp(const char *username) {
    int idx = find_index_by_name(username);
    if (idx < 0) return;
    if (++failed_otps[idx] >= OTP_FAIL_THRESHOLD) {
        users[idx].isLocked = 1;
        log_event("ACCOUNT LOCKED (FAILED OTP THRESHOLD)", username);
        save_storage();
    } else {
        log_event("FAILED OTP ATTEMPT", username);
    }
}

void reset_failed_counters(const char *username) {
    int idx = find_index_by_name(username);
    if (idx < 0) return;
    failed_passwords[idx] = 0;
    failed_otps[idx] = 0;
}

int get_failed_passwords(const char *username) {
    int idx = find_index_by_name(username);
    if (idx < 0) return 0;
    return failed_passwords[idx];
}

int get_failed_otps(const char *username) {
    int idx = find_index_by_name(username);
    if (idx < 0) return 0;
    return failed_otps[idx];
}

/************* SECURITY LOGGING (KEEP OTP VALUES OUT) *************/
void log_event(const char *event, const char *username) {
    FILE *f = fopen("security.log", "a");
    if (!f) return;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(f, "[%02d-%02d-%04d %02d:%02d:%02d] USER: %s | EVENT: %s\n",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec,
            username ? username : "UNKNOWN",
            event ? event : "UNKNOWN");
    fclose(f);
}
