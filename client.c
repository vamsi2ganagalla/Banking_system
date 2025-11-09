#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080


ssize_t recv_safe(int sock, void *buf, size_t size) {
    ssize_t n = read(sock, buf, size - 1);
    if (n <= 0) {
        printf("\nServer closed connection. Client disconnected gracefully.\n");
        close(sock);
        exit(0);
    }
    ((char *)buf)[n] = '\0';
    return n;
}

ssize_t send_safe(int sock, const void *buf, size_t size) {
    ssize_t n = write(sock, buf, size);
    if (n <= 0) {
        printf("\nServer closed connection. Client disconnected gracefully.\n");
        close(sock);
        exit(0);
    }
    return n;
}



void customer_dashboard(int sock) {
    int choice;
    double amount;
    int target;
    char buffer[4096], feedback[256], newpwd[64];

    while (1) {
        printf("\n--- Customer Menu ---\n"
               "1. View Balance\n"
               "2. Deposit\n"
               "3. Withdraw \n"
               "4. Transfer\n"
               "5. Apply Loan\n"
               "6. Change Password\n"
               "7. Add Feedback\n"
               "8. View History\n"
               "9. LogOut \n"
               "10. Exit \n"
               "Enter choice: ");
        scanf("%d", &choice);
        send_safe(sock, &choice, sizeof(choice));

        switch (choice) {
            case 1:
            case 8:
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 2:
                printf("Enter amount to deposit: ");
                scanf("%lf", &amount);
                send_safe(sock, &amount, sizeof(amount));
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;
            case 3:
                printf("Enter amount withdraw: ");
                scanf("%lf", &amount);
                send_safe(sock, &amount, sizeof(amount));
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 4:
                printf("Enter recive acc: ");
                scanf("%d", &target);
                printf("Enter amount to send: ");
                scanf("%lf", &amount);
                send_safe(sock, &target, sizeof(target));
                send_safe(sock, &amount, sizeof(amount));
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 5:
                printf("Enter loan amt: ");
                scanf("%lf", &amount);
                send_safe(sock, &amount, sizeof(amount));
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 6:
                printf("Enter new password: ");
                scanf("%s",newpwd);
                send_safe(sock, newpwd, sizeof(newpwd));
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 7:
                printf("Enter feedback: ");
                getchar();  
                fgets(feedback, sizeof(feedback), stdin);
                feedback[strcspn(feedback, "\n")] = 0;
                send_safe(sock, feedback, sizeof(feedback));
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 9:
                printf("Log OUt\n");
                return;

            case 10:
                printf("Exiting\n");
                exit(0);
            default:
                printf("Invalid choice\n");
                break;

        }
    }
}


void employee_dashboard(int sock) {
    int choice, custID;
    char buffer[4096], pwd[64], status[32], newpwd[64];

    while (1) {
        printf("\n--------------------------------- Employee Menu ---------------------------------\n"
               "1. Add Customer\n"
               "2. Modify Customer Details \n"
               "3. View Assigned Loans\n"
               "4. Approve/Reject Loans\n"
               "5. View Transactions\n"
               "6. Change Password\n"
               "7. Logout\n"
               "8. Exit\n"
               "Enter choice: ");
        scanf("%d", &choice);
        send_safe(sock, &choice, sizeof(choice));

        switch (choice) {
            case 1:
                printf( "Enter new password: ");
                scanf("%s",pwd);
                send_safe(sock, pwd, sizeof(pwd));
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 2:
                printf("Enter Customer ID: ");
                scanf("%d", &custID);
                printf("Enter password: ");
                scanf("%s",pwd);
                send_safe(sock, &custID, sizeof(custID));
                send_safe(sock, pwd, sizeof(pwd));
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 3:
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;
            case 5:
                printf("enter customer id to view transactions: ");
                scanf("%d", &custID);
                write(sock, &custID, sizeof(custID));
                read(sock, buffer, sizeof(buffer));
                printf("\n--- Transaction History for Customer %d ---\n%s\n", custID, buffer);
                break;


            case 4:
                printf("Enter customer ID: ");
                scanf("%d", &custID);
                printf("Enter status (approved/rejected): ");
                scanf("%s", status);
                send_safe(sock, &custID, sizeof(custID));
                send_safe(sock, status, sizeof(status));
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 6:
                printf("Enter new password: ");
                scanf("%s",newpwd);
                send_safe(sock, newpwd, sizeof(newpwd));
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 7:
                printf("Logged out\n");
                return;

            case 8:
                printf("Exiting\n");
                exit(0);
            default:
                printf("Invalid choice!.\n");
                break;
        }
    }
}


void manager_dashboard(int sock) {
    int choice, custID, empID;
    char buffer[4096], status[16], newpwd[64];

    while (1) {
        printf("\n--- Manager Menu ---\n"
               "1. Activate/Deactivate acc\n"
               "2. Assign Loan to Employee\n"
               "3. view Feedback\n"
               "4. Change Password\n"
               "5. View Customers details\n"
               "6. Logout\n"
               "7. Exit\n"
               "Enter choice: ");
        scanf("%d", &choice);
        send_safe(sock, &choice, sizeof(choice));

        switch (choice) {
            case 1:
                printf("Enter Customer ID: ");
                scanf("%d", &custID);
                printf("Enter action (active/inactive): ");
                scanf("%s", status);
                send_safe(sock, &custID, sizeof(custID));
                send_safe(sock, status, sizeof(status));
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 2:
                printf("Enter Customer ID : ");
                scanf("%d", &custID);
                printf("Enter Employee ID for assigning: ");
                scanf("%d", &empID);
                send_safe(sock, &custID, sizeof(custID));
                send_safe(sock, &empID, sizeof(empID));
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 3:
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 4:
                printf("New password: ");
                scanf("%s",newpwd);
                send_safe(sock, newpwd, sizeof(newpwd));
                recv_safe(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 5:
                recv_safe(sock, buffer, sizeof(buffer));
                printf("\n--- All Customer Accounts ---\n%s\n", buffer);
                break;

            case 6:
                printf("Logged Out...\n");
                return;

            case 7:
                printf("Exiting\n");
                exit(0);
            default:
                printf("Invalid choice.\n");
                break;
        }
    }
}

void admin_menu(int sock) {
    int choice, userID;
    char buffer[4096], newpwd[64], newrole[32];

    while (1) {
        printf("\n--- Admin Menu ---\n"
               "1. Add Bank Employee\n"
               "2. Change Customer/Employee Details\n"
               "3. Manage Roles\n"
               "4. Change Password\n"
               "5. Logout\n"
               "6. Exit\n"
               "Enter choice: ");
        scanf("%d", &choice);
        write(sock, &choice, sizeof(choice));

        switch (choice) {
            case 1:
                printf("Enter employee password: ");
                scanf("%s", newpwd);
                write(sock, newpwd, sizeof(newpwd));
                read(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 2:
                printf("Enter ID to modify: ");
                scanf("%d", &userID);
                printf("Enter New password: ");
                scanf("%s", newpwd);
                write(sock, &userID, sizeof(userID));
                write(sock, newpwd, sizeof(newpwd));
                read(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 3:
                printf("Enter User ID to change role: ");
                scanf("%d", &userID);
                printf("Enter new role (Employee/Manager): ");
                scanf("%s", newrole);
                write(sock, &userID, sizeof(userID));
                write(sock, newrole, sizeof(newrole));
                read(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 4:
                printf("Enter new  password: ");
                scanf("%s", newpwd);
                write(sock, newpwd, sizeof(newpwd));
                read(sock, buffer, sizeof(buffer));
                printf("%s\n", buffer);
                break;

            case 5:
                printf("Logged out...\n");
                return;

            case 6:
                printf("Exiting.\n");
                exit(0);
            default:
                printf("Invalid choice\n");
                break;

        }
    }
}


int main() {
    while (1) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server = {0};
        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);

        inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
	if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
            perror("Connection failed");
            printf("Server might be offline. Try again later.\n");
            close(sock);
            sleep(2);
            continue;
        }


        printf("\nConnected to server.\n");
        char id[32], pwd[32], role[32];

        printf("User ID: ");
        scanf("%s", id);
        printf("Password: ");
        scanf("%s",pwd);

        send_safe(sock, id, sizeof(id));
        send_safe(sock, pwd, sizeof(pwd));
        recv_safe(sock, role, sizeof(role));

        if (strcmp(role, "Invalid") == 0) {
            printf("Invalid login credentials.\n");
            close(sock);
            continue;
        } 
        else if (strcmp(role, "Inactive") == 0) {
            printf("Your account is currently inactive. Please contact the bank.\n");
            close(sock);
            continue;
        }
        else if (strcmp(role, "AlreadyLoggedIn") == 0) {
            printf("This user is already logged in elsewhere.\n");
            close(sock);
            continue;
        }

        printf("Login successful");

        if (strcmp(role, "Customer") == 0)
            customer_dashboard(sock);
        else if (strcmp(role, "Employee") == 0)
            employee_dashboard(sock);
        else if (strcmp(role, "Manager") == 0)
            manager_dashboard(sock);
        else if (strcmp(role, "Admin") == 0)
            admin_menu(sock);

        close(sock);

        char again;
        printf("\nDo you want to login again? (y/n): ");
        scanf(" %c", &again);
        if (again == 'n' || again == 'N') break;
    }

    printf("Exiting client.\n");
    return 0;
}
