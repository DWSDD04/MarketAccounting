#ifndef SUPPLIER_H
#define SUPPLIER_H

#include <QString>

struct Supplier {
    int id = -1;
    QString name;
    QString address;
    QString phone;
    QString fax;
    QString accountNumber;
    QString vatNumber;
    QString currency = "USD";
    double creditLimit = 0.0;
    int groupId = 1;
    QString notes;
    bool isActive = true;
};

#endif