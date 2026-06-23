#pragma once
#ifndef DEBT_H
#define DEBT_H

#include <QString>
#include <QDate>

struct Debt {
    int id = -1;
    QString partyType; // "supplier" or "customer"
    int partyId = 0;
    double amount = 0.0;
    QString debtType; // "owe" or "owed"
    QDate dueDate;
    QString description;
    bool isPaid = false;
    QDate paidAt;
};

#endif