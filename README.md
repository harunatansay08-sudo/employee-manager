# Employee & Data Management System (C)

A menu-driven console project written in C, demonstrating functions, structures, arrays, validated numeric input, CRUD operations, and local file persistence.

## Build and run

```bash
gcc -std=c11 -Wall -Wextra -pedantic employee_manager.c -o employee_manager
./employee_manager
```

Demo credentials: `admin` / `1234`.

## Features

- Employee records: list, add, search by ID, rename, delete.
- Numeric data: list, add, edit, delete.
- Save records to `management_data.txt` in the working directory; reload on startup.
- Menu loops until Exit is selected.
- Session-only password change.
- Invalid numerical input is rejected rather than leaving uninitialized values.

## Security notice

This is an **educational demo**, not production authentication. The demo password is hard-coded, entered visibly, and password changes are not persisted. Do not store real employee information or use this code to protect sensitive data. For production, use proper authentication, password hashing, permissions, secure storage, and tests.

## Limitations

- Up to 100 employee records and 100 numeric data items.
- Names limited to 63 bytes and cannot contain `|`.
- No concurrent writers or database transactions.
- Local file storage; file path is relative to the current working directory.
- If an existing save file is invalid, default sample records are loaded. Review backups before overwriting.
