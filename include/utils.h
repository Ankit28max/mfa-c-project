// include/utils.h
#ifndef MFA_UTILS_H
#define MFA_UTILS_H

#include <stddef.h>

#define USERNAME_LEN 48
#define PASSWORD_HASH_LEN 64

typedef struct {
    char username[USERNAME_LEN];
    char passwordHash[PASSWORD_HASH_LEN];
    int isLocked;
} User;

/* Storage & lifecycle */
void init_storage(void);
void save_storage(void);
void shutdown_storage(void);
void create_user(void);

/* Helpers */
void safe_input(char *buf, size_t len);

/* User access */
User* find_user(const char *name);
int verify_password(User *u, const char *password);

/* Logging */
void log_event(const char *event, const char *username);

/* MFA / login */
void login(void);

/* Brute-force counters (in-memory, non-persistent) */
void record_failed_password(const char *username);
void record_failed_otp(const char *username);
void reset_failed_counters(const char *username);
int get_failed_passwords(const char *username);
int get_failed_otps(const char *username);

#endif
