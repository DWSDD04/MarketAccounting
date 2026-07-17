#pragma once
#ifndef USER_H
#define USER_H

#include <QString>

struct User {
    int id = -1;
    QString username;
    QString fullName;
    QString role;        // "admin", "manager", "cashier"
    QString passwordHash;
    bool isActive = true;

    bool isAdmin() const { return role == "admin"; }
    bool isManager() const { return role == "manager" || role == "admin"; }
    bool isCashier() const { return role == "cashier" || isManager(); }
};

#endif