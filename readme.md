# 🔐 Secure Multi-Factor Authentication System (C, Terminal-Based)

A secure, terminal-driven **Multi-Factor Authentication (MFA)** system implemented in **C**, featuring:

- Password-based authentication  
- One-Time Password (OTP) verification  
- Brute-force protection  
- Account lockout  
- Secure file-based user storage  
- Timestamped security logging  
- Safe input handling to prevent buffer overflows  

This project is designed as part of an **Operating Systems coursework assignment**, demonstrating OS-level authentication flow, file handling, and secure coding practices.

---

## 🚀 Features

### ✅ **User Management**
- Create new users with validated usernames.
- Password hashing (djb2-based for assignment use).
- Persistent binary file storage (`users.db`).

### ✅ **Multi-Factor Authentication**
- Password + OTP based login.
- OTP is 6-digit numeric and generated securely.
- OTP can be **requested in advance (Option 4)**.
- OTP reused silently during login (not shown again).

### ✅ **Security Enhancements**
- Account lockout after repeated failed attempts.
- Brute-force protection for both password & OTP.
- Input hardening with `safe_input()` to prevent buffer overflow.
- Constant-time password verification to reduce timing attacks.

### ✅ **Audit Logging**
All actions are logged in:  
