#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include "../include/operations.h"

#define PORT 8080
#define MAX_USERS 100

typedef struct {
    int userID;
    int active;
} Session;

Session sessions[MAX_USERS];
pthread_mutex_t session_lock = PTHREAD_MUTEX_INITIALIZER;

int server_running = 1;
int server_fd;


int logged_in_user(int userID) {
    pthread_mutex_lock(&session_lock);
    for (int i = 0; i < MAX_USERS; i++) {
        if (sessions[i].active && sessions[i].userID == userID) {
            pthread_mutex_unlock(&session_lock);
            return 1;
        }
    }
    pthread_mutex_unlock(&session_lock);
    return 0;
}

void add_session(int userID) {
    pthread_mutex_lock(&session_lock);
    for (int i = 0; i < MAX_USERS; i++) {
        if (!sessions[i].active) {
            sessions[i].active = 1;
            sessions[i].userID = userID;
            break;
        }
    }
    pthread_mutex_unlock(&session_lock);
}

void remove_session(int userID) {
    pthread_mutex_lock(&session_lock);
    for (int i = 0; i < MAX_USERS; i++) {
        if (sessions[i].active && sessions[i].userID == userID) {
            sessions[i].active = 0;
            sessions[i].userID = 0;
            break;
        }
    }
    pthread_mutex_unlock(&session_lock);
}


void *server_command_listener(void *arg) {
    char cmd[32];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(cmd, sizeof(cmd), stdin)) {
            cmd[strcspn(cmd, "\n")] = 0;
            if (strcasecmp(cmd, "exit") == 0) {
                printf("Server shut down\n");
                server_running = 0;
                shutdown(server_fd, SHUT_RDWR);
                close(server_fd);
                break;
            }
        }
    }
    return NULL;
}

void handle_sigint(int sig) {
    printf("\nCaught Ctrl+C kill\n");
    server_running = 0;
    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);
    exit(0);
}


void *handle_client(void *arg) {
    int sock = *(int *)arg;
    free(arg);

    char id[32], pwd[32], role[32];
    int choice, status;
    char buf[4096];
    double amount;

    // LOGIN
    read(sock, id, sizeof(id));
    read(sock, pwd, sizeof(pwd));

    int login_status = auth(id, pwd, role);

    if (login_status == 2) {  
        write(sock, "Inactive", sizeof("Inactive"));
        close(sock);
        return NULL;
    } 
    else if (login_status != 1) {  
        write(sock, "Invalid", sizeof("Invalid"));
        close(sock);
        return NULL;
    }


    int userID = atoi(id);

    if (logged_in_user(userID)) {
        strcpy(role, "AlreadyLoggedIn");
        write(sock, role, strlen(role) + 1);
        printf("Duplicate login");
        close(sock);
        return NULL;
    }

    add_session(userID);
    write(sock, role, strlen(role) + 1);
    printf("%s logged in (ID=%d)\n", role, userID);


    if (strcmp(role, "Customer") == 0) {
        while (read(sock, &choice, sizeof(choice)) > 0) {
            switch (choice) {
                case 1: { 
                    double bal;
                    if (show_balance(userID, &bal) == 0)
                        snprintf(buf, sizeof(buf), "Balance: %.2f", bal);
                    else
                        snprintf(buf, sizeof(buf), "Error reading balance.");
                    write(sock, buf, strlen(buf) + 1);
                    break;
                }
                case 2: { 
                    read(sock, &amount, sizeof(amount));
                    update_bal(userID, amount, 1);
                save_transaction(userID, "Deposit", amount);
                    write(sock, "Deposit successful.", strlen("Deposit successful.") + 1);
                    break;
                }
                case 3: { 
                    read(sock, &amount, sizeof(amount));
                    double bal;
                    show_balance(userID, &bal);
                    if (amount > bal)
                        write(sock, "Insufficient balance.", strlen("Insufficient balance.") + 1);
                    else {
                        update_bal(userID, amount, 0);
                    save_transaction(userID, "Withdraw", amount);
                        write(sock, "Withdrawal successful.", strlen("Withdrawal successful.") + 1);
                    }
                    break;
                }
                case 4: { 
                    int target;
                    read(sock, &target, sizeof(target));
                    read(sock, &amount, sizeof(amount));
                    double bal;
                    show_balance(userID, &bal);
                    if (amount > bal)
                        write(sock, "Insufficient balance.", strlen("Insufficient balance.") + 1);
                    else {
                        update_bal(userID, amount, 0);
                        update_bal(target, amount, 1);
                    save_transaction(userID, "TransferOut", amount);
                    save_transaction(target, "TransferIn", amount);
                        write(sock, "Transfer successful.", strlen("Transfer successful.") + 1);
                    }
                    break;
                }
                case 5: { 
                    read(sock, &amount, sizeof(amount));
                    apply_for_loan(userID, amount);
                    write(sock, "Loan applied successfully.", strlen("Loan applied successfully.") + 1);
                    break;
                }
                case 6: { 
                    char newpwd[64];
                    read(sock, newpwd, sizeof(newpwd));
                    int res = change_password(userID, newpwd);
                    snprintf(buf, sizeof(buf),
                             res == 0 ? "Password changed successfully."
                                      : "Failed to change password.");
                    write(sock, buf, strlen(buf) + 1);
                    break;
                }
                case 7: { 
                    char feedback[256];
                    read(sock, feedback, sizeof(feedback));
                    add_feedback(userID, feedback);
                    write(sock, "Feedback added.", strlen("Feedback added.") + 1);
                    break;
                }
                case 8: { 
                    view_history(userID, buf, sizeof(buf));
                    write(sock, buf, strlen(buf) + 1);
                    break;
                }
                case 9: 
                case 10:
                    printf("Customer disconnected.\n");
                    remove_session(userID);
                    close(sock);
                    return NULL;
            }
        }
    }

    
    else if (strcmp(role, "Employee") == 0) {
        while (read(sock, &choice, sizeof(choice)) > 0) {
            switch (choice) {
                case 1: { 
                    char password[64];
                    read(sock, password, sizeof(password));
                    int newID = add_customer(password);
                    snprintf(buf, sizeof(buf),
                             newID > 0 ? "New customer created successfully with ID: %d"
                                       : "Failed to create new customer.", newID);
                    write(sock, buf, strlen(buf) + 1);
                    break;
                }
                case 2: { 
                    int custID;
                    char newpwd[64];
                    read(sock, &custID, sizeof(custID));
                    read(sock, newpwd, sizeof(newpwd));
                    int res = modify_customer_details(custID, newpwd);
                    snprintf(buf, sizeof(buf),
                             res == 0 ? "Customer %d password updated successfully."
                                      : "Customer not found or update failed.", custID);
                    write(sock, buf, strlen(buf) + 1);
                    break;
                }
                case 3:
                    show_loans(userID, buf, sizeof(buf));
                    write(sock, buf, strlen(buf) + 1);
                    break;
                case 4: {
                    int cid;
                    char st[32];
                    read(sock, &cid, sizeof(cid));
                    read(sock, st, sizeof(st));
                    status = update_loan_status(cid, st);
                    write(sock, status == 0 ? "Loan status updated." : "Update failed.",
                          strlen(status == 0 ? "Loan status updated." : "Update failed.") + 1);
                    break;
                }
                case 5: {
                    int custID;
                    read(sock, &custID, sizeof(custID));
                    view_history(custID, buf, sizeof(buf));
                    write(sock, buf, strlen(buf) + 1);
                    break;
                }
                case 6: { 
                    char newpwd[64];
                    read(sock, newpwd, sizeof(newpwd));
                    int res = change_password(userID, newpwd);
                    snprintf(buf, sizeof(buf),
                             res == 0 ? "Password changed successfully."
                                      : "Failed to change password.");
                    write(sock, buf, strlen(buf) + 1);
                    break;
                }
                case 7:
                case 8:
                    printf("Employee disconnected.\n");
                    remove_session(userID);
                    close(sock);
                    return NULL;
            }
        }
    }

    
    else if (strcmp(role, "Manager") == 0) {
        while (read(sock, &choice, sizeof(choice)) > 0) {
            switch (choice) {
                case 1: {
                    int cid;
                    char action[16];
                    read(sock, &cid, sizeof(cid));
                    read(sock, action, sizeof(action));
                    int res = change_customer_status(cid, action);
                    snprintf(buf, sizeof(buf),
                             res == 0 ? "Customer %d set to %s" : "Failed to update.",
                             cid, action);
                    write(sock, buf, strlen(buf) + 1);
                    break;
                }
                case 2: {
                    int cid, empID;
                    read(sock, &cid, sizeof(cid));
                    read(sock, &empID, sizeof(empID));
                    int res = assign_loan_to_employee(cid, empID);
                    snprintf(buf, sizeof(buf),
                             res == 0 ? "Loan of %d assigned to Employee %d"
                                      : "Failed to assign.",
                             cid, empID);
                    write(sock, buf, strlen(buf) + 1);
                    break;
                }
                case 3:
                    show_feedbacks(buf, sizeof(buf));
                    write(sock, buf, strlen(buf) + 1);
                    break;
                case 4: {
                    char newpwd[64];
                    read(sock, newpwd, sizeof(newpwd));
                    change_password(userID, newpwd);
                    write(sock, "Password changed.", strlen("Password changed.") + 1);
                    break;
                }
                case 5:
                    view_all_customers(buf, sizeof(buf));
                    write(sock, buf, strlen(buf) + 1);
                    break;
                case 6:
                case 7:
                    printf("Manager disconnected.\n");
                    remove_session(userID);
                    close(sock);
                    return NULL;
            }
        }
    }

    
    else if (strcmp(role, "Admin") == 0) {
        while (read(sock, &choice, sizeof(choice)) > 0) {
            switch (choice) {
                case 1: {
                    char password[64];
                    read(sock, password, sizeof(password));
                    int newID = add_employee(password);
                    if (newID > 0)
                        snprintf(buf, sizeof(buf), "Employee created successfully with ID: %d", newID);
                    else
                        snprintf(buf, sizeof(buf), "Failed to create employee.");
                    write(sock, buf, strlen(buf) + 1);
                    break;
                }
                case 2: { 
                    int uid;
                    char newpwd[64];
                    read(sock, &uid, sizeof(uid));
                    read(sock, newpwd, sizeof(newpwd));
                    int res = modify_user_details(uid, newpwd);
                    snprintf(buf, sizeof(buf),
                            res == 0 ? "Password updated for user %d." : "Failed to update user %d.", uid);
                    write(sock, buf, strlen(buf) + 1);
                    break;
                }
                case 3: {
                    int uid;
                    char newrole[32];
                    read(sock, &uid, sizeof(uid));
                    read(sock, newrole, sizeof(newrole));

                    int res = change_role(uid, newrole);

                    
                    if (res == 0) {
                        snprintf(buf, sizeof(buf), "Role updated for user %d → %s.", uid, newrole);
                    } else {
                        snprintf(buf, sizeof(buf),
                                "Role change failed. Only Employee <-> Manager transitions allowed.");
                    }

                    write(sock, buf, strlen(buf) + 1);
                    break;
                }

                case 4: { 
                    char newpwd[64];
                    read(sock, newpwd, sizeof(newpwd));
                    int res = change_password(userID, newpwd);
                    snprintf(buf, sizeof(buf),
                            res == 0 ? "Admin password updated." : "Failed to update password.");
                    write(sock, buf, strlen(buf) + 1);
                    break;
                }
                case 5:
                    printf("Admin logged out.\n");
                    remove_session(userID);
                    close(sock);
                    return NULL;
                case 6:
                    printf("Admin exited.\n");
                    remove_session(userID);
                    close(sock);
                    return NULL;
            }
        }
    }

    remove_session(userID);
    close(sock);
    return NULL;
}



int main() {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    signal(SIGINT, handle_sigint);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    printf("Server started on %d\n", PORT);

    pthread_t cmd_thread;
    pthread_create(&cmd_thread, NULL, server_command_listener, NULL);
    pthread_detach(cmd_thread);

    while (server_running) {
        int client_sock = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
        if (client_sock < 0) {
            if (!server_running) break;
            continue;
        }
        printf("Client connected\n");
        int *new_sock = malloc(sizeof(int));
        *new_sock = client_sock;
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, new_sock);
        pthread_detach(tid);
    }

    printf("Sstopped.\n");
    return 0;
}
