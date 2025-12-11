// include/utils.h
#ifndef MFA_UTILS_H
#define MFA_UTILS_H

#define USERNAME_LEN 48
#define PASSWORD_HASH_LEN 64

typedef struct {
    char username[USERNAME_LEN];
    char passwordHash[PASSWORD_HASH_LEN];
    int isLocked;
} User;

// Storage
void init_storage(void);
void save_storage(void);
void shutdown_storage(void);
void create_user(void);

// Helpers
void safe_input(char *buf, size_t len);
User* find_user(const char *name);
int verify_password(User *u, const char *password);

// Logging
void log_event(const char *event, const char *username);

// Login (MFA)
void login(void);

#endif
