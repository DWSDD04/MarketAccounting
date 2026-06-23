#pragma once
#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QString>
#include <QDateTime>

struct Transaction {
    int id = -1;
    QString type; // "sale", "purchase", "return", "expense", "income"
    QString referenceNumber;
    int partyId = 0;
    double totalAmount = 0.0;
    double discount = 0.0;
    double taxAmount = 0.0;
    double netAmount = 0.0;
    QString paymentMethod = "cash";
    QString notes;
    QDateTime createdAt;
};

#endif