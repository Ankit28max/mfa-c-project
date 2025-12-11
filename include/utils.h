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

// storage functions
void init_storage(void);
void shutdown_storage(void);
void create_user(void);

// helper functions
User* find_user(const char *name);
int verify_password(User *u, const char *password);
void safe_input(char *buf, size_t len);

// login function
void login(void);

#endif
