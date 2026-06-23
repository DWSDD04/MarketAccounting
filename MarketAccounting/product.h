#pragma once
#ifndef PRODUCT_H
#define PRODUCT_H

#include <QString>

struct Product {
    int id = -1;
    QString code;
    QString name;
    int categoryId = 0;
    int supplierId = 0;
    double purchasePrice = 0.0;
    double salePrice = 0.0;
    int quantity = 0;
    int minStock = 10;
    QString unit = "piece";
    QString barcode;
    bool isActive = true;
};

#endif