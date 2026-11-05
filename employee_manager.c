/* Employee & Data Management System - educational console application.
 * Login is DEMO ONLY. Do not use for real security or sensitive records.
 * Build: gcc -std=c11 -Wall -Wextra -pedantic employee_manager.c -o employee_manager
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#define MAX_EMPLOYEES 100
#define MAX_DATA 100
#define NAME_SIZE 64
#define PATH "management_data.txt"

typedef struct { int id; char name[NAME_SIZE]; } Employee;
typedef struct {
    int data[MAX_DATA];
    size_t data_count;
    Employee employees[MAX_EMPLOYEES];
    size_t employee_count;
} Database;

static int read_line(char *buf, size_t cap) {
    int ch;
    if (!fgets(buf, (int)cap, stdin)) return 0;
    size_t n = strlen(buf);
    if (n && buf[n - 1] == '\n') { buf[n - 1] = '\0'; return 1; }
    while ((ch = getchar()) != '\n' && ch != EOF) {}
    return 1;
}
static int ask_int(const char *prompt, int *out) {
    char input[128], *end;
    long value;
    for (;;) {
        printf("%s", prompt);
        if (!read_line(input, sizeof(input))) return 0;
        errno = 0;
        value = strtol(input, &end, 10);
        if (end != input && errno != ERANGE && value >= INT_MIN && value <= INT_MAX) {
            while (*end == ' ' || *end == '\t') end++;
            if (*end == '\0') { *out = (int)value; return 1; }
        }
        puts("Invalid number. Try again.");
    }
}
static int ask_name(const char *prompt, char *out, size_t cap) {
    char buf[NAME_SIZE];
    for (;;) {
        printf("%s", prompt);
        if (!read_line(buf, sizeof(buf))) return 0;
        if (!buf[0] || strchr(buf, '|')) { puts("Name cannot be empty or contain '|'."); continue; }
        snprintf(out, cap, "%s", buf);
        return 1;
    }
}
static void defaults(Database *db) {
    const int data[] = {1,2,3,4,5,6,7,8,9,10};
    const int ids[] = {1001,1002,1003,1004,1005};
    memset(db, 0, sizeof(*db));
    memcpy(db->data, data, sizeof(data)); db->data_count = sizeof(data)/sizeof(data[0]);
    db->employee_count = sizeof(ids)/sizeof(ids[0]);
    for (size_t i = 0; i < db->employee_count; ++i) {
        db->employees[i].id = ids[i];
        snprintf(db->employees[i].name, NAME_SIZE, "Employee %d", ids[i]);
    }
}
static int save_db(const Database *db) {
    FILE *f = fopen(PATH ".tmp", "w");
    if (!f) { perror("Cannot save data"); return 0; }
    int ok = fprintf(f, "EMS1\n%zu\n", db->data_count) > 0;
    for (size_t i = 0; i < db->data_count; ++i)
        if (fprintf(f, "%d\n", db->data[i]) < 0) ok = 0;
    if (fprintf(f, "%zu\n", db->employee_count) < 0) ok = 0;
    for (size_t i = 0; i < db->employee_count; ++i)
        if (fprintf(f, "%d|%s\n", db->employees[i].id, db->employees[i].name) < 0) ok = 0;
    if (fclose(f) != 0) ok = 0;
    if (!ok || rename(PATH ".tmp", PATH) != 0) {
        perror("Cannot complete save"); remove(PATH ".tmp"); return 0;
    }
    return 1;
}
static int load_db(Database *db) {
    FILE *f = fopen(PATH, "r");
    if (!f) return 0;
    Database *tmp = calloc(1, sizeof(*tmp));
    if (!tmp) { fclose(f); return 0; }
    char line[256]; size_t count;
    int ok = fgets(line, sizeof(line), f) && strcmp(line, "EMS1\n") == 0;
    if (ok) ok = fscanf(f, "%zu", &count) == 1 && count <= MAX_DATA;
    if (ok) tmp->data_count = count;
    for (size_t i = 0; ok && i < tmp->data_count; ++i)
        ok = fscanf(f, "%d", &tmp->data[i]) == 1;
    if (ok) ok = fscanf(f, "%zu", &count) == 1 && count <= MAX_EMPLOYEES;
    if (ok) tmp->employee_count = count;
    for (size_t i = 0; ok && i < tmp->employee_count; ++i) {
        int id; char name[NAME_SIZE];
        ok = fscanf(f, " %d|%63[^\n]", &id, name) == 2 && id > 0 && !strchr(name, '|');
        if (ok) {
            tmp->employees[i].id = id;
            snprintf(tmp->employees[i].name, NAME_SIZE, "%s", name);
            for (size_t j = 0; j < i; ++j)
                if (tmp->employees[j].id == id) ok = 0;
        }
    }
    fclose(f);
    if (ok) *db = *tmp;
    free(tmp);
    return ok;
}
static int find_employee(const Database *db, int id) {
    for (size_t i = 0; i < db->employee_count; ++i)
        if (db->employees[i].id == id) return (int)i;
    return -1;
}
static void list_employees(const Database *db) {
    printf("\nEMPLOYEES (%zu)\n", db->employee_count);
    if (!db->employee_count) puts("No employees.");
    for (size_t i = 0; i < db->employee_count; ++i)
        printf("ID: %d | Name: %s\n", db->employees[i].id, db->employees[i].name);
}
static int manage_employees(Database *db) {
    int option, id, index;
    for (;;) {
        puts("\n--- EMPLOYEE MANAGEMENT ---\n1. List\n2. Add\n3. Search\n4. Update name\n5. Delete\n0. Back");
        if (!ask_int("Choice: ", &option)) return 0;
        switch (option) {
            case 0: return 1;
            case 1: list_employees(db); break;
            case 2:
                if (db->employee_count == MAX_EMPLOYEES) { puts("Employee limit reached."); break; }
                if (!ask_int("New ID: ", &id)) return 0;
                if (id <= 0 || find_employee(db, id) >= 0) { puts("ID must be positive and unique."); break; }
                if (!ask_name("Name: ", db->employees[db->employee_count].name, NAME_SIZE)) return 0;
                db->employees[db->employee_count++].id = id;
                puts(save_db(db) ? "Employee added and saved." : "Employee added, but save failed.");
                break;
            case 3:
                if (!ask_int("Search ID: ", &id)) return 0;
                index = find_employee(db, id);
                if (index < 0) puts("Not found.");
                else printf("Found: %d | %s\n", db->employees[index].id, db->employees[index].name);
                break;
            case 4:
                if (!ask_int("ID to update: ", &id)) return 0;
                index = find_employee(db, id);
                if (index < 0) { puts("Not found."); break; }
                if (!ask_name("New name: ", db->employees[index].name, NAME_SIZE)) return 0;
                puts(save_db(db) ? "Employee updated and saved." : "Employee updated, but save failed.");
                break;
            case 5:
                if (!ask_int("ID to delete: ", &id)) return 0;
                index = find_employee(db, id);
                if (index < 0) { puts("Not found."); break; }
                for (size_t i = (size_t)index; i + 1 < db->employee_count; ++i)
                    db->employees[i] = db->employees[i + 1];
                db->employee_count--;
                puts(save_db(db) ? "Employee deleted and saved." : "Employee deleted, but save failed.");
                break;
            default: puts("Invalid option.");
        }
    }
}
static int manage_data(Database *db) {
    int choice, value, position;
    for (;;) {
        puts("\n--- DATA MANAGEMENT ---\n1. List\n2. Edit value\n3. Add value\n4. Delete value\n0. Back");
        if (!ask_int("Choice: ", &choice)) return 0;
        switch (choice) {
            case 0: return 1;
            case 1:
                printf("Data (%zu): ", db->data_count);
                for (size_t i = 0; i < db->data_count; ++i) printf("[%zu]=%d ", i + 1, db->data[i]);
                puts(""); break;
            case 2:
            case 4:
                if (!ask_int("Position (starting at 1): ", &position)) return 0;
                if (position <= 0 || (size_t)position > db->data_count) { puts("Invalid position."); break; }
                if (choice == 2) {
                    if (!ask_int("New value: ", &value)) return 0;
                    db->data[position - 1] = value;
                } else {
                    for (size_t i = (size_t)(position - 1); i + 1 < db->data_count; ++i)
                        db->data[i] = db->data[i + 1];
                    db->data_count--;
                }
                puts(save_db(db) ? "Changes saved." : "Change made, but save failed.");
                break;
            case 3:
                if (db->data_count == MAX_DATA) { puts("Data limit reached."); break; }
                if (!ask_int("New value: ", &value)) return 0;
                db->data[db->data_count++] = value;
                puts(save_db(db) ? "Value added and saved." : "Value added, but save failed.");
                break;
            default: puts("Invalid option.");
        }
    }
}
static int manage_security(char *password, size_t cap) {
    char old[64], next[64], confirm[64];
    puts("\n--- SECURITY (DEMO ONLY) ---");
    printf("Current password: ");
    if (!read_line(old, sizeof(old))) return 0;
    if (strcmp(old, password)) { puts("Incorrect password."); return 1; }
    printf("New password (4-63 characters): ");
    if (!read_line(next, sizeof(next))) return 0;
    if (strlen(next) < 4) { puts("Password too short."); return 1; }
    printf("Confirm password: ");
    if (!read_line(confirm, sizeof(confirm))) return 0;
    if (strcmp(next, confirm)) { puts("Passwords do not match."); return 1; }
    snprintf(password, cap, "%s", next);
    puts("Password changed for this run only (not saved to disk).");
    return 1;
}
int main(void) {
    Database db;
    char username[64], password[64] = "1234", entered[64];
    int choice;
    if (load_db(&db)) puts("Saved records loaded.");
    else { defaults(&db); puts("Starting with sample records."); }
    puts("=== EMPLOYEE & DATA MANAGER ===");
    puts("DEMO LOGIN ONLY -- NOT FOR REAL SECURITY");
    printf("Username: ");
    if (!read_line(username, sizeof(username))) return 1;
    printf("Password: ");
    if (!read_line(entered, sizeof(entered))) return 1;
    if (strcmp(username, "admin") || strcmp(entered, password)) { puts("Access denied."); return 1; }
    puts("Access granted.");
    for (;;) {
        puts("\n========== MAIN MENU ==========\n1. Data\n2. Employees\n3. Security\n4. Save now\n0. Exit");
        if (!ask_int("Choice: ", &choice)) { puts("Input ended."); break; }
        switch (choice) {
            case 0: puts("Goodbye!"); return 0;
            case 1: if (!manage_data(&db)) return 0; break;
            case 2: if (!manage_employees(&db)) return 0; break;
            case 3: if (!manage_security(password, sizeof(password))) return 0; break;
            case 4: puts(save_db(&db) ? "Saved." : "Save failed."); break;
            default: puts("Invalid choice.");
        }
    }
    return 0;
}
