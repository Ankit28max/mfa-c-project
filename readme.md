📘 Secure Multi-Factor Authentication System (C, Terminal-Based)
Operating Systems Course Project — MFA Module with Security Hardening

This project implements a secure Multi-Factor Authentication (MFA) system in C with:

Secure password hashing

OTP (One-Time Password) authentication

User account lockout

Security event logging

Buffer overflow protection

Brute-force defense

Menu-driven terminal UI

Designed for academic demonstration of process security, memory safety, file handling, and OS-level authentication logic.

🏗 Project Structure
mfa-c-project/
│
├── include/
│   └── utils.h
│
├── src/
│   ├── main.c          ← UI + MFA + OTP + menu
│   └── storage.c       ← Hashing + persistence + logging + brute force defense
│
├── users.db            ← Binary storage (auto created)
├── security.log        ← Human-readable event log
├── Makefile
└── README.md

🔐 Features Implemented
✔ User Creation

Username validation (no spaces, length-limited)

Password validation (min length, safe input)

Password hashing using djb2

Stored in users.db safely

✔ Login System

Username verification

Password verification (constant-time compare)

OTP verification

Secure error handling (no info leaks)

✔ OTP System

Two ways to get OTP:

Option 4 — Request OTP (pre-login)

Option 2 — Login (OTP auto-generated)

Cached OTP is used once and then deleted.

✔ Account Lockout

Account locks when:

3 wrong passwords

3 wrong OTP attempts

Lock state is saved in database.

✔ Security Logging

Written to security.log with timestamp:

Failed password

Failed OTP

Account lock

OTP request

Login success

⚠ No OTP values are stored in logs.

✔ Memory Safety

Safe input handling (safe_input())

No buffer overflow

All arrays length-bounded

No unsafe functions (gets, scanf("%s"), etc.)

Safe numeric parsing with strtol()

🎯 How to Compile

Using GCC:

gcc src/main.c src/storage.c -Iinclude -o mfa


Using Makefile:

mingw32-make

🚀 How to Run
./mfa

🧠 MFA Authentication Flow
          ┌────────────┐
          │  Start App │
          └──────┬─────┘
                 │
                 ▼
        ┌────────────────────┐
        │ User selects Login │
        └─────────┬──────────┘
                  │
                  ▼
      ┌──────────────────────────┐
      │ Enter username/password  │
      └───────────┬─────────────┘
                  │
              Correct?
             ┌───┴────┐
             │  Yes    │ No
             ▼         ▼
         ┌────────┐  Log fail
         │ Generate/Reuse OTP │
         └───────┬───────────┘
                 │
                 ▼
         ┌─────────────────┐
         │ Enter OTP       │
         └───────┬─────────┘
                 │
             Correct?
           ┌───┴────┐
           │  Yes    │ No
           ▼         ▼
   ┌─────────────────────┐
   │ LOGIN SUCCESS       │
   └─────────────────────┘


🛡 Security Mechanisms Summary
🔸 1. Hash-Based Authentication

Passwords are never stored in plain text.

🔸 2. OTP Verification

Adds a second factor of authentication.

🔸 3. Brute Force Prevention

3 failed passwords → lock
3 failed OTPs → lock

🔸 4. Permanent Lockout Persistence

Lock saved in database → survives restarts.

🔸 5. Safe Input + Bound Checks

Prevents buffer overflow attacks.

🔸 6. Timestamped Security Log

Tracks all critical events.

🔸 7. Constant-Time Hash Compare

Protects against timing attacks.

🧾 Event Log Example

security.log contains entries like:

[11-12-2025 10:22:03] USER: ankit | EVENT: FAILED PASSWORD ATTEMPT
[11-12-2025 10:22:04] USER: ankit | EVENT: OTP REQUEST SUCCESSFUL (OTP NOT LOGGED)
[11-12-2025 10:22:12] USER: ankit | EVENT: LOGIN SUCCESS

🔬 Testing Scenarios
Scenario	Expected Behavior
Wrong username	Show error + log event
Wrong password	3 attempts → lock
Wrong OTP	3 attempts → lock
Request OTP then login	OTP reused silently
Login without request	OTP printed
Locked account	Cannot login or request OTP