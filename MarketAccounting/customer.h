#pragma once
#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <QString>

struct Customer {
    int id = -1;
    QString name;
    QString phone;
    QString email;
    QString address;
    double balance = 0.0;
    bool isActive = true;
};

#endif