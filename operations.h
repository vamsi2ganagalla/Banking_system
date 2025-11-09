#ifndef BANK_OPS_H
#define BANK_OPS_H

#include <stddef.h>

// -------------------- USER AUTH --------------------
int auth(const char *id, const char *pwd, char *role);

// -------------------- CUSTOMER OPS --------------------
int show_balance(int custID, double *balance);
int update_bal(int custID, double amount, int isDeposit);
int save_transaction(int custID, const char *type, double amount);
int view_history(int custID, char *buffer, size_t size);

// -------------------- LOAN OPS --------------------
int apply_for_loan(int custID, double amount);
int show_loans(int empID, char *buffer, size_t size);
int update_loan_status(int custID, const char *status);

// -------------------- PASSWORD OPS --------------------
int change_password(int userID, const char *newpwd);
int modify_customer_details(int custID, const char *newpwd);

// -------------------- FEEDBACK OPS --------------------
int add_feedback(int custID, const char *feedback);
int show_feedbacks(char *buffer, size_t size);

// -------------------- EMPLOYEE OPS --------------------
int add_customer(const char *password);

// -------------------- MANAGER OPS --------------------
int change_customer_status(int custID, const char *new_status);
int assign_loan_to_employee(int custID, int empID);
int view_all_customers(char *buffer, size_t size);

// --- Admin Functions ---
int add_employee(const char *password);
int modify_user_details(int userID, const char *newpwd);
int change_role(int userID, const char *new_role);

#endif
